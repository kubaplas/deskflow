/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2011 Nick Bolton
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "OSXKeyStateTests.h"

#include "base/EventQueue.h"

#define SHIFT_ID_L kKeyShift_L
#define SHIFT_ID_R kKeyShift_R
#define SHIFT_BUTTON 57
#define A_CHAR_ID 0x00000061
#define A_CHAR_BUTTON 001
// latin small letter a with ogonek, typed with Option+a on Polish layouts
#define A_OGONEK_ID 0x00000105
#define ALT_R_BUTTON 62

void OSXKeyStateTests::initTestCase()
{
  m_arch.init();
  m_log.setFilter(LogLevel::Level::Verbose);
}

void OSXKeyStateTests::mapModifiersFromOSX_OSXMask()
{
  deskflow::KeyMap keyMap;
  EventQueue eventQueue;
  OSXKeyState keyState(&eventQueue, keyMap, {"en"}, true);

  KeyModifierMask outMask = 0;

  uint32_t shiftMask = 0 | kCGEventFlagMaskShift;
  outMask = keyState.mapModifiersFromOSX(shiftMask);
  QCOMPARE(outMask, KeyModifierShift);

  uint32_t ctrlMask = 0 | kCGEventFlagMaskControl;
  outMask = keyState.mapModifiersFromOSX(ctrlMask);
  QCOMPARE(outMask, KeyModifierControl);

  uint32_t altMask = 0 | kCGEventFlagMaskAlternate;
  outMask = keyState.mapModifiersFromOSX(altMask);
  QCOMPARE(outMask, KeyModifierAlt);

  uint32_t cmdMask = 0 | kCGEventFlagMaskCommand;
  outMask = keyState.mapModifiersFromOSX(cmdMask);
  QCOMPARE(outMask, KeyModifierSuper);

  uint32_t capsMask = 0 | kCGEventFlagMaskAlphaShift;
  outMask = keyState.mapModifiersFromOSX(capsMask);
  QCOMPARE(outMask, KeyModifierCapsLock);

  uint32_t numMask = 0 | kCGEventFlagMaskNumericPad;
  outMask = keyState.mapModifiersFromOSX(numMask);
  QCOMPARE(outMask, KeyModifierNumLock);
}

void OSXKeyStateTests::mapAltGr_keyAndModifiers()
{
  // macOS has no AltGr key, the right Option key takes its place
  QCOMPARE(OSXKeyState::mapAltGrKey(kKeyAltGr), static_cast<KeyID>(kKeyAlt_R));
  QCOMPARE(OSXKeyState::mapAltGrKey(kKeyAlt_L), static_cast<KeyID>(kKeyAlt_L));
  QCOMPARE(OSXKeyState::mapAltGrKey(A_OGONEK_ID), static_cast<KeyID>(A_OGONEK_ID));

  QCOMPARE(OSXKeyState::mapAltGrModifiers(KeyModifierAltGr), static_cast<KeyModifierMask>(KeyModifierAlt));
  QCOMPARE(
      OSXKeyState::mapAltGrModifiers(KeyModifierAltGr | KeyModifierShift),
      static_cast<KeyModifierMask>(KeyModifierAlt | KeyModifierShift)
  );
  QCOMPARE(
      OSXKeyState::mapAltGrModifiers(KeyModifierAltGr | KeyModifierAlt), static_cast<KeyModifierMask>(KeyModifierAlt)
  );

  // masks without AltGr are left alone
  QCOMPARE(OSXKeyState::mapAltGrModifiers(0), static_cast<KeyModifierMask>(0));
  QCOMPARE(OSXKeyState::mapAltGrModifiers(KeyModifierControl), static_cast<KeyModifierMask>(KeyModifierControl));
  QCOMPARE(OSXKeyState::mapAltGrModifiers(KeyModifierAlt), static_cast<KeyModifierMask>(KeyModifierAlt));
}

