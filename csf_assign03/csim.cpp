#include "cache.h"
#include "helper_functions.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  // Handel Cache configuration
  bool wrAlloc = false;
  bool wrThrough = false;
  bool Fifo = false;
  int isInvalid = checkArgValidity(argc, argv, &wrAlloc, &wrThrough, &Fifo);
  if (isInvalid != 0) {
    return isInvalid;
  }

  // store args as ints
  uint32_t numSets = stoul(argv[1]);
  uint32_t numBlocks = stoul(argv[2]);
  uint32_t blockSize = stoul(argv[3]);

  int logSets = logBase2(numSets);
  int logBlockSize = logBase2(blockSize);

  string trace; // stores each trace line

  vector<Block> blocks(numBlocks);
  map<uint32_t, uint32_t> m;
  Set s = {blocks, m};
  vector<Set> sets(numSets, s);
  Cache c = {sets, wrAlloc, wrThrough, Fifo, numSets, numBlocks};

  while (getline(cin, trace)) {
    uint32_t addr =
        stol(trace.substr(4, 8), 0, 16); // mem address from the trace
    uint32_t tag =
        addr >> (logBlockSize + logSets); // upper bits representing the tag
    uint32_t setIndex =
        addr << (32 -
                 (logBlockSize + logSets)); // lower bits representing the index
    setIndex = setIndex >> (32 - logSets);
    if (trace[0] == 'l') {
      operateL(&c, setIndex, tag, blockSize);
    } else {
      operateS(&c, setIndex, tag, blockSize);
    }
  }

  totalL();
  return 0;
}