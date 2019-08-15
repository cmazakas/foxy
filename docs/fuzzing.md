# Fuzzing

The `foxy::parse_uri` function has been fuzzed using [libFuzzer](http://llvm.org/docs/LibFuzzer.html)
on both OS X and Linux using clang-6 on OS X and clang-8 on an Ubuntu VM. On both operating systems
the fuzzer was let run for approximately 10 - 20 minutes.

Results:

OS X:
```
Christians-MBP:build_Debug exbigboss$ ./uri-fuzzer
INFO: Seed: 1722869126
INFO: Loaded 1 modules   (10 inline 8-bit counters): 10 [0x10a077ffe, 0x10a078008),
INFO: Loaded 1 PC tables (10 PCs): 10 [0x10a078008,0x10a0780a8),
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 6 ft: 6 corp: 1/1b exec/s: 0 rss: 46Mb
#14     NEW    cov: 6 ft: 11 corp: 2/2b exec/s: 0 rss: 46Mb L: 1/1 MS: 2 ShuffleBytes-ChangeByte-
#65536  pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 32768 rss: 251Mb
#131072 pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 32768 rss: 426Mb
#262144 pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 29127 rss: 430Mb
#524288 pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 29127 rss: 431Mb
#1048576        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 29127 rss: 433Mb
#2097152        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28728 rss: 433Mb
#4194304        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28926 rss: 434Mb
#8388608        pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28728 rss: 435Mb
#16777216       pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28532 rss: 436Mb
#33554432       pulse  cov: 6 ft: 11 corp: 2/2b exec/s: 28460 rss: 437Mb
^C==1904== libFuzzer: run interrupted; exiting
```

---

To [ToC](./index.md#Table-of-Contents)
