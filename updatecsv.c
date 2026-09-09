#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Code to read:
 * join.c
 * awk, lib.c, function readcsvrec
 */
// TODO: remove \r in case it appears. Look at lib.c from awk.

struct record {
	char 	**line;
	int 	  ncol;
};

struct table {
	char 		**names;
	struct record 	**records;
	size_t 		  nrecords;
};

int 	readcsv(FILE *, struct table *);
void 	usage(void);
void 	free_table(struct table *);
long	readnum(char *s);


char delim;
long c1, c2; 	/* column with ids from file 1 and 2 */

struct table tbl1 = { NULL, NULL, 0 };
struct table tbl2 = { NULL, NULL, 0 };

int
main(int argc, char *argv[])
{

	delim = ',';
	c1 = c2 = 1;

	int ch;
	while ((ch = getopt(argc, argv, "1:2:d:h")) != -1) {
		switch (ch) {
		case '1':
			c1 = readnum(optarg);
			break;
		case '2':
			c2 = readnum(optarg);
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

	readcsv(fp, &tbl1);
	fclose(fp);

	readcsv(stdin, &tbl2);


	/* row 1 is names */
	for (size_t i=1; i < tbl1.nrecords; i++) {
		struct record *r = tbl1.records[i];
		printf("%s\n", r->line[c1]);
//		for (int j=0; j < r->ncol; j++)
//			printf("%s\t", r->line[j]);
//		puts("");
	}

	puts("");

	for (size_t i=1; i < tbl2.nrecords; i++) {
		struct record *r = tbl2.records[i];
		printf("%s\n", r->line[c2]);
//		for (int j=0; j < r->ncol; j++)
//			printf("%s\t", r->line[j]);
//		puts("");
	}

	free_table(&tbl1);
	free_table(&tbl2);

	exit(EXIT_SUCCESS);
}

long
readnum(char *s)
{
	long num;
	char *end;

	if ((num = strtol(s, &end, 10)) < 1) {
		fprintf(stderr, "error: option is less than 1 --%s\n", s);
		exit(EXIT_FAILURE);
	}
	if (*end) {
		fprintf(stderr, "error: illegal field number -- %s\n", s);
		exit(EXIT_FAILURE);
	}
	return --num;
}

int
readcsv(FILE *fp, struct table *tbl)
{
	// TODO: handle comma inside quotes?
	char *line = NULL;
	size_t linesz = 0;
	ssize_t linelen;
	
	char *sp, *ep, *qp; 	/* start pointer, end pointer and quote pointer */
	char *rp; 		/* carriage return pointer */

	(void)qp;

	while ((linelen = getline(&line, &linesz, fp)) != -1) {
		if (line[linelen - 1] == '\n')
			line[linelen - 1] = '\0';

		if ((rp = strchr(line, '\r')) != NULL) 
			*rp = '\0';

		/* ignore lines without delimiter */
		if ((ep = strchr(line, delim)) == NULL)
			continue;

		struct record *rec = (struct record *) malloc(sizeof(struct record));
		rec->ncol = 0; 		
		rec->line = 0;

		sp = line;
		for (;;) {
			size_t len;
			char *tmp;

			rec->line = realloc(rec->line, (size_t)(1 + rec->ncol) * sizeof(char *));

			len = (size_t)(ep - sp);
			tmp = (char *) malloc((len+1) * sizeof(char));
			(void)strncpy(tmp, sp, len);
			tmp[len] = '\0';

			rec->line[rec->ncol++] = tmp;

			sp = ep;
			if (*sp == '\0')
				break;
			sp++;

			if ((ep = strchr(sp, delim)) == NULL)
				ep = strchr(sp, '\0');
		}
		tbl->records = realloc(tbl->records, (size_t)(1 + tbl->nrecords) * sizeof(struct record *));
		tbl->records[tbl->nrecords++] = rec;
	}

	free(line);
	return 0;
}

void
free_table(struct table *tbl)
{
	struct record *r;
	for (size_t i = 0; i < tbl->nrecords; i++) {
		r = tbl->records[i];
		for (int j = 0; j < r->ncol; j++) {
			free(r->line[j]);
		}
		free(r->line);
		free(r);
	}
	free(tbl->records);
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
