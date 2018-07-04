// -*- coding: utf-8 -*-
// Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
// All rights reserved. 

#include "entropist.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <stdint.h>
#include <Carbon/Carbon.h>


static CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
  Entropist *entropist = reinterpret_cast<Entropist*>(refcon);
  switch (type)
  {
    case kCGEventKeyDown:
    {
      CGEventTimestamp timestamp = CGEventGetTimestamp(event);
      entropist->add(reinterpret_cast<uint8_t*>(&timestamp), sizeof(timestamp));
      CGKeyCode keycode = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
      entropist->add(reinterpret_cast<uint8_t*>(&keycode), sizeof(keycode));
      CGEventFlags flags = static_cast<CGEventFlags>(CGEventGetFlags(event));
      entropist->add(reinterpret_cast<uint8_t*>(&flags), sizeof(flags));
      break;
    }
    case kCGEventKeyUp:
    {
      CGEventTimestamp timestamp = CGEventGetTimestamp(event);
      entropist->add(reinterpret_cast<uint8_t*>(&timestamp), sizeof(timestamp));
    }
    case kCGEventMouseMoved:
    {
      CGEventTimestamp timestamp = CGEventGetTimestamp(event);
      entropist->add(reinterpret_cast<uint8_t*>(&timestamp), sizeof(timestamp));
      CGPoint location = CGEventGetLocation(event);
      entropist->add(reinterpret_cast<uint8_t*>(&location), sizeof(location));
      break;
    }
    case kCGEventLeftMouseUp:
    {
      CGEventTimestamp timestamp = CGEventGetTimestamp(event);
      entropist->add(reinterpret_cast<uint8_t*>(&timestamp), sizeof(timestamp));
      CGPoint location = CGEventGetLocation(event);
      entropist->add(reinterpret_cast<uint8_t*>(&location), sizeof(location));
      break;
    }
    case kCGEventScrollWheel:
    {
      CGEventTimestamp timestamp = CGEventGetTimestamp(event);
      entropist->add(reinterpret_cast<uint8_t*>(&timestamp), sizeof(timestamp));
      CGPoint location = CGEventGetLocation(event);
      entropist->add(reinterpret_cast<uint8_t*>(&location), sizeof(location));
      int64_t da1 = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1);
      entropist->add(reinterpret_cast<uint8_t*>(&da1), sizeof(da1));
      int64_t da2 = CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis2);
      entropist->add(reinterpret_cast<uint8_t*>(&da2), sizeof(da2));
      break;
    }
    default:
      break;
  }
  entropist->output();
  return event;
}


void Entropist::runner(void)
{
  const CGEventMask eventMask = 
    CGEventMaskBit(kCGEventMouseMoved) |
    CGEventMaskBit(kCGEventLeftMouseUp) |
    CGEventMaskBit(kCGEventScrollWheel) |
    CGEventMaskBit(kCGEventKeyDown) |
    CGEventMaskBit(kCGEventKeyUp);
  CFMachPortRef eventTap = CGEventTapCreate(
      kCGSessionEventTap,
      kCGHeadInsertEventTap,
      kCGEventTapOptionDefault,
      eventMask,
      eventCallback,
      &instance());
  if (!eventTap)
  {
      std::cerr << "failed to create event tap" << std::endl;
      return;
  }
  CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
  CGEventTapEnable(eventTap, true);
  if (eventTap)
  {
    CFRelease(eventTap);
  }
  if (runLoopSource)
  {
    CFRelease(runLoopSource);
  }
  CFRunLoopRun();
}
