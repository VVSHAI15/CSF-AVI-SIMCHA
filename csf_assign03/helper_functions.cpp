#include "helper_functions.h"
#include "cache.h"
#include <iostream>
#include <string.h>

using namespace std;

bool isPowerOfTwo(int k) {
  bool n = k && !(k & (k - 1));
  return n;
}

bool isInt(const string &str) {
  return all_of(str.begin(), str.end(), ::isdigit);
}

int checkArgValidity(int argc, char *argv[], bool *wAlloc, bool *wThrough,
                     bool *isFifo) {
  // invalid # of args in command line
  if (argc != 7) {
    cerr << "Invalid number of args, should be 7" << endl;
    return 1;
  }
  // check if relevent arguements are integers
  if (!isInt(argv[1]) || !isInt(argv[2]) || !isInt(argv[3])) {
    cerr << "A relevent arg isn't of type integer" << endl;
    return 1;
  }
  // check if relevent args are power of two
  if (!isPowerOfTwo(stol(argv[1])) || !isPowerOfTwo(stol(argv[2])) ||
      !isPowerOfTwo(stol(argv[3])) || !(stol(argv[3]) >= 4)) {
    cerr << "A relevent arg isn't a power of two" << endl;
    return 1;
  }

  // check strings for Cache configurations and reset flags if needed

  if (strcmp(argv[4], "write-allocate") == 0) {
    *wAlloc = true;
  } else if (strcmp(argv[4], "no-write-allocate") != 0) {
    cerr << "Invalid fourth arg" << endl;
    return 1;
  }

  if (strcmp(argv[5], "write-through") == 0) {
    *wThrough = true;
  } else if (strcmp(argv[5], "write-back") != 0) {
    cerr << "Invalid fifth argument" << endl;
    return 1;
  }

  if (strcmp(argv[6], "fifo") == 0) {
    *isFifo = true;
  } else if (strcmp(argv[6], "lru") != 0) {
    cerr << "Invalid fifth argument" << endl;
    return 1;
  }

  if (!*wAlloc && !*wThrough) {
    cerr << "Invalid arg combination" << endl;
    return 1;
  }

  return 0;
}

int logBase2(uint32_t value) {
  uint32_t result = 0;
  while (value >>= 1) { // Shift right until value becomes 0
    result++;
  }
  return result;
}
