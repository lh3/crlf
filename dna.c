#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <zlib.h>
#include "crlf.h"

static const unsigned char seq_nt6_table[128] = {
    0, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  0, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 1, 5, 2,  5, 5, 5, 3,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  4, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 1, 5, 2,  5, 5, 5, 3,  5, 5, 5, 5,  5, 5, 5, 5,
    5, 5, 5, 5,  4, 5, 5, 5,  5, 5, 5, 5,  5, 5, 5, 5
};

static const char *seq_nt2char = "$ACGTN";

int main_dna2crlf(int argc, char *argv[])
{
	uint8_t *buf;
	uint32_t dectab[256];
	int c, from_stdin, type = 1, buf_len;
	gzFile fp;
	crlf_write_f encode = 0;
	crlf_t *crlf;

	while ((c = getopt(argc, argv, "c:")) >= 0) {
		if (c == 'c') {
			if (strcmp(optarg, "rl53") == 0) type = 1;
			else {
				fprintf(stderr, "[E::%s] unknown codec '%s'\n", __func__, optarg);
				return 1;
			}
		}
	}
	if (type == 1) {
		crlf_dectab_RL53(dectab);
		encode = crlf_write_RL53;
	} else abort();
	from_stdin = !isatty(fileno(stdin));
	if (!from_stdin && optind == argc) {
		fprintf(stderr, "Usage: crlf dna2crlf [-c codec] [in.txt]\n");
		return 1;
	}

	crlf = crlf_create(0, 6, 255, 0, dectab, encode, 1);

	fp = !from_stdin && strcmp(argv[optind], "-")? gzopen(argv[optind], "rb") : gzdopen(fileno(stdin), "rb");
	buf = (uint8_t*)malloc(CRLF_BUF_LEN);
	while ((buf_len = gzread(fp, buf, CRLF_BUF_LEN)) > 0) {
		int i;
		fprintf(stderr, "%d\n", buf_len);
		for (i = 0; i < buf_len; ++i) {
			if (isspace(buf[i])) continue;
			c = buf[i] > 127? 5 : seq_nt6_table[buf[i]];
			crlf_write(crlf, c, 1);
		}
	}
	fprintf(stderr, "%d\n", crlf->i);
	free(buf);
	gzclose(fp);

	crlf_close(crlf);
	return 0;
}
