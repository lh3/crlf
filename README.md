CRLF, or Concise Run-Length Format, is a lightweight format to store run-length
encoded strings, typically for small alphabets such as DNA. This repository
contains the spec, a library and a tool to convert encodings. The library is
implemented in two files `crlf.h` and `crlf.c` without any dependencies to other
libraries.

The minimal code create a CRLF to stdout from a BWT string:
```C
uint32_t dectab[256], l_BWT, i;
uint8_t *BWT; // $ACGTN encoded as 012345
crlf_t *crlf;
crlf_dectab_RL53(dectab); // generate the decoding table
crlf = crlf_create(0, 6, dectab, crlf_write_RL53, 0, 0);
for (i = 0; i < l_BWT; ++i)
    crlf_write(crlf, BWT[i], 1);
crlf_close(crlf);
```
The minimal code to read a CRLF from stdin:
```C
int c;
uint64_t i, l;
crlf_t *crlf;
crlf = crlf_open(0);
while ((c = crlf_read(crlf, &l)) >= 0)
    for (i = 0; i < l; ++i)
        putchar("$ACGTN"[c]);
crlf_close(crlf);
```

Basic APIs (see `crlf.h` for details):

* `crlf_create()` creates a CRLF and writes the header, with a user-provided
  decoding table and a function pointer for encoding a run.

* `crlf_open()` opens an existing CRLF for reading.

* `crlf_close()` closes a CRLF.

* `crlf_write()` writes a run to CRLF. If this function is called consecutively
  on runs of the same symbol, these   runs will be merged.

* `crlf_read()` reads a run from CRLF until it meets the next run of a different
  symbol from the current run.
