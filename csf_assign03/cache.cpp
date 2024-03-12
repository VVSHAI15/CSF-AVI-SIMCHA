#include "cache.h"
#include <iostream>
#include <limits>

using namespace std;

// Constructor
Cache::Cache(bool writeAllocate, bool writeThrough, bool isFifo,
             uint32_t setsCount, uint32_t blocksPerSet)
    : writeAllocate(writeAllocate), writeThrough(writeThrough), isFifo(isFifo),
      setsCount(setsCount), blocksPerSet(blocksPerSet), loadHits(0),
      loadMisses(0), saveHits(0), saveMisses(0), totalCycles(0), counter(0) {
  sets.resize(setsCount);
  for (auto &set : sets) {
    set.blocks.resize(blocksPerSet);
  }
}

// Load data from cache
void Cache::load(uint32_t setIndex, uint32_t tag, uint32_t blockSize) {
  Set &set = sets.at(setIndex);
  auto blockIter = set.index.find(tag);

  if (blockIter != set.index.end()) { // Cache hit
    if (!isFifo) {
      set.blocks[blockIter->second].timeStamp = counter++;
    }
    loadHits++;
  } else { // Cache miss
    handleMiss(set, tag, blockSize, true);
  }
}

// Store data to cache
void Cache::store(uint32_t setIndex, uint32_t tag, uint32_t blockSize) {
  Set &set = sets.at(setIndex);
  auto blockIter = set.index.find(tag);

  if (blockIter != set.index.end()) { // Store hit
    if (!isFifo) {
      set.blocks[blockIter->second].timeStamp = counter++;
    }
    if (writeThrough) {
      totalCycles += 101; // Memory write cycle
    } else {
      set.blocks[blockIter->second].isDirty = true;
    }
    ++saveHits;
  } else if (writeAllocate) { // Write miss with write-allocate
    handleMiss(set, tag, blockSize, false);
  } else { // Write miss without write-allocate
    saveMisses++;
    totalCycles += 101; // Memory write cycle
  }
  ++counter;
}

// Handle cache miss
void Cache::handleMiss(Set &set, uint32_t tag, uint32_t blockSize,
                       bool isLoad) {
  uint32_t minIndex = 0, minValue = numeric_limits<uint32_t>::max();
  bool foundInvalid = false;

  for (uint32_t i = 0; i < set.blocks.size(); ++i) {
    if (!set.blocks[i].valid) {
      replaceBlock(set, i, tag, blockSize, isLoad);
      foundInvalid = true;
      break;
    } else if (set.blocks[i].timeStamp < minValue) {
      minValue = set.blocks[i].timeStamp;
      minIndex = i;
    }
  }

  if (!foundInvalid) { // Replace LRU or FIFO block
    uint32_t oldTag = set.blocks[minIndex].tag;
    set.index.erase(oldTag);
    replaceBlock(set, minIndex, tag, blockSize, isLoad);
  }
}

// Replace a block within a set
void Cache::replaceBlock(Set &set, uint32_t index, uint32_t tag,
                         uint32_t blockSize, bool isLoad) {
  Block &block = set.blocks[index];
  if (block.isDirty) { // Update memory if dirty
    block.isDirty = false;
    totalCycles += blockSize / 4 * 100;
  }
  block.timeStamp = counter++;
  block.tag = tag;
  block.valid = true;
  set.index[tag] = index;

  if (isLoad) {
    loadMisses++;
    totalCycles += blockSize / 4 * 100;
  } else {
    block.isDirty = !writeThrough;
    saveMisses++;
    totalCycles += blockSize / 4 * 100 + 1;
  }
}

// Print cache statistics
void Cache::printResults() const {
  cout << "Total loads: " << loadHits + loadMisses << endl
       << "Total stores: " << saveHits + saveMisses << endl
       << "Load hits: " << loadHits << endl
       << "Load misses: " << loadMisses << endl
       << "Store hits: " << saveHits << endl
       << "Store misses: " << saveMisses << endl
       << "Total Cycles: " << totalCycles << endl;
}
