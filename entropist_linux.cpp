// -*- coding: utf-8 -*-
// Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
// All rights reserved.

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


std::vector<std::string> split(const std::string &s, char seperator)
{
  std::vector<std::string> result;
  std::string::size_type prev_pos = 0, pos = 0;
  while((pos = s.find(seperator, pos)) != std::string::npos) {
    result.push_back(s.substr(prev_pos, pos-prev_pos));
    prev_pos = ++pos;
  }
  result.push_back(s.substr(prev_pos, pos-prev_pos));
  return result;
}

constexpr unsigned int BITS_PER_LONG = sizeof(long) * 8;
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)  ((array[LONG(bit)] >> OFF(bit)) & 1)

struct DeviceInfo {
  DeviceInfo(void)
  : isKeyboard(false)
  , isMouse(false)
  , valid(false)
  { /* ... */ }
  bool isKeyboard;
  bool isMouse;
  bool valid;
  std::string name;
};


void Entropist::findDevices(void)
{
  std::cout << "Searching for devices ..." << std::endl;
  DIR *dir;
  struct dirent *ent;
  std::map<std::string, DeviceInfo> usableDevices;
  if ((dir = opendir("/dev/input")) != NULL)
  {
    while ((ent = readdir(dir)) != NULL)
    {
      if (std::string(ent->d_name).find("event") == 0)
      {
        unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
        char name[256];
        std::string fname = std::string("/dev/input/") + std::string(ent->d_name);
        usableDevices[fname] = DeviceInfo();
        std::cout << std::endl << "> " << fname << std::endl;
        int fd = open(fname.c_str(), O_RDONLY);
        if (fd < 0)
        {
          std::cout << "ERROR: Could not open " << fname << std::endl;
        }
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        std::string devName(name);
        usableDevices[fname].valid = true;
        usableDevices[fname].name.resize(devName.size());
        std::transform(devName.begin(), devName.end(), usableDevices[fname].name.begin(), ::tolower);
        std::cout << ">> Input device name: \"" <<  usableDevices[fname].name << "\"" << std::endl;
        memset(bit, 0, sizeof(bit));
        ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);
        if (test_bit(EV_KEY, bit[0]))
        {
            std::cout << ">>> KEYBOARD";
            static const std::vector<int> MandatoryKeyCodes = {KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_KPASTERISK, KEY_SPACE};
            ioctl(fd, EVIOCGBIT(EV_KEY, KEY_MAX), bit[EV_KEY]);
            usableDevices[fname].isKeyboard = std::all_of(
              MandatoryKeyCodes.begin(),
              MandatoryKeyCodes.end(),
              [&bit](int v) {
                return test_bit(v, bit[EV_KEY]);
              });
            if (usableDevices[fname].isKeyboard)
            {
              std::cout << " FOR SURE !!!!!!!!!!!!!!!!!!!!!";
            }
            std::cout << std::endl;
        }
        if (test_bit(EV_ABS, bit[0]))
        {
          std::cout << ">>> MOUSE";
          if (usableDevices[fname].name.find("mouse") != std::string::npos)
          {
              std::cout << " FOR SURE !!!!!!!!!!!!!!!!!!!!!";
          }
          std::cout << std::endl;

          /*
          ioctl(fd, EVIOCGBIT(EV_ABS, KEY_MAX), bit[EV_ABS]);
          std::cout << ">>>> Event codes:" << std::endl;
          for (int j = 0; j < KEY_MAX; ++j)
          {
            if (test_bit(j, bit[EV_ABS]))
            {
              std::cout << std::dec << j << " 0x" << std::hex << std::setw(4) << std::setfill('0') << j << " ";
              std::cout << std::endl << ">>>>>ABS" << std::endl;
              int abs[5];
              ioctl(fd, EVIOCGABS(j), abs);
              for (int k = 0; k < 5; ++k)
              {
                if ((k < 3) || (abs[k] > 0))
                {
                  std::cout << std::dec << k << " => " << abs[k] << std::endl;
                }
              }
            }
          }
          std::cout << std::endl;
          */
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
