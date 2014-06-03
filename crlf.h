#ifndef CRLF_H
#define CRLF_H

#include <stdio.h>
#include <stdint.h>

#define CRLF_BUF_LEN 0x10000

struct crlf_s;
typedef int (*crlf_write_f)(struct crlf_s*, int c, uint64_t);

typedef struct {
	char tag[2];
	uint64_t len;
	uint8_t *data;
} crlf_tag_t;

typedef struct crlf_s {
	uint8_t is_writing;   // if the file is open for writing
	uint8_t n_symbols;    // number of symbols, including the sentinel
	uint32_t dectab[256]; // decoding table
	crlf_write_f encode;  // encoding function; only used on writing
	FILE *fp;             // file pointer

	uint32_t n_tags;      // number of tags
	crlf_tag_t *tags;     // only filled on reading!

	// The following are related to bufferring
	int c, i, buf_len;
	uint64_t l;
	uint8_t buf[CRLF_BUF_LEN];
} crlf_t;

#ifdef __cplusplus
extern "C" {
#endif

	crlf_t *crlf_create(const char *fn, int n_symbols, const uint32_t dectab[256], crlf_write_f encode, uint32_t n_tags, const crlf_tag_t *tags);
	crlf_t *crlf_open(const char *fn);
	int crlf_close(crlf_t *crlf);

	int crlf_dectab_RL53(uint32_t dectab[256]);
	int crlf_write_RL53(crlf_t *crlf, int c, uint64_t l);

#ifdef __cplusplus
}
#endif

static inline void crlf_write_byte(crlf_t *crlf, uint8_t byte)
{
	if (crlf->i == CRLF_BUF_LEN) {
		fwrite(crlf->buf, 1, crlf->i, crlf->fp);
		crlf->i = 0;
	}
	crlf->buf[crlf->i++] = byte;
}

static inline int crlf_write(crlf_t *crlf, int c, uint64_t l)
{
	int ret = 0;
	if (c >= crlf->n_symbols || l == 0) return -1;
	if (crlf->l > 0) { // a staging run
		if (c != crlf->c) { // a new run
			ret = crlf->encode(crlf, crlf->c, crlf->l);
			crlf->c = c, crlf->l = l;
		} else crlf->l += l; // extend the staging run
	} else crlf->c = c, crlf->l = l;
	return ret;
}

static inline int crlf_read_byte(crlf_t *crlf, uint32_t *l)
{
	uint32_t x;
	if (crlf->buf_len == 0) return -1;
	if (crlf->i == crlf->buf_len) { // then fill the buffer
		crlf->buf_len = fread(crlf->buf, 1, CRLF_BUF_LEN, crlf->fp);
		if (crlf->buf_len == 0) return -1;
	}
	x = crlf->dectab[crlf->buf[crlf->i++]];
	*l = x>>8;
	return x&7;
}

static inline int crlf_read(crlf_t *crlf, uint64_t *l)
{
	int c, ret_c;
	uint32_t l1;
	if (crlf->buf_len == 0) {
		if (crlf->l > 0) {
			*l = crlf->l;
			return crlf->c;
		} else return -1;
	}
	while ((c = crlf_read_byte(crlf, &l1)) == crlf->c)
		crlf->l += l1;
	*l = crlf->l, ret_c = crlf->c;
	crlf->l = l1, crlf->c = c;
	return ret_c;
}

#endif
