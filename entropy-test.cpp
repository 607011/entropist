/* Copyright (c) 2018 Oliver Lau <oliver@ersatzworld.net>
 * All rights reserved. 
 */

#include "entropy-test.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

double calcEntropyBits(const std::vector<uint8_t> &buf)
{
  std::vector<int> histo(256, 0);
  for (auto x : buf)
  {
    ++histo[x];
  }
  double ent = 0.0;
  const double n = double(buf.size());
  for (auto h : histo)
  {
    const double p = double(h) / n;
    if (p > 0)
    {
      ent += p * M_LOG2E * log(1.0 / p);
    }
  }
  return ent;
}
