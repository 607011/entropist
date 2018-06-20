/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved.
 */

#ifndef __ENTROPIST_H__
#define __ENTROPIST_H__

#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <cryptopp/sha3.h>
#include <cryptopp/hex.h>
#include "entropy-test.h"

// Singleton
class Entropist {
public:
  static Entropist &instance(void)
  {
    static Entropist instance;
    return instance;
  }


  static Entropist &self(void)
  {
    return instance();
  }


  void run(void)
  {
    mThread = std::thread(Entropist::runner);
  }


#ifdef LINUX
  void setMouseInput(const std::string &in);
  void setKeyboardInput(const std::string &in);
  const std::string &mouseInput(void) const;
  const std::string &keyboardInput(void) const;
#endif 


  void join(void)
  {
    mThread.join();
  }  


  void setHexOutput(bool v)
  {
    mHexOutput = v;
  }


  bool setOutputFilename(const std::string &fname)
  {
    if (fname.size() > 0)
    {
      mOut.open(fname, std::fstream::out | std::fstream::binary | std::fstream::app);
      return mOut.is_open();
    }
    return false;
  }


  void add(const uint8_t *buf, int bufSize)
  {
    if (!mEntropyCalculated)
    {
      std::copy(buf, buf + bufSize, std::back_inserter(mEntropySamplePool));
    }
    mHash.Update(buf, bufSize);
    mTotalBits += mEntropyBitsPerByte * bufSize;
  }


  void output(void) {
    if (!mEntropyCalculated && mEntropySamplePool.size() > EntropySamplePoolSize)
    {
      mEntropyBitsPerByte = EntropySafetyFactor * calcEntropyBits(mEntropySamplePool);
      mEntropyCalculated = true;
    }
    if (mTotalBits > MinBitsForUpdate)
    {
      mTotalBits = 0;
      mHash.Final(mDigest);
      if (mHexOutput)
      {
        std::string hexDigest;
        CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hexDigest), false);
        encoder.Put(mDigest, sizeof(mDigest));
        encoder.MessageEnd();
        if (mOut.is_open())
        {
          mOut << hexDigest << std::flush;
        }
        else
        {
          std::cout << hexDigest << std::flush;
        }
      }
      else
      {
        if (mOut.is_open())
        {
          mOut << mDigest << std::flush;
        }
        else
        {
          std::cout << mDigest << std::flush;
        }
      }
    }
  }


  double entropyBitsPerByte(void) const
  {
    return mEntropyBitsPerByte;
  }


protected:
  static constexpr double MinBitsForUpdate = 8 * CryptoPP::SHA3_512::DIGESTSIZE;
  static constexpr double EntropySafetyFactor = 1.0 / 100;
  double mTotalBits;
  bool mHexOutput;
  std::ofstream mOut;
  std::thread mThread;
  CryptoPP::SHA3_512 mHash;
  uint8_t mDigest[CryptoPP::SHA3_512::DIGESTSIZE];

#ifdef LINUX
  std::string mMouseInput;
  std::string mKeyboardInput;
#endif

  static constexpr int EntropySamplePoolSize = 8192;
  std::vector<uint8_t> mEntropySamplePool;
  double mAvgEntropy;
  bool mEntropyCalculated;
  double mEntropyBitsPerByte;

  static void runner(void);

private:
  Entropist(void)
    : mTotalBits(0)
    , mHexOutput(false)
    , mAvgEntropy(1.0)
    , mEntropyCalculated(false)
    , mEntropyBitsPerByte(EntropySafetyFactor)
#ifdef LINUX
    , mMouseInput("/dev/input/event6")
    , mKeyboardInput("/dev/input/event2")
#endif
  {
    // ...
  }
  Entropist(Entropist const &) = delete;
  void operator=(Entropist const &) = delete;
};

#endif // __ENTROPIST_H__

