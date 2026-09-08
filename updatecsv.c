#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Code to read:
 * join.c
 * awk, lib.c, function readcsvrec
 */

// TODO: remove \r in case it appears. Look at lib.c from awk.

int 	readcsv(FILE *fp);
void 	usage(void);

char delim;
long c1, c2; 	/* column with ids from file 1 and 2 */

int
main(int argc, char *argv[])
{
	char *end;

	delim = ',';
	c1 = c2 = 1;

	int ch;
	while ((ch = getopt(argc, argv, "1:2:d:h")) != -1) {
		switch (ch) {
		case '1':
			if ((c1 = strtol(optarg, &end, 10)) < 1) {
				fprintf(stderr, "error: -1 option is less than 1\n");
				exit(EXIT_FAILURE);
			}
			if (*end) {
				fprintf(stderr, "error: illegal field number -- %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case '2':
			if ((c2 = strtol(optarg, &end, 10)) < 1) {
				fprintf(stderr, "error: -1 option is less than 1\n");
				exit(EXIT_FAILURE);
			}
			if (*end) {
				fprintf(stderr, "error: illegal field number -- %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'd':
			delim = optarg[0];
			break;
		case 'h':
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		fprintf(stderr, "missing args\n");
		usage();
		exit(EXIT_FAILURE);
	}

	FILE *fp;
	if ((fp = fopen(*argv, "r")) == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	readcsv(fp);
	fclose(fp);

	readcsv(stdin);

	exit(EXIT_SUCCESS);
}

int
readcsv(FILE *fp)
{
	int c;

	// TODO: check if is inside quotes
	while ((c = fgetc(fp)) != EOF) { 
		if (c == delim)
			putchar('\t');
		else
			putchar(c);
	}
	return 0;
}

void
usage(void)
{
	fprintf(stderr, "usage: updatecsv [-d delim] [-1 COL1] [-2 COL2] FILE1 < FILE2\n");
	fprintf(stderr, "\nUpdate FILE1 with data from FILE2\n");
	fprintf(stderr, " -d  change delimiter. comma by default\n");
	fprintf(stderr, " -1  number of column with ID in FILE1. Default is 1\n");
	fprintf(stderr, " -2  number of column with ID in FILE2. Default is 1\n");
}
