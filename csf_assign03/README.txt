We designed each experiment to isolate the effects of different cache parameters, allowing us to conclude their individual and combined impact on performance. We will outline each parameter we tested, how and why we tested it, as well as our conclusions. 

Exploring Associativity
First, we wanted to test how increasing associativity from direct-mapped to fully associative impacts conflict misses and overall cache efficiency. We kept the other parameters constant.

Direct-Mapped:
./csim 256 1 64 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 312,297
Load misses: 5,900
Store hits: 194,064
Store misses: 3,422
Total Cycles: 34,519,086

2-way Set-Associative:
./csim 128 2 64 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 315,464
Load misses: 2,733
Store hits: 194,661
Store misses: 2,825
Total Cycles: 28,556,386

4-way Set-Associative:
./csim 64 4 64 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 316,157
Load misses: 2,040
Store hits: 194,821
Store misses: 2,665
Total Cycles: 27,207,586

8-way Set-Associative:
./csim 32 8 64 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 316,316
Load misses: 1,881
Store hits: 194,862
Store misses: 2,624
Total Cycles: 26,891,686

Fully Associative:
./csim 1 256 64 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 316,383
Load misses: 1,814
Store hits: 194,867
Store misses: 2,619
Total Cycles: 26,776,986

Conclusion: Higher associativity consistently improves load and store hit rates, decreasing total cycles. The fully associative cache, while showing the best performance, may not be practical due to its complexity and cost. A 4-way set-associative cache may offer a more balanced compromise.

Impact of Write Policies
Next, we tried to assess how different write policies affect cache performance, specifically comparing write-back versus write-through and the effect of write-allocate. Here we used a 4-way set-associative cache because we think from the last experiment that it may be the optimal choice for associativity. 

Write-Through (4-way set-associative):
./csim 64 4 64 write-allocate write-through lru < gcc.trace
Total Cycles: 27,207,586

Write-Back (4-way set-associative):
./csim 64 4 64 write-allocate write-back lru < gcc.trace
Total Cycles: 12,537,065

No-Write-Allocate (4-way set-associative):
./csim 64 4 64 no-write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 315,059
Load misses: 3,138
Store hits: 172,862
Store misses: 24,624
Total Cycles: 24,966,886

Conclusions: The write-back policy dramatically reduces total cycles, showcasing its efficiency in reducing memory write operations and enhancing overall cache performance. The no-write-allocate option significantly increases store misses, underscoring the effectiveness of write-allocate in leveraging cache for writes.

Block Size Variation
Next, we wanted to test the effects of changing block sizes on cache performance, considering the balance between spatial locality and miss penalty. Here we standardized to a 2-way set-associative cache. 

32 bytes block size (2-way set-associative):
./csim 128 2 32 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 313,219
Load misses: 4,978
Store hits: 192,181
Store misses: 5,305
Total Cycles: 27,641,986

64 bytes block size (2-way set-associative):
./csim 128 2 64 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 315,464
Load misses: 2,733
Store hits: 194,661
Store misses: 2,825
Total Cycles: 28,556,386

128 bytes block size (2-way set-associative):
./csim 128 2 128 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 316,531
Load misses: 1,666
Store hits: 195,991
Store misses: 1,495
Total Cycles: 29,911,786

Conclusions: While larger block sizes tend to improve hit rates by capturing more spatial locality, they also increase the penalty for misses, leading to higher total cycles in some cases. The choice of block size is a trade-off, and the 64-byte block size in a 2-way set-associative cache configuration seems to give a balanced option, optimizing between hit rate improvement and penalty minimization.

Eviction Policy Impact
Next, we wanted to determine how LRU and FIFO eviction policies perform in managing cache evictions and their overall impact on cache efficiency. We standardized to a 4-way set-associative cache for this, as this is still our optimal choice. 

LRU Eviction Policy (4-way set-associative):
./csim 64 4 64 write-allocate write-through lru < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 316,157
Load misses: 2,040
Store hits: 194,821
Store misses: 2,665
Total Cycles: 27,207,586

FIFO Eviction Policy (4-way set-associative):
./csim 64 4 64 write-allocate write-through fifo < gcc.trace
Total loads: 318,197
Total stores: 197,486
Load hits: 315,570
Load misses: 2,627
Store hits: 194,716
Store misses: 2,770
Total Cycles: 28,304,286

Conclusions: The LRU eviction policy slightly outperforms FIFO in this context, suggesting its better capability in managing cache blocks by more effectively utilizing the principle of temporal locality. However, the difference in total cycles between LRU and FIFO indicates that while the choice of eviction policy is important, its impact may be less pronounced compared to factors like associativity and write policies.

Final Conclusions
The comprehensive analysis clearly indicates that a 4-way set-associative cache, configured with 64-byte block size, employing a write-back policy and LRU for eviction, stands out as the optimal choice. This configuration offers an effective balance between high performance (in terms of hit rates and reduced total cycles) and practical considerations such as complexity and resource utilization. It excels in minimizing conflict misses through an optimal level of associativity, efficiently managing write operations with the write-back policy, and effectively handling block evictions through LRU, thereby providing a high-performance yet cost-effective caching solution for this specific workload.







Contributions: We Pair coded most of the assignment. Stuart worked a little more on the Cache structure, and Aviyshe did more work on the simulator. 