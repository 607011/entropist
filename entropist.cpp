/* Generate random numbers from global key events (macOS version).
 *
 * Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved. 
 */

#include <iostream>
#include <string>
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

#ifdef DEBUG
#include <iomanip>
std::string stringFromCFString(CFStringRef cfStr)
{
  static const CFIndex MaxLength = 3 + 1; // CFStringGetMaximumSizeForEncoding(1, kCFStringEncodingUTF8) + 1
  char buffer[MaxLength] = {0};
  CFStringGetCString(cfStr, buffer, MaxLength, kCFStringEncodingUTF8);
  return std::string(buffer);
}

CFStringRef stringFromKey(CGKeyCode keyCode, CGEventFlags modifiers)
{
  TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
  CFDataRef layoutData = reinterpret_cast<CFDataRef>(TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData));
  const UCKeyboardLayout *keyboardLayout = reinterpret_cast<const UCKeyboardLayout *>(CFDataGetBytePtr(layoutData));
  UInt32 deadKeyState = 0;
  static const UniCharCount MaxLength = 4;
  UniChar chars[MaxLength] = {0};
  UniCharCount realLength = 0;
  UInt32 modifierKeyState = 0;
  if (modifiers & kCGEventFlagMaskShift) {
    modifierKeyState |= shiftKey;
  }
  if (modifiers & kCGEventFlagMaskControl) {
    modifierKeyState |= controlKey;
  }
  if (modifiers & kCGEventFlagMaskAlternate) {
    modifierKeyState |= optionKey;
  }
  if (modifiers & kCGEventFlagMaskCommand) {
    modifierKeyState |= cmdKey;
  }
  UCKeyTranslate(keyboardLayout,
                 keyCode,
                 kUCKeyActionDown,
                 (modifierKeyState >> 8) & 0xff,
                 LMGetKbdType(),
                 kUCKeyTranslateNoDeadKeysBit,
                 &deadKeyState,
                 MaxLength / sizeof(UniChar),
                 &realLength,
                 chars);
  CFRelease(currentKeyboard);
  return CFStringCreateWithCharacters(kCFAllocatorDefault, chars, realLength);
}
#endif

CryptoPP::SHA512 hash;
CryptoPP::byte digest[CryptoPP::SHA512::DIGESTSIZE];

CGEventRef keyEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
  if (type == kCGEventKeyUp)
  {
	  CGKeyCode keycode = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    CGEventFlags flags = static_cast<CGEventFlags>(CGEventGetFlags(event));
    CGEventTimestamp timestamp = CGEventGetTimestamp(event);
#ifdef DEBUG
    CFStringRef keyStrRef = stringFromKey(keycode, flags);
    std::cout << timestamp << " " << std::hex << std::setw(8) << std::setfill('0') << flags << ": ";
    if (flags & kCGEventFlagMaskShift) {
      std::cout << "Shift+";
    }
    if (flags & kCGEventFlagMaskAlternate) {
      std::cout << "Alt+";
    }
    if (flags & kCGEventFlagMaskControl) {
      std::cout << "Ctrl+";
    }
    if (flags & kCGEventFlagMaskCommand) {
      std::cout << "Cmd+";
    }
    std::cout << stringFromCFString(keyStrRef) << std::endl;
    // CGEventSetIntegerValueField(event, kCGKeyboardEventKeycode, (int64_t)keycode);
#endif
    hash.Update(reinterpret_cast<CryptoPP::byte *>(&timestamp), sizeof(timestamp) / sizeof(CryptoPP::byte));
    hash.Update(reinterpret_cast<CryptoPP::byte *>(&keycode), sizeof(keycode) / sizeof(CryptoPP::byte));
    hash.Update(reinterpret_cast<CryptoPP::byte *>(&flags), sizeof(flags) / sizeof(CryptoPP::byte));
    hash.Final(digest);
    std::string hexDigest;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hexDigest), false);
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();
    std::cout << "\r" << hexDigest << std::flush;
  }
  return event;
}

int main(void)
{
  CGEventMask eventMask = ((1 << kCGEventKeyDown) | (1 << kCGEventKeyUp));
  CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, eventMask, keyEventCallback, NULL);
  if (!eventTap) {
      std::cerr << "failed to create event tap" << std::endl;
      return 1;
  }
  CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
  CGEventTapEnable(eventTap, true);
  CFRunLoopRun();
  return 0;
}
