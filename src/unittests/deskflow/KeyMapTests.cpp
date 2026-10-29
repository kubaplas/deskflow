/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2016 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */
#include "KeyMapTests.h"

#include "deskflow/KeyMap.h"

using namespace deskflow;
using KeyItemList = KeyMap::KeyItemList;
using KeyEntryList = std::vector<KeyItemList>;

void KeyMapTests::findBestKey_requiredDown_matchExactFirstItem()
{
  KeyMap keyMap;
  KeyEntryList entryList;
  KeyItemList itemList;
  KeyMap::KeyItem item;
  item.m_required = KeyModifierShift;
  item.m_sensitive = KeyModifierShift;
  KeyModifierMask desiredState = KeyModifierShift;
  itemList.push_back(item);
  entryList.push_back(itemList);

  QCOMPARE(keyMap.findBestKey(entryList, desiredState), 0);
}

void KeyMapTests::findBestKey_requiredAndExtraSensitiveDown_matchExactFirstItem()
{
  KeyMap keyMap;
  KeyEntryList entryList;
  KeyItemList itemList;
  KeyMap::KeyItem item;
  item.m_required = KeyModifierShift;
  item.m_sensitive = KeyModifierShift | KeyModifierAlt;
  KeyModifierMask desiredState = KeyModifierShift;
  itemList.push_back(item);
  entryList.push_back(itemList);

  QCOMPARE(keyMap.findBestKey(entryList, desiredState), 0);
}

void KeyMapTests::findBestKey_requiredAndExtraSensitiveDown_matchExactSecondItem()
{
  KeyMap keyMap;
  KeyEntryList entryList;
  KeyItemList itemList1;
  KeyMap::KeyItem item1;
  item1.m_required = KeyModifierAlt;
  item1.m_sensitive = KeyModifierShift | KeyModifierAlt;
  KeyMap::KeyItemList itemList2;
  KeyMap::KeyItem item2;
  item2.m_required = KeyModifierShift;
  item2.m_sensitive = KeyModifierShift | KeyModifierAlt;
  KeyModifierMask desiredState = KeyModifierShift;
  itemList1.push_back(item1);
  itemList2.push_back(item2);
  entryList.push_back(itemList1);
  entryList.push_back(itemList2);
  QCOMPARE(keyMap.findBestKey(entryList, desiredState), 1);
}

void KeyMapTests::findBestKey_extraSensitiveDown_matchExactSecondItem()
{
  KeyMap keyMap;
  KeyEntryList entryList;
  KeyItemList itemList1;
  KeyMap::KeyItem item1;
  item1.m_required = 0;
  item1.m_sensitive = KeyModifierAlt;
  KeyMap::KeyItemList itemList2;
  KeyMap::KeyItem item2;
  item2.m_required = 0;
  item2.m_sensitive = KeyModifierShift;
  KeyModifierMask desiredState = KeyModifierAlt;
  itemList1.push_back(item1);
  itemList2.push_back(item2);
  entryList.push_back(itemList1);
  entryList.push_back(itemList2);

  QCOMPARE(keyMap.findBestKey(entryList, desiredState), 1);
}

void KeyMapTests::findBestKey_noRequiredDown_matchOneRequiredChangeItem()
{
  KeyMap keyMap;
  KeyEntryList entryList;
  KeyItemList itemList1;
  KeyMap::KeyItem item1;
  item1.m_required = KeyModifierShift | KeyModifierAlt;
  item1.m_sensitive = KeyModifierShift | KeyModifierAlt;
  KeyMap::KeyItemList itemList2;
  KeyMap::KeyItem item2;
  item2.m_required = KeyModifierShift;
  item2.m_sensitive = KeyModifierShift | KeyModifierAlt;
  KeyModifierMask desiredState = 0;
  itemList1.push_back(item1);
  itemList2.push_back(item2);
  entryList.push_back(itemList1);
  entryList.push_back(itemList2);

  QCOMPARE(keyMap.findBestKey(entryList, desiredState), 1);
}

void KeyMapTests::findBestKey_onlyOneRequiredDown_matchTwoRequiredChangesItem()
{
  KeyMap keyMap;
  KeyEntryList entryList;
  KeyItemList itemList1;
  KeyMap::KeyItem item1;
  item1.m_required = KeyModifierShift | KeyModifierAlt | KeyModifierControl;
  item1.m_sensitive = KeyModifierShift | KeyModifierAlt | KeyModifierControl;
  KeyItemList itemList2;
  KeyMap::KeyItem item2;
  item2.m_required = KeyModifierShift | KeyModifierAlt;
  item2.m_sensitive = KeyModifierShift | KeyModifierAlt | KeyModifierControl;
  KeyModifierMask desiredState = 0;
  itemList1.push_back(item1);
  itemList2.push_back(item2);
  entryList.push_back(itemList1);
  entryList.push_back(itemList2);

  QCOMPARE(keyMap.findBestKey(entryList, desiredState), 1);
}

void KeyMapTests::findBestKey_noRequiredDown_cannotMatch()
{
  KeyMap keyMap;
  KeyEntryList entryList;
  KeyItemList itemList;
  KeyMap::KeyItem item;
  item.m_required = 0xffffffff;
  item.m_sensitive = 0xffffffff;
  KeyModifierMask desiredState = 0;
  itemList.push_back(item);
  entryList.push_back(itemList);

  QCOMPARE(keyMap.findBestKey(entryList, desiredState), -1);
}

