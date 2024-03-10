#include "cache.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#ifndef __HELPERS_H__
#define __HELPERS_H__

using namespace std;

// used
// http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
bool isPowerOfTwo(int k);

bool isInt(const string &str);

int checkArgValidity(int argc, char *argv[], bool *wAlloc, bool *wThrough,
                     bool *isFifo);

// used http://www.graphics.stanford.edu/~seander/bithacks.html
int logBase2(uint32_t value);

#endif