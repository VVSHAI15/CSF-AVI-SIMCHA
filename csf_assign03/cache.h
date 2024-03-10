
#ifndef __CACHE_H__
#define __CACHE_H__

#include <cstdint>
#include <map>
#include <vector>

struct Block {
  uint32_t tag{0};
  bool valid{false};
  uint32_t timeStamp{0};
  bool is_dirty{false};
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
};

// tag   - tag of the block
void operateS(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size);

void operateL(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size);

void totalL();

void replace_block(uint32_t index, uint32_t tag, Block *b, Set *s, bool is_load,
                   bool w_through, uint32_t block_size);

#endif