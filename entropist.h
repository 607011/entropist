/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 */

#ifndef __ENTROPIST_H__
#define __ENTROPIST_H__

#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <cryptopp/sha3.h>
#include <cryptopp/hex.h>

#ifdef MACOS
#include <Carbon/Carbon.h>
#endif


class Entropist {
public:
  ~Entropist() {}

  static Entropist &instance(void)
  {
    static Entropist instance;
    return instance;
  }


  void run(void)
  {
    thread = std::thread(Entropist::runner);
  }

#ifdef LINUX
  void setMouseInput(const std::string &in)
  {
    mouseInput = in;
  }

  void setKeyboardInput(const std::string &in)
  {
    keyboardInput = in;
  }
#endif 


  void join(void)
  {
    thread.join();
  }  


  void setHexOutput(bool v)
  {
    hexOutput = v;
  }


  bool setOutputFilename(const std::string &fname)
  {
    if (fname.size() > 0)
    {
      out.open(fname, std::fstream::out | std::fstream::binary | std::fstream::app);
      return out.is_open();
    }
    return false;
  }


  void output(void) {
    if (totalBits > MIN_BITS)
    {
      totalBits = 0;
      hash.Final(digest);
      if (hexOutput)
      {
        std::string hexDigest;
        CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hexDigest), false);
        encoder.Put(instance().digest, sizeof(digest));
        encoder.MessageEnd();
        if (out.is_open())
        {
          out << hexDigest << std::flush;
        }
        else
        {
          std::cout << hexDigest << std::flush;
        }
      }
      else
      {
        if (out.is_open())
        {
          out << digest << std::flush;
        }
        else
        {
          std::cout << digest << std::flush;
        }
      }
    }
  }


protected:
  static const int MIN_BITS = 8 * CryptoPP::SHA3_512::DIGESTSIZE;
  int totalBits;
  bool hexOutput;
  std::ofstream out;
  std::thread thread;
  CryptoPP::SHA3_512 hash;
  uint8_t digest[CryptoPP::SHA3_512::DIGESTSIZE];

#ifdef LINUX
  std::string mouseInput;
  std::string keyboardInput;
#endif

  static void runner(void);

#ifdef MACOS
  static CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);
#endif

private:
  Entropist(void)
    : totalBits(0)
    , hexOutput(false)
#ifdef LINUX
    , mouseInput("/dev/input/event6")
    , keyboardInput("/dev/input/event2")
#endif
  {
    // ...
  }
  Entropist(Entropist const &) = delete;
  void operator=(Entropist const &) = delete;
};

#endif // __ENTROPIST_H__

