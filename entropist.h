/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 */

#ifndef __ENTROPIST_H__
#define __ENTROPIST_H__

#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <Carbon/Carbon.h>

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
    if (total > THRESHOLD)
    {
      total = 0;
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
  static const int THRESHOLD = 128 * CryptoPP::SHA512::DIGESTSIZE;
  int total;
  bool hexOutput;
  std::ofstream out;
  std::thread thread;
  CryptoPP::SHA512 hash;
  CryptoPP::byte digest[CryptoPP::SHA512::DIGESTSIZE];

  static CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);
  static void runner(void);

private:
  Entropist(void)
    : total(0)
    , hexOutput(false)
  {
    // ...
  }
  Entropist(Entropist const &) = delete;
  void operator=(Entropist const &) = delete;
};

#endif // __ENTROPIST_H__
