Version 1.1 Benchmark

Resting insert
p50     42 ns
p95     84 ns
p99    125 ns
p99.9 2208 ns
max 276250 ns

Exact match
p50     42 ns
p95     84 ns
p99     84 ns
p99.9  209 ns
max   8500 ns


likely issue: memory management or container growth or mix 

fix: reserve memory upfront xs




v1.2 --> added reserve to our unordered map


Resting Limit Insert
------------------------------
Operations: 100000
Throughput: 1.02723e+07 operations/sec
Average latency: 73.5406 ns
p50 latency: 42 ns
p95 latency: 84 ns
p99 latency: 125 ns
p99.9 latency: 2375 ns
Maximum latency: 90916 ns

Exact Match
------------------------------
Operations: 100000
Throughput: 9.0285e+06 operations/sec
Average latency: 59.1062 ns
p50 latency: 42 ns
p95 latency: 84 ns
p99 latency: 84 ns
p99.9 latency: 416 ns
Maximum latency: 12167 ns


issues: long tail was not just rehashing, max did drop but other issues present. 
Next culprit is per order heap allocation from our std::list 

when we are pushing back into our list, we are asking for more memory from the alocator 
which needs contruction and then to be linked
--> overhead i need to eliminate 



1.3 

Resting Limit Insert
------------------------------
Operations: 100000
Throughput: 2.19264e+07 operations/sec
Average latency: 30.6282 ns
p50 latency: 41 ns
p95 latency: 42 ns
p99 latency: 42 ns
p99.9 latency: 1042 ns
Maximum latency: 11708 ns

Memory activity
Allocations: 200001
Deallocations: 0
Allocated bytes: 8800072
Allocations / operation: 2.00001

Exact Match
------------------------------
Operations: 100000
Throughput: 1.6995e+07 operations/sec
Average latency: 31.0637 ns
p50 latency: 41 ns
p95 latency: 42 ns
p99 latency: 42 ns
p99.9 latency: 125 ns
Maximum latency: 2541 ns

Memory activity
Allocations: 100000
Deallocations: 200001
Allocated bytes: 3200000
Allocations / operation: 1




breakdown:
Resting insert:
200,001 allocations / 100,000 operations
≈ 2 allocations per order

Exact match:
100,000 allocations / 100,000 operations
= 1 allocation per order

Exact match:
200,001 deallocations
≈ 2 deallocations per order

BUY arrives
    │
    ├─ std::list creates Order node
    │       └── ALLOCATION #1
    │
    └─ unordered_map creates OrderLocation node
            └── ALLOCATION #2

100,000 list allocations
+
100,000 unordered_map node allocations
+
1 container structural allocation
=
200,001