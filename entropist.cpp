/* Generate random numbers from global key events (macOS version).
 *
 * Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved. 
 */

#include <iostream>
#include <string>
#include <thread>
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>


class TypingEntropist {
public:
  ~TypingEntropist() {}

  static TypingEntropist &instance(void)
  {
    static TypingEntropist instance;
    return instance;
  }

  void run(void);
  void join(void);

protected:
  static const int THRESHOLD = 128 * CryptoPP::SHA512::DIGESTSIZE;
  int total;
  std::thread thread;
  CryptoPP::SHA512 hash;
  CryptoPP::byte digest[CryptoPP::SHA512::DIGESTSIZE];

  static CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);
  static void runner(void);

private:
  TypingEntropist(void);
  TypingEntropist(TypingEntropist const &) = delete;
  void operator=(TypingEntropist const &) = delete;
};


TypingEntropist::TypingEntropist(void)
  : total(THRESHOLD)
{
  // ...
}


void TypingEntropist::run(void)
{
  thread = std::thread(TypingEntropist::runner);
}


void TypingEntropist::join(void)
{
  thread.join();
}


CGEventRef TypingEntropist::eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
  if (type == kCGEventKeyUp)
  {
    CGKeyCode keycode = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    CGEventFlags flags = static_cast<CGEventFlags>(CGEventGetFlags(event));
    CGEventTimestamp timestamp = CGEventGetTimestamp(event);
    instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&timestamp), sizeof(timestamp) / sizeof(CryptoPP::byte));
    instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&keycode), sizeof(keycode) / sizeof(CryptoPP::byte));
    instance().hash.Update(reinterpret_cast<CryptoPP::byte *>(&flags), sizeof(flags) / sizeof(CryptoPP::byte));
    instance().total -= sizeof(timestamp) + sizeof(keycode) + sizeof(flags);
    if (instance().total < 0)
    {
      instance().total = THRESHOLD;
      instance().hash.Final(instance().digest);
      std::string hexDigest;
      CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(hexDigest), false);
      encoder.Put(instance().digest, sizeof(digest));
      encoder.MessageEnd();
      std::cout << "\r" << hexDigest << std::flush;
    }
  }
  return event;
}


void TypingEntropist::runner(void)
{
  CGEventMask eventMask = 1 << kCGEventKeyUp;
  CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, eventMask, eventCallback, NULL);
  if (!eventTap) {
      std::cerr << "failed to create event tap" << std::endl;
      return;
  }
  CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
  CGEventTapEnable(eventTap, true);
  CFRunLoopRun();
}


int main(void)
{
  std::cout << "Launching typing entropist ..." << std::endl;
  TypingEntropist &e = TypingEntropist::instance();
  e.run();
  e.join();
  return 0;
}
