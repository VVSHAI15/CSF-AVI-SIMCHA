#include "cache.h"
#include <iostream>
#include <limits>

using namespace std;

// Constructor
Cache::Cache(bool writeAllocate, bool writeThrough, bool isLRU,
             uint32_t setsCount, uint32_t blocksPerSet)
    : writeAllocate(writeAllocate), writeThrough(writeThrough), isLRU(isLRU),
      setsCount(setsCount), blocksPerSet(blocksPerSet), loadHits(0),
      loadMisses(0), saveHits(0), saveMisses(0), totalCycles(0), counter(0) {
  sets.resize(
      setsCount); // Resize the vector of sets according to the number of sets
  for (auto &set : sets) {
    set.blocks.resize(blocksPerSet); // Resize each set's block vector to the
                                     // number of blocks per set
  }
}

// Load data from cache
void Cache::load(uint32_t setIndex, uint32_t tag, uint32_t blockSize) {
  Set &set = sets.at(setIndex);
  auto blockIter = set.index.find(tag); // Look for the block with the given tag

  if (blockIter != set.index.end()) { // Cache hit
    if (isLRU) {                      // Update timestamp only for LRU policy
      set.blocks[blockIter->second].timeStamp = counter++;
    }
    loadHits++; // Increment the counter
  } else {      // Cache miss
    handleMiss(set, tag, blockSize, true);
  }
}

// Store data to cache
void Cache::store(uint32_t setIndex, uint32_t tag, uint32_t blockSize) {
  Set &set = sets.at(setIndex);
  auto blockIter = set.index.find(tag); // Look for the block with the given tag

  if (blockIter != set.index.end()) { // Store hit
    if (isLRU) {                      // Update timestamp only for LRU policy
      set.blocks[blockIter->second].timeStamp = counter++;
    }
    if (writeThrough) {
      totalCycles += 101; // Memory write cycle
    } else {
      set.blocks[blockIter->second].isDirty =
          true; // Mark block as dirty for write-back
    }
    ++saveHits;
  } else if (writeAllocate) { // Write miss with write-allocate
    handleMiss(set, tag, blockSize, false);
  } else { // Write miss without write-allocate
    saveMisses++;
    totalCycles += 101; // Memory write cycle
  }
  ++counter; // Increment the LRU counter
}

// Handle cache miss
void Cache::handleMiss(Set &set, uint32_t tag, uint32_t blockSize,
                       bool isLoad) {
  uint32_t minIndex = 0, minValue = std::numeric_limits<uint32_t>::max();
  bool foundInvalid = false; // Flag to check if an invalid block was found
  uint32_t index = 0;        // Used to track the current index within the loop

  // Loop through blocks to find an invalid block or the LRU block
  for (auto it = set.blocks.begin(); it != set.blocks.end(); ++it, ++index) {
    if (!it->valid) { // Found an invalid block
      replaceBlock(set, index, tag, blockSize,
                   isLoad); // Replace the invalid block
      foundInvalid = true;
      break; // Exit the loop after replacing an invalid block
    } else if (it->timeStamp < minValue) { // Update LRU information
      minValue = it->timeStamp;
      minIndex = index;
    }
  }

  if (!foundInvalid) { // No invalid block found, replace LRU or FIFO block
    uint32_t oldTag = set.blocks[minIndex].tag;
    set.index.erase(oldTag); // Remove the old block from the index
    replaceBlock(set, minIndex, tag, blockSize,
                 isLoad); // Replace the LRU/FIFO block
  }
}

// Replace a block within a set
void Cache::replaceBlock(Set &set, uint32_t index, uint32_t tag,
                         uint32_t blockSize, bool isLoad) {
  Block &block = set.blocks[index];
  if (block.isDirty) { // Update memory if dirty
    block.isDirty = false;
    totalCycles += blockSize / 4 * 100; // Simulate memory write back time
  }
  block.timeStamp = counter++; // Update the block's timestamp for LRU
  block.tag = tag;             // Set the new tag for the block
  block.valid = true;          // Mark the block as valid
  set.index[tag] = index;      // Update the set's index with the new block

  if (isLoad) {   // If this replacement is due to a load operation
    loadMisses++; // Increment load misses counter
    totalCycles += blockSize / 4 * 100;
  } else { // This replacement is due to a store operation
    block.isDirty = !writeThrough;
    saveMisses++; // Increment store misses counter
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
