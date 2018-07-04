// -*- coding: utf-8 -*-
// Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
// All rights reserved.

#ifdef LINUX

#include "entropist.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include <linux/input.h>
#include <linux/input-event-codes.h>


void Entropist::runner(const std::string &deviceName)
{
#ifndef NDEBUG
  std::cout << "Opening " << deviceName << " ..." << std::endl;
#endif
  int fd = open(deviceName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    std::cerr << "ERROR: Cannot open " << deviceName << "." << std::endl;
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
    if (evt.type == EV_ABS || evt.type == EV_REL || evt.type == EV_KEY) {
      self().add(reinterpret_cast<uint8_t*>(&evt), sizeof(evt));
      self().output();
    }
  }
  close(fd);
}


constexpr unsigned int BITS_PER_LONG = sizeof(long) * 8;
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x) ((x)%BITS_PER_LONG)
#define BIT(x) (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)


void Entropist::findDevices(void)
{
#ifndef NDEBUG
  std::cout << "Searching for devices ..." << std::endl;
#endif
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir("/dev/input")) != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      if (std::string(ent->d_name).find("event") == 0)
      {
        unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
        char name[256];
        std::string fname = std::string("/dev/input/") + std::string(ent->d_name);
        mDetectedDevices[fname] = DeviceInfo();
#ifndef NDEBUG
        std::cout << std::endl << "> " << fname << std::endl;
#endif
        int fd = open(fname.c_str(), O_RDONLY);
        if (fd < 0)
        {
          std::cout << "ERROR: Could not open " << fname << std::endl;
        }
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        std::string devName(name);
        mDetectedDevices[fname].valid = true;
        mDetectedDevices[fname].name.resize(devName.size());
        std::transform(devName.begin(), devName.end(), mDetectedDevices[fname].name.begin(), ::tolower);
#ifndef NDEBUG
        std::cout << ">> Input device name: \"" <<  mDetectedDevices[fname].name << "\"" << std::endl;
#endif
        memset(bit, 0, sizeof(bit));
        ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
        if (test_bit(EV_KEY, bit[0]))
        {
#ifndef NDEBUG
            std::cout << ">>> KEYBOARD";
#endif
            mDetectedDevices[fname].maybeKeyboard = true;
            static const std::vector<int> MandatoryKeyCodes = {KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_KPASTERISK, KEY_SPACE};
            ioctl(fd, EVIOCGBIT(EV_KEY, KEY_MAX), bit[EV_KEY]);
            mDetectedDevices[fname].isKeyboard = std::all_of(
              MandatoryKeyCodes.begin(),
              MandatoryKeyCodes.end(),
              [&bit](int v) {
                return test_bit(v, bit[EV_KEY]);
              });
#ifndef NDEBUG
            if (mDetectedDevices[fname].isKeyboard)
            {
              std::cout << " FOR SURE !!!!!!!!!!!!!!!!!!!!!";
            }
            std::cout << std::endl;
#endif
        }
        if (test_bit(EV_ABS, bit[0]) || test_bit(EV_REL, bit[0]))
        {
          mDetectedDevices[fname].maybeMouse = true;
#ifndef NDEBUG
          std::cout << ">>> MOUSE";
#endif
          if (mDetectedDevices[fname].name.find("mouse") != std::string::npos)
          {
#ifndef NDEBUG
              std::cout << " FOR SURE !!!!!!!!!!!!!!!!!!!!!";
#endif
              mDetectedDevices[fname].isMouse = true;
          }
#ifndef NDEBUG
          std::cout << std::endl;
#endif
        }
      }
    }
    closedir(dir);
  }
  else
  {
    std::cerr << "ERROR: Could not open directory /dev/input." << std::endl;
  }
}

#endif
