#include "cache.h" // Ensure this is correctly named as per your file structure, case-sensitive
// #include "helper_functions.h" // Assuming this contains the checkArgValidity
// function
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>

using namespace std;

// used
// http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
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

// used http://www.graphics.stanford.edu/~seander/bithacks.html
int logBase2(uint32_t value) {
  uint32_t result = 0;
  while (value >>= 1) { // Shift right until value becomes 0
    result++;
  }
  return result;
}

int main(int argc, char *argv[]) {
  // Handle Cache configuration
  bool wrAlloc = false, wrThrough = false;
  bool Fifo = false;
  int isInvalid = checkArgValidity(argc, argv, &wrAlloc, &wrThrough, &Fifo);
  if (isInvalid != 0) {
    return isInvalid;
  }

  // Store args as ints
  uint32_t numSets = stoul(argv[1]);
  uint32_t numBlocks = stoul(argv[2]);
  uint32_t blockSize = stoul(argv[3]);

  int logSets = logBase2(numSets);
  int logBlockSize = logBase2(blockSize);

  std::vector<Block> blocks(numBlocks);
  std::map<uint32_t, uint32_t> m;
  Set s = {blocks, m};
  vector<Set> sets(numSets, s);
  Cache c = Cache(wrAlloc, wrThrough, Fifo, numSets, numBlocks);

  string trace;

  while (getline(cin, trace)) {

    // read the line and get the values
    uint32_t addr = stol(trace.substr(2, 10), 0, 16);
    // get the tag and the index using bitwise operations
    uint32_t tag = addr >> (logBlockSize + logSets);
    uint32_t setIndex = addr << (32 - (logBlockSize + logSets));
    setIndex = setIndex >> (32 - logSets);

    if (logSets == 0)
      setIndex = 0;
    // operate load or save accordingly
    // cout << "xinan isnt cute" << endl;
    // cout << trace << endl;

    if (trace[0] == 's') {
      operateS(&c, setIndex, tag, blockSize);

    } else {
      operateL(&c, setIndex, tag, blockSize);
    }
  }

  printResults(c);
  return 0;
}
