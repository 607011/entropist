/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved. 
 */

#include "entropist.h"
#include <iostream>


CGEventRef Entropist::eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
  switch (type)
  {
    case kCGEventKeyUp:
    {
      CGKeyCode keycode = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
      CGEventFlags flags = static_cast<CGEventFlags>(CGEventGetFlags(event));
      CGEventTimestamp timestamp = CGEventGetTimestamp(event);
      instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&timestamp), sizeof(timestamp) / sizeof(CryptoPP::byte));
      instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&keycode), sizeof(keycode) / sizeof(CryptoPP::byte));
      instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&flags), sizeof(flags) / sizeof(CryptoPP::byte));
      instance().totalBits += 2;
      break;
    }
    case kCGEventMouseMoved:
    {
      CGPoint location = CGEventGetLocation(event);
      CGEventTimestamp timestamp = CGEventGetTimestamp(event);
      instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&location), sizeof(location) / sizeof(CryptoPP::byte));
      instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&timestamp), sizeof(timestamp) / sizeof(CryptoPP::byte));
      instance().totalBits += 4;
      break;
    }
    default:
      break;
  }
  instance().output();
  return event;
}


void Entropist::runner(void)
{
  const CGEventMask eventMask = 
    CGEventMaskBit(kCGEventMouseMoved) |
    CGEventMaskBit(kCGEventKeyUp);
  CFMachPortRef eventTap = CGEventTapCreate(
      kCGSessionEventTap,
      kCGHeadInsertEventTap,
      kCGEventTapOptionDefault,
      eventMask,
      eventCallback,
      nullptr);
  if (!eventTap)
  {
      std::cerr << "failed to create event tap" << std::endl;
      return;
  }
  CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
  CGEventTapEnable(eventTap, true);
  if (eventTap) {
    CFRelease(eventTap);
  }
  if (runLoopSource) {
    CFRelease(runLoopSource);
  }
  CFRunLoopRun();
}
