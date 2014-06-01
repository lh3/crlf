#include <stdio.h>
#include <string.h>

int main_dna2crlf(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	int ret = 0;
	if (argc == 1) {
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage:   crlf <command> [arguments]\n\n");
		fprintf(stderr, "Command: dna2crlf    convert plain DNA string to CRLF\n");
		fprintf(stderr, "\n");
		return 1;
	}
	if (strcmp(argv[1], "dna2crlf") == 0) ret = main_dna2crlf(argc-1, argv+1);
	else {
		fprintf(stderr, "[E::%s] unknown command\n", __func__);
		return 1;
	}
	return ret;
}
