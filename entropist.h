// -*- coding: utf-8 -*-
// Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
// All rights reserved.


#ifndef __ENTROPIST_H__
#define __ENTROPIST_H__

#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <mutex>
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
#ifdef LINUX
    for (auto const &dev : mDetectedDevices)
    {
      if (dev.second.maybeMouse || dev.second.isKeyboard)
      {
        mThreads.push_back(std::thread(Entropist::runner, dev.first));
      }
    }
#else
    mThread  = std::thread(Entropist::runner);
#endif
  }


  void join(void)
  {
#ifdef LINUX
    for (auto &&t : mThreads)
    {
      t.join();
    }
#else
    mThread.join();
#endif
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
    mAddMutex.lock();
    if (!mEntropyCalculated)
    {
      std::copy(buf, buf + bufSize, std::back_inserter(mEntropySamplePool));
    }
    mHash.Update(buf, bufSize);
    mTotalBits += mEntropyBitsPerByte * bufSize;
    mAddMutex.unlock();
  }


  void output(void) {
    mAddMutex.lock();
    if (!mEntropyCalculated && mEntropySamplePool.size() > EntropySamplePoolSize)
    {
      const double ent = calcEntropyBits(mEntropySamplePool);
      mEntropyBitsPerByte = EntropySafetyFactor * ent;
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
    mAddMutex.unlock();
  }


  double entropyBitsPerByte(void) const
  {
    return mEntropyBitsPerByte;
  }


#ifdef LINUX
  void findDevices(void);
#endif


protected:
  static constexpr double MinBitsForUpdate = 8 * CryptoPP::SHA3_512::DIGESTSIZE;
  static constexpr double EntropySafetyFactor = 1.0 / 100;
  double mTotalBits;
  bool mHexOutput;
  std::ofstream mOut;
#ifdef LINUX
  std::vector<std::thread> mThreads;
#else
  std::thread mThread;
#endif
  CryptoPP::SHA3_512 mHash;
  uint8_t mDigest[CryptoPP::SHA3_512::DIGESTSIZE];

  static constexpr int EntropySamplePoolSize = 8192;
  std::vector<uint8_t> mEntropySamplePool;
  double mAvgEntropy;
  bool mEntropyCalculated;
  double mEntropyBitsPerByte;
  std::mutex mAddMutex;

#ifdef LINUX
  struct DeviceInfo {
    DeviceInfo(void)
    : maybeKeyboard(false)
    , isKeyboard(false)
    , maybeMouse(false)
    , isMouse(false)
    , valid(false)
    { /* ... */ }
    bool maybeKeyboard;
    bool isKeyboard;
    bool maybeMouse;
    bool isMouse;
    bool valid;
    std::string name;
  };
  std::map<std::string, DeviceInfo> mDetectedDevices;
#endif

#ifdef LINUX
  static void runner(const std::string &deviceName);
#else
  static void runner(void);
#endif

private:
  Entropist(void)
    : mTotalBits(0)
    , mHexOutput(false)
    , mAvgEntropy(1.0)
    , mEntropyCalculated(false)
    , mEntropyBitsPerByte(EntropySafetyFactor)
  {
#ifdef LINUX
    findDevices();
#endif
  }
  Entropist(Entropist const &) = delete;
  void operator=(Entropist const &) = delete;
};

#endif // __ENTROPIST_H__

