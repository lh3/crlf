#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "crlf.h"

static inline uint64_t crlf_cal_counts(int n_symbols, uint8_t len)
{
	int i;
	uint64_t n_cnt;
	if (len == 255) return 0;
	for (i = 0, n_cnt = 1; i <= len; ++i)
		n_cnt *= n_symbols;
	n_cnt = (n_cnt - 1) / (n_symbols - 1);
	return n_cnt;
}

crlf_t *crlf_create(const char *fn, int n_symbols, uint8_t len, const uint64_t *cnt, const uint32_t dectab[256], crlf_write_f encode, int overwrite)
{
	crlf_t *crlf;
	FILE *fp;
	int to_stdout;
	int64_t n_cnt;

	to_stdout = (fn == 0 || strcmp(fn, "-") == 0);
	if (!overwrite && !to_stdout) {
		fp = fopen(fn, "r");
		if (fp != 0) {
			fclose(fp);
			return 0;
		}
	}
	fp = to_stdout? stdout : fopen(fn, "wb");
	if (fp == 0) return 0;

	if (cnt == 0) len = 255;
	crlf = (crlf_t*)calloc(1, sizeof(crlf_t));
	crlf->n_symbols = n_symbols;
	crlf->encode = encode;
	crlf->is_writing = 1;
	crlf->len = len;
	crlf->fp = fp;
	n_cnt = crlf_cal_counts(n_symbols, len);
	crlf->cnt = (uint64_t*)calloc(n_cnt, 8);
	memcpy(crlf->cnt, cnt, n_cnt * 8);
	memcpy(crlf->dectab, dectab, 256 * 4);

	fwrite("CRL\1", 1, 4, fp);
	fwrite(&n_symbols, 1, 1, fp);
	fwrite(&len, 1, 1, fp);
	fwrite(crlf->cnt, 8, n_cnt, fp);
	fwrite(crlf->dectab, 4, 256, fp);
	return crlf;
}

crlf_t *crlf_open(const char *fn)
{
	FILE *fp;
	char magic[4];
	crlf_t *crlf;
	uint64_t n_cnt;
	uint32_t l;

	fp = (fn && strcmp(fn, "-"))? fopen(fn, "rb") : stdin;
	if (fp == 0) return 0;
	fread(magic, 1, 4, fp);
	if (strncmp(magic, "CRL\1", 4) != 0) {
		fclose(fp);
		return 0;
	}

	crlf = (crlf_t*)calloc(1, sizeof(crlf_t));
	crlf->fp = fp;
	fread(&crlf->n_symbols, 1, 1, fp);
	fread(&crlf->len, 1, 1, fp);
	n_cnt = crlf_cal_counts(crlf->n_symbols, crlf->len);
	crlf->cnt = (uint64_t*)calloc(n_cnt, 8);
	fread(crlf->cnt, 8, n_cnt, fp);
	fread(crlf->dectab, 4, 256, fp);
	crlf->buf_len = fread(crlf->buf, 1, CRLF_BUF_LEN, fp);
	crlf->c = crlf_read_byte(crlf, &l);
	crlf->l = l;
	return crlf;
}

int crlf_close(crlf_t *crlf)
{
	if (crlf == 0) return -1;
	if (crlf->is_writing) {
		crlf->encode(crlf, crlf->c, crlf->l);
		fwrite(crlf->buf, 1, crlf->i, crlf->fp);
	}
	fclose(crlf->fp);
	free(crlf->cnt);
	free(crlf);
	return 0;
}

/**************
 *** Codecs ***
 **************/

int crlf_dectab_RL53(uint32_t dectab[256])
{
	uint32_t x;
	for (x = 0; x < 256; ++x)
		dectab[x] = x>>3<<8 | (x&7);
	return 8;
}

int crlf_write_RL53(crlf_t *crlf, int c, uint64_t l)
{
	while (l > 31) {
		crlf_write_byte(crlf, 31<<3 | c);
		l -= 31;
	}
	crlf_write_byte(crlf, l<<3 | c);
	return 0;
}
