# Fuzzing

The `foxy::parse_uri` function has been fuzzed using [libFuzzer](http://llvm.org/docs/LibFuzzer.html)
on both OS X and Linux using clang-6 on OS X and clang-8 on an Ubuntu VM. On both operating systems
the fuzzer was let run for approximately 20 minutes with `ubsan` and `asan` enabled.

The corresponding toolchain files `tools/foxy-linux-clang.cmake` and `tools/foxy-osx.cmake` contain
the compiler flags that show how the fuzzing binaries were built.

Results:

OS X:

```
Christians-MacBook-Pro:build_Debug exbigboss$ ./uri-parser -max_total_time=1200
INFO: Seed: 4259057045
INFO: Loaded 1 modules   (9 inline 8-bit counters): 9 [0x10b002d50, 0x10b002d59),
INFO: Loaded 1 PC tables (9 PCs): 9 [0x10b002d60,0x10b002df0),
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 6 ft: 6 corp: 1/1b exec/s: 0 rss: 46Mb
#13     NEW    cov: 6 ft: 11 corp: 2/2b exec/s: 0 rss: 46Mb L: 1/1 MS: 1 ChangeByte-
#65536  pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 32768 rss: 250Mb
#131072 pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 32768 rss: 426Mb
#262144 pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 29127 rss: 429Mb
#524288 pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 29127 rss: 431Mb
#1048576        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 29127 rss: 433Mb
#2097152        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 29127 rss: 434Mb
#4194304        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28532 rss: 434Mb
#8388608        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28244 rss: 435Mb
#16777216       pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28102 rss: 436Mb
#33554432       pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28173 rss: 438Mb
#33803883       DONE   cov: 6 ft: 11 corp: 2/2b exec/s: 28146 rss: 438Mb
Done 33803883 runs in 1201 second(s)
```

Linux:

```
chris@ubuntu-template:~/f3/build_clang_debug$ ./uri-parser -max_total_time=1200
INFO: Seed: 3626518709
INFO: Loaded 1 modules   (4 inline 8-bit counters): 4 [0xda6f30, 0xda6f34),
INFO: Loaded 1 PC tables (4 PCs): 4 [0xaba818,0xaba858),
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 3 ft: 4 corp: 1/1b lim: 4 exec/s: 0 rss: 32Mb
#13     NEW    cov: 3 ft: 6 corp: 2/2b lim: 4 exec/s: 0 rss: 33Mb L: 1/1 MS: 1 ChangeByte-
#17     NEW    cov: 3 ft: 7 corp: 3/3b lim: 4 exec/s: 0 rss: 33Mb L: 1/1 MS: 4 ChangeBinInt-ShuffleBytes-ShuffleBytes-ChangeBit-
#65536  pulse  cov: 3 ft: 7 corp: 3/3b lim: 652 exec/s: 21845 rss: 172Mb
#131072 pulse  cov: 3 ft: 7 corp: 3/3b lim: 1300 exec/s: 26214 rss: 322Mb
#262144 pulse  cov: 3 ft: 7 corp: 3/3b lim: 2600 exec/s: 21845 rss: 488Mb
#341626 NEW    cov: 3 ft: 8 corp: 4/8b lim: 3392 exec/s: 22775 rss: 526Mb L: 5/5 MS: 4 InsertByte-CopyPart-ChangeBit-InsertByte-
#341678 REDUCE cov: 3 ft: 8 corp: 4/6b lim: 3392 exec/s: 22778 rss: 526Mb L: 3/3 MS: 2 ChangeBinInt-EraseBytes-
#524288 pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 21845 rss: 549Mb
#1048576        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 23831 rss: 549Mb
#2097152        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 25266 rss: 549Mb
#4194304        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 26051 rss: 549Mb
#8388608        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 26132 rss: 549Mb
#16777216       pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 25811 rss: 549Mb
#30775638       DONE   cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 25625 rss: 549Mb
Done 30775638 runs in 1201 second(s)
```

---

To [ToC](./index.md#Table-of-Contents)
