#include "cache.h" // Ensure this is correctly named as per your file structure, case-sensitive
#include "helper_functions.h" // Assuming this contains the checkArgValidity function
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

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

  // Initialize Cache instance with the provided configurations
  // Cache cache(numSets, numBlocks, blockSize, wrAlloc, wrThrough,
  // Fifo); // Correctly instantiate 'Cache' object

  /*
    uint32_t addr = stol("0x0000AA40");
    uint32_t tag = addr >> (logBlockSize + logSets);
    uint32_t setIndex = addr << (32 - (logBlockSize + logSets));
    cache.load(addr, tag, setIndex);
  */

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

  /*
    // Output cache statistics
    cout << "Total loads: " << cache.getTotalLoads() << "\n";
    cout << "Total stores: " << cache.getTotalStores() << "\n";
    cout << "Load hits: " << cache.getLoadHits() << "\n";
    cout << "Load misses: " << cache.getLoadMisses() << "\n";
    cout << "Store hits: " << cache.getStoreHits() << "\n";
    cout << "Store misses: " << cache.getStoreMisses() << "\n";
    cout << "Total cycles: " << cache.getTotalCycles() << "\n";
    */
  totalL(c);
  return 0;
}
