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

  while(true)
  {
    XNextEvent(display, &event);
    switch(event.type)
    {
      case MotionNotify:
      {
        printf("x %d y %d\n", event.xmotion.x, event.xmotion.y );
        break;
      }
      default:
        break;
    }
  }
}