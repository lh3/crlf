#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "crlf.h"

static void gen_dectab_RL53(uint32_t dectab[256])
{
	uint32_t x;
	for (x = 0; x < 256; ++x)
		dectab[x] = x>>3<<8 | (x&7);
}

static void gen_dectab_RL35(uint32_t dectab[256])
{
	uint32_t x;
	for (x = 0; x < 256; ++x)
		dectab[x] = (x&31)<<8 | (x>>5);
}

crlf_t *crlf_create(const char *fn, uint8_t n_symbols, uint16_t enc, uint8_t len, const uint64_t *cnt, int overwrite)
{
	crlf_t *crlf;
	FILE *fp;
	int to_stdout;
	int64_t n_cnt;
	uint32_t dectab[256];

	if (enc == CRLF_ENC_RL53) gen_dectab_RL53(dectab); // TODO: check if n_symbols is too large
	else if (enc == CRLF_ENC_RL35) gen_dectab_RL35(dectab);
	else return 0;
	
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

	n_cnt = crlf_cal_counts(n_symbols, len);

	crlf = (crlf_t*)calloc(1, sizeof(crlf_t));
	crlf->is_writing = 1;
	crlf->len = len;
	crlf->enc = enc;
	crlf->fp = fp;
	crlf->cnt = (uint64_t*)calloc(n_cnt, 8);
	memcpy(crlf->cnt, cnt, n_cnt * 8);
	memcpy(crlf->dectab, dectab, 256 * 4);

	fwrite("CRL\1", 1, 4, fp);
	fwrite(&n_symbols, 1, 1, fp);
	fwrite(&enc, 2, 1, fp);
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
	fread(&crlf->enc, 2, 1, fp);
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
	if (crlf->is_writing && crlf->i > 0)
		fwrite(crlf->buf, 1, crlf->i, crlf->fp);
	fclose(crlf->fp);
	free(crlf->cnt);
	free(crlf);
	return 0;
}
