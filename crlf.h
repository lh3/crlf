#ifndef CRLF_H
#define CRLF_H

#include <stdio.h>
#include <stdint.h>

#define CRLF_ENC_NULL  0
#define CRLF_ENC_RL53  1
#define CRLF_ENC_RL35  2
#define CRLF_ENC_ASCII 100

#define CRLF_BUF_LEN  0x10000

typedef struct {
	uint8_t len, is_writing, n_symbols;
	uint16_t enc;
	uint64_t *cnt;
	FILE *fp;
	uint32_t dectab[256];

	int c, i, buf_len;
	uint64_t l;
	uint8_t buf[CRLF_BUF_LEN];
} crlf_t;

#ifdef __cplusplus
extern "C" {
#endif

	crlf_t *crlf_create(const char *fn, uint8_t n_symbols, uint16_t enc, uint8_t len, const uint64_t *cnt, int overwrite);
	crlf_t *crlf_open(const char *fn);
	int crlf_close(crlf_t *crlf);

#ifdef __cplusplus
}
#endif

static inline uint64_t crlf_cal_counts(int n_symbols, int len)
{
	int i;
	uint64_t n_cnt;
	for (i = 0, n_cnt = 1; i <= len; ++i)
		n_cnt *= n_symbols;
	n_cnt = (n_cnt - 1) / (n_symbols - 1);
	return n_cnt;
}

static inline void crlf_write_byte(crlf_t *crlf, uint8_t byte)
{
	if (crlf->i == CRLF_BUF_LEN) {
		fwrite(crlf->buf, 1, crlf->i, crlf->fp);
		crlf->i = 0;
	}
	crlf->buf[crlf->i++] = byte;
}

static inline int crlf_write_RL53(crlf_t *crlf, int c, uint64_t l)
{
	if (l == 0 || c >= crlf->n_symbols) return -1;
	if (crlf->l > 0) { // a staging run
		if (c != crlf->c) { // a new run
			while (crlf->l > 31) {
				crlf_write_byte(crlf, 31<<3 | crlf->c);
				crlf->l -= 31;
			}
			crlf_write_byte(crlf, crlf->l<<3 | crlf->c);
			crlf->c = c, crlf->l = l;
		} else crlf->l += l; // extend the staging run
	} else crlf->c = c, crlf->l = l;
	return 0;
}

static inline int crlf_write(crlf_t *crlf, int c, uint64_t l)
{
	if (crlf->enc == CRLF_ENC_RL53) return crlf_write_RL53(crlf, c, l);
	else return -1;
}

static inline int crlf_read_byte(crlf_t *crlf, uint32_t *l)
{
	uint32_t x;
	if (crlf->buf_len == 0) return -1;
	if (crlf->i == crlf->buf_len) { // then fill the buffer
		crlf->buf_len = fread(crlf->buf, 1, CRLF_BUF_LEN, crlf->fp);
		if (crlf->buf_len == 0) return -1;
	}
	x = crlf->dectab[crlf->i++];
	*l = x>>8;
	return x&7;
}

static inline int crlf_read(crlf_t *crlf, int64_t *l)
{
	int c, ret_c;
	uint32_t l1;
	if (crlf->buf_len == 0) return -1;
	while ((c = crlf_read_byte(crlf, &l1)) == crlf->c)
		crlf->l += l1;
	*l = crlf->l, ret_c = crlf->c;
	crlf->l = l1, crlf->c = c;
	return ret_c;
}

#endif
