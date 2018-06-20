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
  int fd = open(instance().mouseInput.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    std::cerr << "ERROR: Cannot open " << instance().mouseInput << "." << std::endl;
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
      instance().hash.Update(reinterpret_cast<uint8_t*>(&evt), sizeof(evt));
      instance().totalBits += 4;
      instance().output();
    }
  }
  close(fd);
}
