# Performance Benchmarks

## 2026-01-21

**Build:** Debug
**Command:** `./build/sim/irata2_bench --cycles 5000000 --warmup 100000`

| Workload | Cycles | Elapsed (s) | Cycles/sec | Halt reason |
| --- | --- | --- | --- | --- |
| loop | 5,000,000 | 49.0292 | 101,980 | timeout |
| mem | 5,000,000 | 53.8152 | 92,910.5 | timeout |

Notes:
- Results captured in the local dev sandbox; expect different numbers on release builds.
