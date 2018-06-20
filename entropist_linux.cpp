/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 */

#include "entropist.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include <linux/input.h>
#include <linux/input-event-codes.h>

void Entropist::runner(void)
{
  int fd = open(self().mouseInput().c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    std::cerr << "ERROR: Cannot open " << self().mouseInput() << "." << std::endl;
    return;
  }
  fcntl(fd, F_SETFL, 0);
  while (true)
  {
    struct input_event evt;
    int n = read(fd, (void*)&evt, sizeof(evt));
    if (n < 0) {
      std::cerr << "Read failed." << std::endl;
      break;
    }
    if (evt.type == EV_ABS || evt.type == EV_REL) {
      self().add(reinterpret_cast<uint8_t*>(&evt), sizeof(evt));
      self().output();
    }
  }
  close(fd);
}


void Entropist::setMouseInput(const std::string &in)
{
  mMouseInput = in;
}

void Entropist::setKeyboardInput(const std::string &in)
{
  mKeyboardInput = in;
}

const std::string &Entropist::mouseInput(void) const
{
  return mMouseInput;
}

const std::string &Entropist::keyboardInput(void) const
{
  return mKeyboardInput;
}
