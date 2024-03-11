#include "cache.h"
// #include "helper_functions.h"
#include <iostream>

using namespace std;

Cache::Cache(bool wrAlloc, bool wrThrough, bool Fifo, uint32_t numSets,
             uint32_t numBlocks)
    : wrAlloc(wrAlloc), wrThrough(wrThrough), Fifo(Fifo), numSets(numSets),
      numBlocks(numBlocks) {
  sets.resize(numSets); // Ensure there are 'numSets' sets
  for (auto &set : sets) {
    set.blocks.resize(numBlocks); // Ensure each set has 'numBlocks' blocks
  }
}

void operateL(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size) {
  // Access the set within the cache using the set index
  Set &s = c->sets.at(setIndex);

  // Attempt to find the block with the given tag in the set
  auto i = s.index.find(tag);

  // If a block with the tag exists (cache hit)
  if (i != s.index.end()) {
    // Retrieve the index of the block within the set
    uint32_t blockIndx = i->second;

    // If using FIFO, no need to update the timestamp; otherwise, update it for
    // LRU
    if (!c->Fifo) {
      s.blocks.at(blockIndx).timeStamp = c->counter++;
    }

    // Increment load hit count
    c->loadHits++;
  } else {
    // Initialize variables to find either an invalid block or the least
    // recently used one
    uint32_t minIndx = 0;
    uint32_t minVal =
        UINT32_MAX; // Use UINT32_MAX for maximum unsigned int value

    // Iterate over all blocks in the set
    for (uint32_t i = 0; i < s.blocks.size(); i++) {
      // If a block is valid, check if it has the minimum timestamp
      if (s.blocks.at(i).valid) {
        if (minVal > s.blocks.at(i).timeStamp) {
          minVal = s.blocks.at(i).timeStamp;
          minIndx = i;
        }
      } else {
        // If found an invalid block, replace it and return immediately
        replaceBlock(i, tag, &s.blocks.at(i), &s, true, c->wrThrough,
                     block_size, c);
        return;
      }
    }

    // If no invalid block is found, replace the block with the minimum
    // timestamp (least recently used or FIFO order)
    uint32_t tmp = s.blocks.at(minIndx).tag;
    replaceBlock(minIndx, tag, &s.blocks.at(minIndx), &s, true, c->wrThrough,
                 block_size, c);

    // Remove the old tag-to-index mapping as it has been replaced
    s.index.erase(tmp);
  }
}

void replaceBlock(uint32_t index, uint32_t tag, Block *b, Set *s, bool is_load,
                  bool w_through, uint32_t size, Cache *c) {
  // if old block is dirty first we need to update the memory
  if (b->isDirty) {
    b->isDirty = false;
    c->totalCycles += size / 4 * 100;
  }
  if (!is_load) {
    // set if the block is dirty or not depending on whether cache is write
    // through or not
    b->isDirty = !w_through;
    c->saveMisses++;
    c->totalCycles += ((size) / 4 * 100) + 1;
  }
  if (is_load) {
    c->loadMisses++;
    c->totalCycles += size / 4 * 100;
  }
  // update the new block to hold the values from the new block
  b->timeStamp = c->counter++;
  // update the map so new tag points to this index
  s->index.insert(std::pair<uint32_t, uint32_t>(tag, index));
  b->tag = tag;
  b->valid = true;
}

void operateS(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size) {
  // Access the set within the cache using the set index
  Set &s = c->sets.at(setIndex);

  // Attempt to find the block with the given tag in the set
  auto i = s.index.find(tag);

  // If a block with the tag exists (store hit)
  if (i != s.index.end()) {
    // Retrieve the index of the block within the set
    uint32_t blockIndx = i->second;

    // If not using FIFO (e.g., LRU), update the block's timestamp
    if (!c->Fifo) {
      s.blocks.at(blockIndx).timeStamp = c->counter++;
    }

    // Handling write policy
    if (c->wrThrough) {
      // For write-through, immediately increment total cycles for memory write
      c->totalCycles += 101;
    } else {
      // Mark the block as dirty for write-back
      s.blocks.at(blockIndx).isDirty = true;
    }
    // Increment store hit count
    c->saveHits++;
  } else if (c->wrAlloc) {
    // For write-allocate, find a block to replace
    uint32_t minIndx = 0, minVal = UINT32_MAX;

    // Search for either an invalid or the least recently used block
    for (uint32_t i = 0; i < s.blocks.size(); i++) {
      if (s.blocks.at(i).valid && minVal > s.blocks.at(i).timeStamp) {
        minVal = s.blocks.at(i).timeStamp;
        minIndx = i;
      } else if (!s.blocks.at(i).valid) {
        // If an invalid block is found, replace it directly
        replaceBlock(i, tag, &s.blocks.at(i), &s, false, c->wrThrough,
                     block_size, c);
        return;
      }
    }

    // Replace the least recently used or first invalid block found
    uint32_t tmp = s.blocks.at(minIndx).tag;
    s.index.erase(tmp);
    replaceBlock(minIndx, tag, &s.blocks.at(minIndx), &s, false, c->wrThrough,
                 block_size, c);
  } else {
    // For no-write-allocate, simply update cycle count for memory write
    c->saveMisses++;
    c->totalCycles += 101;
  }
  // Increment operation counter
  c->counter++;
}

// Get functions --> possible for extra functionality without changing the
// counters themselves
uint32_t getCacheCounter(const Cache &cache) { return cache.counter; }
uint32_t getCacheLoadMisses(const Cache &cache) { return cache.loadMisses; }
uint32_t getCacheLoadHits(const Cache &cache) { return cache.loadHits; }
uint32_t getCacheSaveHits(const Cache &cache) { return cache.saveHits; }
uint32_t getCacheSaveMisses(const Cache &cache) { return cache.saveMisses; }
uint32_t getCacheMultipliedCycles(const Cache &cache) {
  return cache.multipliedCycles;
}
uint32_t getCacheTotalCycles(const Cache &cache) { return cache.totalCycles; }

void printResults(const Cache &cache) {
  cout << "Total loads: " << getCacheLoadHits(cache) + getCacheLoadMisses(cache)
       << endl;
  cout << "Total stores: "
       << getCacheSaveHits(cache) + getCacheSaveMisses(cache) << endl;
  cout << "Load hits: " << getCacheLoadHits(cache) << endl;
  cout << "Load misses: " << getCacheLoadMisses(cache) << endl;
  cout << "Store hits: " << getCacheSaveHits(cache) << endl;
  cout << "Store misses: " << getCacheSaveMisses(cache) << endl;
  cout << "Total Cycles: " << getCacheTotalCycles(cache) << endl;
}