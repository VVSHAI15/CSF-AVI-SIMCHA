
#include "cache.h"
#include "helper_functions.h"
#include <iostream>

uint32_t counter = 0;
uint32_t loadMisses = 0;
uint32_t loadHits = 0;
uint32_t save_hits = 0;
uint32_t save_misses = 0;
uint32_t multiplied_cycles = 0;
uint32_t total_cycles = 0;

void operateL(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size) {
  Set &s = c->sets.at(setIndex);
  auto i = s.index.find(tag);
  if (i != s.index.end()) {
    // if hit update accordingly
    uint32_t index_of_slot = i->second;
    // if fifo update for hit
    if (!c->Fifo)
      s.blocks.at(index_of_slot).timeStamp = counter++;
    loadHits++;
  } else {
    uint32_t index_of_min = 0;
    uint32_t min_val = 0xffffffff;
    // loop through, if there is an invalid block found update that, and if not
    // replace the block with the lowest ts
    for (uint32_t i = 0; i < s.blocks.size(); i++) {
      if (s.blocks.at(i).valid) {
        if (min_val > s.blocks.at(i).timeStamp) {
          // update the min val if needed
          min_val = s.blocks.at(i).timeStamp;
          index_of_min = i;
        }
      } else {
        // if an invalid block found replace it
        replace_block(i, tag, &s.blocks.at(i), &s, true, c->wrThrough,
                      block_size);
        return;
      }
    }
    // no invalid block found, updatet the block with the lowest timestamp
    uint32_t tmp = s.blocks.at(index_of_min).tag;
    replace_block(index_of_min, tag, &s.blocks.at(index_of_min), &s, true,
                  c->wrThrough, block_size);
    // remove the old mapping since it got replaced
    s.index.erase(tmp);
    return;
  }
}

void replace_block(uint32_t index, uint32_t tag, Block *b, Set *s, bool is_load,
                   bool w_through, uint32_t size) {
  // if old block is dirty first we need to update the memory
  if (b->is_dirty) {
    b->is_dirty = false;
    total_cycles += size / 4 * 100;
  }
  if (!is_load) {
    // set if the block is dirty or not depending on whether cache is write
    // through or not
    b->is_dirty = !w_through;
    save_misses++;
    total_cycles += ((size) / 4 * 100) + 1;
  }
  if (is_load) {
    loadMisses++;
    total_cycles += size / 4 * 100;
  }
  // update the new block to hold the values from the new block
  b->timeStamp = counter++;
  // update the map so new tag points to this index
  s->index.insert(std::pair<uint32_t, uint32_t>(tag, index));
  b->tag = tag;
  b->valid = true;
}

void operateS(Cache *c, uint32_t setIndex, uint32_t tag, uint32_t block_size) {
  Set &s = c->sets.at(setIndex);
  auto i = s.index.find(tag);
  if (i != s.index.end()) {
    uint32_t index_of_slot = i->second;
    // if fifo update for a hit
    if (!c->Fifo)
      s.blocks.at(index_of_slot).timeStamp = counter++;
    // if write through update the cycle count or tag as dirty d
    if (c->wrThrough) {
      total_cycles += 101;
    } else {
      s.blocks.at(index_of_slot).is_dirty = true;
    }
    save_hits++;
  } else if (c->wrAlloc) {
    uint32_t index_of_min = 0;
    uint32_t min_val = 0xffffffff;
    // loop through, if there is an invalid block found update that, and if not
    // replace the block with the lowest ts
    for (uint32_t i = 0; i < s.blocks.size(); i++) {
      if (s.blocks.at(i).valid) {
        if (min_val > s.blocks.at(i).timeStamp) {
          min_val = s.blocks.at(i).timeStamp;
          index_of_min = i;
        }
      } else {
        // invalid block found, update the invalid block
        replace_block(i, tag, &s.blocks.at(i), &s, false, c->wrThrough,
                      block_size);
        return;
      }
    }
    // no invalid block, update the block w min time stamp
    uint32_t tmp = s.blocks.at(index_of_min).tag;
    s.index.erase(tmp);
    replace_block(index_of_min, tag, &s.blocks.at(index_of_min), &s, false,
                  c->wrThrough, block_size);
    return;
  } else {
    // since its not write allocate we only update the cycle count
    save_misses++;
    total_cycles += 101;
    counter++;
  }
}

void totalL() {
  std::cout << "Total loads: " << loadHits + loadMisses << "\n";
  std::cout << "Total stores: " << save_hits + save_misses << "\n";
  std::cout << "Load hits: " << loadHits << "\n";
  std::cout << "Load misses: " << loadMisses << "\n";
  std::cout << "Store hits: " << save_hits << "\n";
  std::cout << "Store misses: " << save_misses << "\n";
  std::cout << "Total Cycles: " << total_cycles + save_hits + loadHits
            << std::endl;
}