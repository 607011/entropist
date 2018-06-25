// -*- coding: utf-8 -*-
// Generate random numbers from global key events (macOS version).
// Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
// All rights reserved.

#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include <cryptopp/sha3.h>
#include <cryptopp/hex.h>
#include "entropist.h"
#include "optionparser.h"

enum  optionIndex { UNKNOWN, HELP, OUTPUT, HEX, VERBOSE };

struct Arg: public option::Arg
{
  static void printError(const char* msg1, const option::Option& opt, const char* msg2)
  {
    std::cerr << msg1 << opt.name << msg2;
  }
  static option::ArgStatus Required(const option::Option& option, bool msg)
  {
    if (option.arg != 0)
      return option::ARG_OK;
    if (msg)
      printError("Option '", option, "' requires an argument\n");
    return option::ARG_ILLEGAL;
  }
};

const option::Descriptor usage[] =
{
  {UNKNOWN, 0, "", "", option::Arg::NoArg, "USAGE: entropist [options]\n\nOptions:" },
  {HELP, 0, "?" , "help", option::Arg::NoArg, "  --help  \tPrint usage and exit." },
  {HEX, 0, "" , "hex", option::Arg::NoArg, "  --hex  \tOutput hexadecimal data instead of binary." },
  {OUTPUT, 0, "o", "output", Arg::Required, "  --output=<arg>, -o <arg>  \tSet output filename." },
  {VERBOSE, 0, "v" , "", option::Arg::NoArg, "  -v  \tVerbose output." },
  {UNKNOWN, 0, "", "" , option::Arg::NoArg,
    "\nExamples:\n"
    "  entropist\n"
    "  entropist -v\n"
    "  entropist -o myentropy.hex --hex\n"
    "  entropist --output myrandomdata.bin\n" },
  {0, 0, 0, 0, 0, 0}
};

bool verbose = false;

int main(int argc, char *argv[])
{
#ifdef LINUX
  if (getuid() != 0)
  {
    std::cerr << "This program must be run as root." << std::endl;
    return EXIT_FAILURE;
  }
#endif  

  if (argc > 0)
  {
    --argc;
    ++argv;
  }
  Entropist &e = Entropist::instance();
  option::Stats stats(usage, argc, argv);
  option::Option* options = reinterpret_cast<option::Option* >(calloc(stats.options_max, sizeof(option::Option)));
  option::Option* buffer = reinterpret_cast<option::Option* >(calloc(stats.buffer_max,  sizeof(option::Option)));
  option::Parser parse(usage, argc, argv, options, buffer);

  bool allOptionsValid = true;

  for (int i = 0; i < parse.optionsCount() && allOptionsValid; ++i)
  {
    option::Option& opt = buffer[i];
    switch (opt.index())
    {
      case OUTPUT:
        if (opt.arg)
        {
          bool ok = e.setOutputFilename(opt.arg);
          if (!ok)
          {
            std::cerr << "Output file '" << opt.arg << "' cannot be opened." << std::endl;
            return EXIT_FAILURE;
          }
        }
        break;
      case VERBOSE:
        verbose = true;
        break;
      case HEX:
        e.setHexOutput(true);
        break;
      case UNKNOWN:
        // fall-through
      default:
        std::cerr << "Unknown option '" << opt.name << "'." << std::endl;
        allOptionsValid = false;
        break;
    }
  }
  if (options[HELP] || parse.error() || !allOptionsValid)
  {
    int columns = getenv("COLUMNS")? atoi(getenv("COLUMNS")) : 80;
    option::printUsage(fwrite, stdout, usage, columns);
    return parse.error() ? EXIT_FAILURE : EXIT_SUCCESS;
  }
  
#ifdef MACOS
  if (getuid() != 0)
  {
    std::cerr << "*** You should run this program as root to make the most of it,\n"
      "*** i.e. leverage key press events to collect more entropy." << std::endl;
  }
#endif


#ifdef LINUX
  e.findDevices();
#endif

  e.run();
  e.join();
  return EXIT_SUCCESS;
}
