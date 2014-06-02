#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <zlib.h>
#include "crlf.h"

#define IN_CRLF 0
#define IN_DNA4 1
#define IN_DNA5 2

#define OUT_RL53 1
#define OUT_DNA5 2

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

int main(int argc, char *argv[])
{
	uint32_t dectab[256];
	int c, from_stdin, in_fmt = IN_CRLF, out_codec = OUT_RL53;
	const char *out_fn = 0;
	crlf_write_f encode = 0;
	crlf_t *in = 0, *out = 0;

	while ((c = getopt(argc, argv, "e:d:o:")) >= 0) {
		if (c == 'd') {
			if (strcmp(optarg, "crlf") == 0) in_fmt = IN_CRLF;
			else if (strcmp(optarg, "dna4") == 0) in_fmt = IN_DNA4;
			else if (strcmp(optarg, "dna5") == 0) in_fmt = IN_DNA5;
			else {
				fprintf(stderr, "[E::%s] unknown input format '%s'\n", __func__, optarg);
				return 1;
			}
		} else if (c == 'e') {
			if (strcmp(optarg, "rl53") == 0) out_codec = OUT_RL53;
			else if (strcmp(optarg, "dna5") == 0) out_codec = OUT_DNA5;
			else {
				fprintf(stderr, "[E::%s] unknown codec '%s'\n", __func__, optarg);
				return 1;
			}
		} else if (c == 'o') out_fn = optarg;
	}

	from_stdin = !isatty(fileno(stdin));
	if (!from_stdin && optind == argc) {
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage:   crlf [options] <file.in>\n\n");
		fprintf(stderr, "Options: -d STR    input format: dna4, dna5 or crlf [crlf]\n");
		fprintf(stderr, "         -e STR    output codec: dna5 or rl53 [rl53]\n");
		fprintf(stderr, "         -o FILE   output file name [stdout]\n");
		fprintf(stderr, "\n");
		return 1;
	}

	if (in_fmt == IN_CRLF) {
		in = crlf_open(!from_stdin && strcmp(argv[optind], "-")? argv[optind] : 0);
		if (in == 0) {
			fprintf(stderr, "[E::%s] failed to read the input CRLF file\n", __func__);
			return 1;
		}
	}

	if (out_codec != OUT_DNA5) {
		if (out_codec == OUT_RL53) {
			encode = crlf_write_RL53;
			crlf_dectab_RL53(dectab);
		} else abort();
		if (in_fmt != IN_CRLF) {
			uint8_t n_symbols;
			if (in_fmt == IN_DNA4) n_symbols = 5;
			else if (in_fmt == IN_DNA5) n_symbols = 6;
			else abort();
			out = crlf_create(out_fn, n_symbols, 255, 0, dectab, encode, 1);
		} else out = crlf_create(out_fn, in->n_symbols, in->len, in->cnt, dectab, encode, 1);
	} else if (in && in->n_symbols > 6) {
		fprintf(stderr, "[E::%s] #symbols in the input is %d. The input cannot be DNA strings.\n", __func__, in->n_symbols);
		return 1; // FIXME: memory leak
	}

	if (in_fmt == IN_CRLF) {
		uint64_t i, l;
		if (out_codec == OUT_DNA5) {
			while ((c = crlf_read(in, &l)) >= 0)
				for (i = 0; i < l; ++i)
					putchar(seq_nt2char[c]);
			putchar('\n');
		} else {
			while ((c = crlf_read(in, &l)) >= 0)
				crlf_write(out, c, l);
		}
		crlf_close(in);
	} else {
		gzFile fp;
		uint8_t *buf;
		int buf_len;

		fp = !from_stdin && strcmp(argv[optind], "-")? gzopen(argv[optind], "rb") : gzdopen(fileno(stdin), "rb");
		buf = (uint8_t*)malloc(CRLF_BUF_LEN);
		while ((buf_len = gzread(fp, buf, CRLF_BUF_LEN)) > 0) {
			int i;
			if (out_codec == OUT_DNA5) {
				for (i = 0; i < buf_len; ++i) {
					if (isspace(buf[i])) continue;
					c = buf[i] > 127? 5 : seq_nt6_table[buf[i]];
					putchar(seq_nt2char[c]);
				}
				putchar('\n');
			} else {
				for (i = 0; i < buf_len; ++i) {
					if (isspace(buf[i])) continue;
					c = buf[i] > 127? 5 : seq_nt6_table[buf[i]];
					if (crlf_write(out, c, 1) < 0) break;
				}
				if (i < buf_len) {
					fprintf(stderr, "[E::%s] failed to write CRLF\n", __func__);
					return 1; // FIXME: memory leak
				}
			}
		}
		free(buf);
		gzclose(fp);
	}

	crlf_close(out);
	return 0;
}
