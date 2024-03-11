#ifndef __CACHE_H__
#define __CACHE_H__

#include <cstdint>
#include <map>
#include <vector>
struct Block {
  uint32_t tag{0};
  bool valid{false};
  uint32_t timeStamp{0};
  bool isDirty{false};
};

struct Set {
  std::vector<Block> blocks;
  std::map<uint32_t, uint32_t> index; // map of tag to index of slot
};

struct Cache {
  std::vector<Set> sets;
  bool wrAlloc;
  bool wrThrough;
  bool Fifo;
  uint32_t numSets;
  uint32_t numBlocks;

  uint32_t counter = 0;
  uint32_t loadMisses = 0;
  uint32_t loadHits = 0;
  uint32_t saveHits = 0;
  uint32_t saveMisses = 0;
  uint32_t multipliedCycles = 0;
  uint32_t totalCycles = 0;

  Cache(bool wrAlloc, bool wrThrough, bool Fifo, uint32_t numSets,
        uint32_t numBlocks);
};

// operates a storing operation
// Parameters:
// c - the cache we are storing to
// setIndex  - index of the set
// tag   - tag of the block
void operateS(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size);

// operates a loading operation
// Parameters:
// c - the cache we are loading from
// setIndex  - index of the set
// tag   - tag of the block
void operateL(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size);

// replaces a block
// Parameters:
// index - index of the block to be replaced
// tag   - tag of the new block
// b     - block to be replaced
// set   - set the block is in
// is_load - true if the operation is a load
// w_through - true if the cache is write through
// block_size - size of the block
void replaceBlock(uint32_t index, uint32_t tag, Block *b, Set *s, bool is_load,
                  bool w_through, uint32_t block_size, Cache *c);

uint32_t getCacheCounter(const Cache &cache);
uint32_t getCacheLoadMisses(const Cache &cache);
uint32_t getCacheLoadHits(const Cache &cache);
uint32_t getCacheSaveHits(const Cache &cache);
uint32_t getCacheSaveMisses(const Cache &cache);
uint32_t getCacheMultipliedCycles(const Cache &cache);
uint32_t getCacheTotalCycles(const Cache &cache);

// prints out the misses, hits and total loads
// Parameters:
// block_size - block size of the cache
void printResults(const Cache &cache);

#endif