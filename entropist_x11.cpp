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
  std::cout << "Entropist::runner()" << std::endl;
  while (true)
  {
    XNextEvent(display, &event);
    std::cout << event.type << ',' << std::flush;
    switch(event.type)
    {
      case MotionNotify:
      {
	std::cout << event.xmotion.time << ' ' << event.xmotion.x << ' ' << event.xmotion.y << std::endl;
        instance().hash.Update(reinterpret_cast<uint8_t*>(&event.xmotion.time), sizeof(event.xmotion.time));
        instance().hash.Update(reinterpret_cast<uint8_t*>(&event.xmotion.x), sizeof(event.xmotion.x));
        instance().hash.Update(reinterpret_cast<uint8_t*>(&event.xmotion.y), sizeof(event.xmotion.y));
        instance().total += sizeof(event.xmotion.time) + sizeof(event.xmotion.x) + sizeof(&event.xmotion.y);
        instance().output();
        break;
      }
      case KeyRelease:
      {
        instance().hash.Update(reinterpret_cast<uint8_t*>(&event.xkey.time), sizeof(event.xkey.time));
        instance().hash.Update(reinterpret_cast<uint8_t*>(&event.xkey.keycode), sizeof(event.xkey.keycode));
        instance().hash.Update(reinterpret_cast<uint8_t*>(&event.xkey.state), sizeof(event.xkey.state));
        instance().total += sizeof(event.xkey.time) + sizeof(event.xkey.keycode) + sizeof(&event.xkey.state);
        instance().output();
        break;
      }
      default:
        break;
    }
  }
}