void OSXKeyStateTests::mapAltGr_optionLayerCharacterIsMapped()
{
  // the macOS key map records characters on the Option layer as requiring
  // Alt.  a server that resolved such a character on its own AltGr layer
  // sends the character with the AltGr modifier, which matches nothing.
  deskflow::KeyMap keyMap;

  deskflow::KeyMap::KeyItem character;
  character.m_id = A_OGONEK_ID;
  character.m_button = A_CHAR_BUTTON;
  character.m_required = KeyModifierAlt;
  character.m_sensitive = KeyModifierShift | KeyModifierAlt;
  keyMap.addKeyEntry(character);

  deskflow::KeyMap::KeyItem option;
  option.m_id = kKeyAlt_R;
  option.m_button = ALT_R_BUTTON;
  option.m_generates = KeyModifierAlt;
  keyMap.addKeyEntry(option);
  keyMap.finish();

  deskflow::KeyMap::Keystrokes keys;
  deskflow::KeyMap::ModifierToKeys activeModifiers;
  KeyModifierMask currentState = 0;
  QVERIFY(keyMap.mapKey(keys, A_OGONEK_ID, 0, activeModifiers, currentState, KeyModifierAltGr, false, "en") == nullptr);

  // translating AltGr to Alt finds the Option layer entry
  keys.clear();
  activeModifiers.clear();
  currentState = 0;
  const auto *mapped = keyMap.mapKey(
      keys, OSXKeyState::mapAltGrKey(A_OGONEK_ID), 0, activeModifiers, currentState,
      OSXKeyState::mapAltGrModifiers(KeyModifierAltGr), false, "en"
  );
  QVERIFY(mapped != nullptr);
  QCOMPARE(mapped->m_button, static_cast<KeyButton>(A_CHAR_BUTTON));

  // the AltGr key itself is the right Option key
  keys.clear();
  activeModifiers.clear();
  currentState = 0;
  mapped = keyMap.mapKey(
      keys, OSXKeyState::mapAltGrKey(kKeyAltGr), 0, activeModifiers, currentState,
      OSXKeyState::mapAltGrModifiers(KeyModifierAltGr), false, "en"
  );
  QVERIFY(mapped != nullptr);
  QCOMPARE(mapped->m_button, static_cast<KeyButton>(ALT_R_BUTTON));
}

void OSXKeyStateTests::fakePollShift()
{
  deskflow::KeyMap keyMap;
  EventQueue eventQueue;
  OSXKeyState keyState(&eventQueue, keyMap, {"en"}, true);
  keyState.updateKeyMap();

  keyState.fakeKeyDown(SHIFT_ID_L, 0, 1, "en");
  QVERIFY(isKeyPressed(keyState, SHIFT_BUTTON));

  keyState.fakeKeyUp(1);
  QVERIFY(!isKeyPressed(keyState, SHIFT_BUTTON));

  keyState.fakeKeyDown(SHIFT_ID_R, 0, 2, "en");
  QVERIFY(isKeyPressed(keyState, SHIFT_BUTTON));

  keyState.fakeKeyUp(2);
  QVERIFY(!isKeyPressed(keyState, SHIFT_BUTTON));
}

void OSXKeyStateTests::fakePollChar()
{
  deskflow::KeyMap keyMap;
  EventQueue eventQueue;
  OSXKeyState keyState(&eventQueue, keyMap, {"en"}, true);
  keyState.updateKeyMap();

  keyState.fakeKeyDown(A_CHAR_ID, 0, 1, "en");
  QVERIFY(isKeyPressed(keyState, A_CHAR_BUTTON));

  keyState.fakeKeyUp(1);
  QVERIFY(!isKeyPressed(keyState, A_CHAR_BUTTON));

  // HACK: delete the key in case it was typed into a text editor.
  // we should really set focus to an invisible window.
  keyState.fakeKeyDown(kKeyBackSpace, 0, 2, "en");
  keyState.fakeKeyUp(2);
}

void OSXKeyStateTests::fakePollCharWithModifier()
{
  deskflow::KeyMap keyMap;
  EventQueue eventQueue;
  OSXKeyState keyState(&eventQueue, keyMap, {"en"}, true);
  keyState.updateKeyMap();

  keyState.fakeKeyDown(A_CHAR_ID, KeyModifierShift, 1, "en");
  QVERIFY(isKeyPressed(keyState, A_CHAR_BUTTON));

  keyState.fakeKeyUp(1);
  QVERIFY(!isKeyPressed(keyState, A_CHAR_BUTTON));

  // HACK: delete the key in case it was typed into a text editor.
  // we should really set focus to an invisible window.
  keyState.fakeKeyDown(kKeyBackSpace, 0, 2, "en");
  keyState.fakeKeyUp(2);
}

bool OSXKeyStateTests::isKeyPressed(const OSXKeyState &keyState, KeyButton button)
{
  // HACK: allow os to realize key state changes.
  Arch::sleep(.2);

  IKeyState::KeyButtonSet pressed;
  keyState.pollPressedKeys(pressed);

  IKeyState::KeyButtonSet::const_iterator it;
  for (it = pressed.begin(); it != pressed.end(); ++it) {
    LOG_DEBUG("checking key %d", *it);
    if (*it == button) {
      return true;
    }
  }
  return false;
}

QTEST_MAIN(OSXKeyStateTests)
