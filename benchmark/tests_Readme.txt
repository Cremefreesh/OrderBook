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