#ifndef CACHE_H
#define CACHE_H

#include <cstdint>
#include <unordered_map>
#include <vector>

// Define a structure for a cache block
struct Block {
  uint32_t tag = 0;
  bool valid = false;
  bool isDirty = false;
  uint32_t timeStamp = 0;
};

// Define a structure for a cache set
struct Set {
  std::vector<Block> blocks;
  std::unordered_map<uint32_t, uint32_t> index; // Maps tags to block indices
};

class Cache {
public:
  // Constructor
  Cache(bool writeAllocate, bool writeThrough, bool isFifo, uint32_t setsCount,
        uint32_t blocksPerSet);

  // Load data from cache
  void load(uint32_t setIndex, uint32_t tag, uint32_t blockSize);

  // Store data to cache
  void store(uint32_t setIndex, uint32_t tag, uint32_t blockSize);

  // Print cache statistics
  void printResults() const;

private:
  bool writeAllocate;
  bool writeThrough;
  bool isFifo;
  uint32_t setsCount;
  uint32_t blocksPerSet;
  uint32_t loadHits;
  uint32_t loadMisses;
  uint32_t saveHits;
  uint32_t saveMisses;
  uint64_t totalCycles;
  uint32_t counter;
  std::vector<Set> sets;

  // Handle cache miss
  void handleMiss(Set &set, uint32_t tag, uint32_t blockSize, bool isLoad);

  // Replace a block within a set
  void replaceBlock(Set &set, uint32_t index, uint32_t tag, uint32_t blockSize,
                    bool isLoad);
};

#endif // CACHE_H
