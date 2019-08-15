# Fuzzing

The `foxy::parse_uri` function has been fuzzed using [libFuzzer](http://llvm.org/docs/LibFuzzer.html)
on both OS X and Linux using clang-6 on OS X and clang-8 on an Ubuntu VM. On both operating systems
the fuzzer was let run for approximately 10 - 20 minutes with `ubsan` and `asan` enabled.

The corresponding toolchain files `tools/foxy-linux-clang.cmake` and `tools/foxy-osx.cmake` contain
the compiler flags that show how the fuzzing binaries were built.

Results:

OS X:

```
Christians-MBP:build_Debug exbigboss$ ./uri-parser
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

Linux:

```
chris@ubuntu-template:~/f3/build_clang_debug$ ./uri-parser -max_total_time=600
INFO: Seed: 574513396
INFO: Loaded 1 modules   (4 inline 8-bit counters): 4 [0xda6f30, 0xda6f34),
INFO: Loaded 1 PC tables (4 PCs): 4 [0xaba8a8,0xaba8e8),
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2      INITED cov: 3 ft: 4 corp: 1/1b lim: 4 exec/s: 0 rss: 32Mb
#3      NEW    cov: 3 ft: 5 corp: 2/2b lim: 4 exec/s: 0 rss: 33Mb L: 1/1 MS: 1 ChangeByte-
#18     NEW    cov: 3 ft: 7 corp: 3/6b lim: 4 exec/s: 0 rss: 33Mb L: 4/4 MS: 5 InsertByte-CrossOver-CopyPart-CopyPart-CopyPart-
#84     REDUCE cov: 3 ft: 7 corp: 3/5b lim: 4 exec/s: 0 rss: 33Mb L: 3/3 MS: 1 EraseBytes-
#160    REDUCE cov: 3 ft: 7 corp: 3/3b lim: 4 exec/s: 0 rss: 33Mb L: 1/1 MS: 1 CrossOver-
#32768  pulse  cov: 3 ft: 7 corp: 3/3b lim: 325 exec/s: 16384 rss: 101Mb
#65536  pulse  cov: 3 ft: 7 corp: 3/3b lim: 652 exec/s: 16384 rss: 172Mb
#99275  NEW    cov: 3 ft: 8 corp: 4/8b lim: 985 exec/s: 16545 rss: 248Mb L: 5/5 MS: 5 ChangeByte-CrossOver-CopyPart-CrossOver-ChangeBit-
#99301  REDUCE cov: 3 ft: 8 corp: 4/6b lim: 985 exec/s: 16550 rss: 248Mb L: 3/3 MS: 1 EraseBytes-
#131072 pulse  cov: 3 ft: 8 corp: 4/6b lim: 1300 exec/s: 16384 rss: 327Mb
#262144 pulse  cov: 3 ft: 8 corp: 4/6b lim: 2600 exec/s: 16384 rss: 491Mb
#524288 pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 18724 rss: 533Mb
#1048576        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 21845 rss: 533Mb
#2097152        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 23563 rss: 533Mb
#4194304        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 25115 rss: 533Mb
#8388608        pulse  cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 25040 rss: 533Mb
#15053191       DONE   cov: 3 ft: 8 corp: 4/6b lim: 4096 exec/s: 25046 rss: 533Mb
Done 15053191 runs in 601 second(s)
```

---

To [ToC](./index.md#Table-of-Contents)
