/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 */

#include <X11/Xlib.h>
#include "entropist.h"

void Entropist::runner(void)
{
  Display *display;
  Window root_window;
  XEvent event;
  display = XOpenDisplay(0);
  root_window = XRootWindow(display, 0);
  XSelectInput(display, root_window, PointerMotionMask);
  while (true)
  {
    XNextEvent(display, &event);
    switch(event.type)
    {
      case MotionNotify:
      {
        instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&event.xmotion.time), sizeof(event.xmotion.time) / sizeof(CryptoPP::byte));
        instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&event.xmotion.x), sizeof(event.xmotion.x) / sizeof(CryptoPP::byte));
        instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&event.xmotion.y), sizeof(event.xmotion.y) / sizeof(CryptoPP::byte));
        instance().total += sizeof(event.xmotion.time) + sizeof(event.xmotion.x) + sizeof(&event.xmotion.y);
        instance.output();
        break;
      }
      case KeyRelease:
      {
        instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&event.xkey.time), sizeof(event.xkey.time) / sizeof(CryptoPP::byte));
        instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&event.xkey.keycode), sizeof(event.xkey.keycode) / sizeof(CryptoPP::byte));
        instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&event.xkey.state), sizeof(event.xkey.state) / sizeof(CryptoPP::byte));
        instance().total += sizeof(event.xkey.time) + sizeof(event.xkey.keycode) + sizeof(&event.xkey.state);
        instance.output();
        break;
      }
      default:
        break;
    }
  }
}
