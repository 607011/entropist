/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 */

#include "entropist.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
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
  std::vector<std::string> output;
  std::string::size_type prev_pos = 0, pos = 0;
  while((pos = s.find(seperator, pos)) != std::string::npos) {
    std::string substring(s.substr(prev_pos, pos-prev_pos));
    output.push_back(substring);
    prev_pos = ++pos;
  }
  output.push_back(s.substr(prev_pos, pos-prev_pos));
  return output;
}

void Entropist::findDevices(void)
{
  std::cout << "Searching devices ..." << std::endl;
  std::ifstream devicesInput;
  std::string deviceListFilename = "/proc/bus/input/devices";
  devicesInput.open(deviceListFilename);
  std::vector<std::string> deviceNames = {"keyboard", "mouse"};
  if (!devicesInput.is_open())
  {
    std::cerr << "Cannot open " << deviceListFilename << " ..." << std::endl;
    return;
  }
  std::string line;
  while (getline(devicesInput, line))
  {
    std::transform(line.begin(), line.end(), line.begin(), ::tolower);
    std::size_t pos;
    pos = line.find("name=", 0);
    if (pos != std::string::npos) {
      for (auto devName : deviceNames)
      {
        pos = line.find(devName, 0);
	if (pos != std::string::npos) {
	  std::cout << line << std::endl;
	  std::string line2;
	  while (getline(devicesInput, line2)) {
            std::transform(line2.begin(), line2.end(), line2.begin(), ::tolower);
	    pos = line2.find("handlers=", 0);
	    if (pos != std::string::npos) {
	      std::cout << line2 << std::endl;
	      std::vector<std::string> hsplit = split(line2, '=');
	      if (hsplit.size() > 1) {
		const std::vector<std::string> &handlers = split(hsplit.at(1), ' ');
	        for (auto h : handlers) {
		  std::cout << ">>>" << h << "<<<" << std::endl;
		}
	      }
	      break;
	    }
	  }
	}
      }
    }
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
