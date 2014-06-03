## Introduction

CRLF, or Concise Run-Length Format, is a lightweight format to store run-length
encoded strings, typically for small alphabets such as DNA. This repository
contains the spec, a library and a tool to convert encodings.

The library is implemented in two files `crlf.h` and `crlf.c` without any
dependencies to other libraries. Users who are not implementing encoders
need to focus on five functions:

* `crlf_create()` creates a CRLF and writes the header, with user-provided
  decoding table and the function pointer for encoding a run.

* `crlf_open()` opens an existing CRLF for reading.

* `crlf_close()` closes a CRLF.

* `crlf_write()` writes a run to CRLF. It may write multiple bytes given a long
  run. If this function is called consecutively on runs of the same symbol, these
  runs will be merged.

* `crlf_read()` reads a run from CRLF until it meets the next run of a different
  symbol from the current run.