void KeyMapTests::isCommand()
{
  KeyMap keyMap;
  KeyModifierMask mask = KeyModifierShift;
  QVERIFY(!keyMap.isCommand(mask));

  mask = KeyModifierControl;
  QVERIFY(keyMap.isCommand(mask));

  mask = KeyModifierAlt;
  QVERIFY(keyMap.isCommand(mask));

  mask = KeyModifierAltGr;
  QVERIFY(keyMap.isCommand(mask));

  mask = KeyModifierMeta;
  QVERIFY(keyMap.isCommand(mask));

  mask = KeyModifierSuper;
  QVERIFY(keyMap.isCommand(mask));
}

void KeyMapTests::mapkey()
{
  KeyMap keyMap{};
  KeyMap::Keystroke stroke('A', true, false, 1);
  KeyMap::KeyItem keyItem;
  keyItem.m_button = 'A';
  keyItem.m_group = 1;
  keyItem.m_id = 'A';
  keyMap.addKeyEntry(keyItem);
  keyMap.finish();
  KeyMap::Keystrokes strokes{stroke};
  KeyMap::ModifierToKeys activeModifiers{};
  KeyModifierMask currentState{};
  KeyModifierMask desiredMask{};
  auto result = keyMap.mapKey(strokes, kKeySetModifiers, 1, activeModifiers, currentState, desiredMask, false, "en");
  QVERIFY(result != nullptr);
  desiredMask = KeyModifierControl;
  result = keyMap.mapKey(strokes, kKeySetModifiers, 1, activeModifiers, currentState, desiredMask, false, "en");
  QVERIFY(result == nullptr);
}

void KeyMapTests::mapCommandKey_shiftedDuplicate_prefersUnshiftedButton()
{
  // some layouts carry the same character on two buttons, one of them only
  // with shift held (Polish Pro has l-stroke on option+l and on
  // option+shift+p).  a command key is synthesized with the desired mask,
  // so the shifted button would be pressed without its shift and generate
  // a different character.  the unshifted button must win.
  static const KeyID lStroke = 0x0142;
  const KeyButton shiftedButton = 36;
  const KeyButton plainButton = 38;

  KeyMap keyMap;

  // added first, so it is the first entry in the list
  KeyMap::KeyItem shifted;
  shifted.m_id = lStroke;
  shifted.m_button = shiftedButton;
  shifted.m_required = KeyModifierShift | KeyModifierAlt;
  shifted.m_sensitive = KeyModifierShift | KeyModifierAlt;
  keyMap.addKeyEntry(shifted);

  KeyMap::KeyItem plain;
  plain.m_id = lStroke;
  plain.m_button = plainButton;
  plain.m_required = KeyModifierAlt;
  plain.m_sensitive = KeyModifierShift | KeyModifierAlt;
  keyMap.addKeyEntry(plain);

  KeyMap::KeyItem alt;
  alt.m_id = kKeyAlt_L;
  alt.m_button = 59;
  alt.m_generates = KeyModifierAlt;
  keyMap.addKeyEntry(alt);
  keyMap.finish();

  KeyMap::Keystrokes keys;
  KeyMap::ModifierToKeys activeModifiers;
  KeyModifierMask currentState = 0;
  const auto *mapped = keyMap.mapKey(keys, lStroke, 0, activeModifiers, currentState, KeyModifierAlt, false, "en");
  QVERIFY(mapped != nullptr);
  QCOMPARE(mapped->m_button, plainButton);
}

void KeyMapTests::mapCommandKey_shiftedKeyId_stillFindsShiftedButton()
{
  // a shifted KeyID with no shift in the desired mask (e.g. ctrl+'A' from a
  // server that sends the upper case id) must still resolve to its button
  static const KeyID upperA = 'A';
  const KeyButton aButton = 1;

  KeyMap keyMap;
  KeyMap::KeyItem item;
  item.m_id = upperA;
  item.m_button = aButton;
  item.m_required = KeyModifierShift;
  item.m_sensitive = KeyModifierShift;
  keyMap.addKeyEntry(item);

  KeyMap::KeyItem ctrl;
  ctrl.m_id = kKeyControl_L;
  ctrl.m_button = 59;
  ctrl.m_generates = KeyModifierControl;
  keyMap.addKeyEntry(ctrl);
  keyMap.finish();

  KeyMap::Keystrokes keys;
  KeyMap::ModifierToKeys activeModifiers;
  KeyModifierMask currentState = 0;
  const auto *mapped = keyMap.mapKey(keys, upperA, 0, activeModifiers, currentState, KeyModifierControl, false, "en");
  QVERIFY(mapped != nullptr);
  QCOMPARE(mapped->m_button, aButton);
}

void KeyMapTests::parseModifiers_plusKey_keepsPlusAsKey()
{
  std::string keystroke = "Control+Shift++";
  KeyModifierMask mask = 0;

  QVERIFY(KeyMap::parseModifiers(keystroke, mask));
  QCOMPARE(mask, static_cast<KeyModifierMask>(KeyModifierControl | KeyModifierShift));
  QCOMPARE(keystroke, std::string("+"));
}

void KeyMapTests::parseKey_plusSymbol_parsesAsAsciiKey()
{
  KeyID key = kKeyNone;

  QVERIFY(KeyMap::parseKey("+", key));
  QCOMPARE(key, static_cast<KeyID>('+'));
}

QTEST_MAIN(KeyMapTests)
