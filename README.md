n-Length Format, is a lightweight format to store run-length
encoded strings, typically for small alphabets such as DNA. This repository
contains the spec, a library and a tool to convert encodings.

The library is implemented in two files `crlf.h` and `crlf.c` without any
dependencies to other libraries. Users who are not implementing encoders
need to focus on five functions (see `crlf.h` for details):

* `crlf_create()` creates a CRLF and writes the header, with a user-provided
  decoding table and a function pointer for encoding a run.

* `crlf_open()` opens an existing CRLF for reading.

* `crlf_close()` closes a CRLF.

* `crlf_write()` writes a run to CRLF. If this function is called consecutively
  on runs of the same symbol, these   runs will be merged.

* `crlf_read()` reads a run from CRLF until it meets the next run of a different
  symbol from the current run.

To create a CRLF, a user-defined encoding function must be provided which looks
like (see `crlf.c`):
```C
int crlf_write_RL53(crlf_t *crlf, int c, uint64_t l)
{
	while (l > 31) {
		crlf_write_byte(crlf, 31<<3 | c);
		l -= 31;
	}
	crlf_write_byte(crlf, l<<3 | c);
	return 0;
}
```
It turns a long run into multiple short runs, represented by bytes, and then
writes them to CRLF. It is also recommended, though not required, to provide
a function to generate the decoding table:
```C
int crlf_dectab_RL53(uint32_t dectab[256])
{
	uint32_t x;
	for (x = 0; x < 256; ++x)
		dectab[x] = x>>3<<8 | (x&7);
	return 8;
}

```
