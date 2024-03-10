#include "cache.h"
#include "helper_functions.h"
#include <iostream>

using namespace std;


int main(int argc, char * argv[]) {
    //Handel Cache configuration
    bool wrAlloc = false;
    bool wrThrough = false;
    bool Fifo = false;
    int isInvalid = checkArgValidity(argc, argv, &wrAlloc, &wrThrough, &Fifo);
    if (isInvalid != 0) {
        return isInvalid;
    }

    //store args as ints
    uint32_t numSets = stoul(argv[1]);
    uint32_t numBlocks = stoul(argv[2]);
    uint32_t blockSize = stoul(argv[3]);
    int logSets = logBase2(nSets);
    int logBlockSize = logBase2(blockSize); 

    return 0; 
}
