#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct record {
	char 	 *id;
	char 	**fields;
	int 	  nfields;
};

struct table {
	char 		 *filename;
	long 		  idfield;
	struct record 	 *names;
	struct record 	**records;
	size_t 		  nrecords;
};


int	 	 readcsv(FILE *, struct table *);
void	 	 usage(void);
void	 	 free_table(struct table *);
void	 	 free_record(struct record *);
long		 readnum(char *);
struct record 	*find_record(const struct table *,  char *);
int 		 assert_uniq_ids(const struct table *);
int 		 assert_no_new_ids(const struct table *, const struct table *);
void 		 find_changes(struct table *, struct table *);
int 		 column_index(struct table *, char *);
void 		 write_table(struct table *, FILE *);
int		 assert_columns_exist(struct table *, struct table *);
int 		 assert_fields_number(struct table *);

char delim;

struct table tbl1 = { NULL, 0, NULL, NULL, 0 };
struct table tbl2 = { NULL, 0, NULL, NULL, 0 };

int
main(int argc, char *argv[])
{
	FILE *fp1, *fp2;
	delim = ',';

	int ch;
	while ((ch = getopt(argc, argv, "1:2:d:h")) != -1) {
		switch (ch) {
		case '1':
			tbl1.idfield = readnum(optarg);
			break;
		case '2':
			tbl2.idfield = readnum(optarg);
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

	if (argc == 0) {
		fprintf(stderr, "error: missing args\n");
		usage();
		exit(EXIT_FAILURE);
	} 

	if ((fp1 = fopen(*argv, "r")) == NULL) {
		err(EXIT_FAILURE, "can't open %s", *argv);
	}
	tbl1.filename = strdup(*argv);
	
	fp2 = argc == 1 ? stdin : (fopen(*(++argv), "r"));
	if (fp2 == NULL) {
		err(EXIT_FAILURE, "can't open %s", *argv);
	}
	tbl2.filename = strdup(argc == 1 ? "stdin" : *argv);

	if (readcsv(fp1, &tbl1) < 1) {
		fclose(fp1);
		free_table(&tbl1);
		exit(EXIT_FAILURE);
	}
	if (readcsv(fp2, &tbl2) < 1) {
		fclose(fp2);
		free_table(&tbl2);
		exit(EXIT_FAILURE);
	}
	fclose(fp1);
	fclose(fp2);

	if (tbl1.idfield >= tbl1.names->nfields)
	    errx(EXIT_FAILURE, "%s: no such column: %ld", tbl1.filename, tbl1.idfield + 1);
	if (tbl2.idfield >= tbl2.names->nfields)
	    errx(EXIT_FAILURE, "%s: no such column: %ld", tbl2.filename, tbl2.idfield + 1);

	// TODO: assert columsn are unique

	if (assert_uniq_ids(&tbl1) != 0 ||  assert_uniq_ids(&tbl2) != 0 || 
		assert_fields_number(&tbl1) || assert_fields_number(&tbl2) ||
		assert_no_new_ids(&tbl1, &tbl2) != 0) {
		free_table(&tbl1);
		free_table(&tbl2);
		exit(EXIT_FAILURE);
	}

	// TODO: add flag if want to add a new column. error if not
	if (assert_columns_exist(&tbl1, &tbl2) != 0) {
		free_table(&tbl1);
		free_table(&tbl2);
		exit(EXIT_FAILURE);
	}

	// create changes_stack
	// apply changes from stack if not empty

	find_changes(&tbl1, &tbl2);

	write_table(&tbl1, stdout);

	free_table(&tbl1);
	free_table(&tbl2);

	exit(EXIT_SUCCESS);
}

long
readnum(char *s)
{
	long num;
	char *end;

	if ((num = strtol(s, &end, 10)) < 1 || *end) {
		errx(EXIT_FAILURE, "illegal column ID indicator: %s", s);
	}

	return --num;
}

int
readcsv(FILE *fp, struct table *tbl)
{
	// TODO: handle comma inside quotes?
	int first_line = 1;
	char *line = NULL;
	size_t linesz = 0;
	ssize_t linelen;
	
	char *sp, *ep, *qp; 	/* start pointer, end pointer and quote pointer */
	char *rp; 		/* carriage return pointer */
	(void)qp;

	while ((linelen = getline(&line, &linesz, fp)) != -1) {
		if (line[linelen - 1] == '\n')
			line[linelen - 1] = '\0';
		if ((rp = strrchr(line, '\r')) != NULL) 
			*rp = '\0';

		/* ignore lines without delimiter */
		// TODO: add option to err if empty lines
		if ((ep = strchr(line, delim)) == NULL)
			continue;

		struct record *rec = (struct record *) malloc(sizeof(struct record));
		if (rec == NULL) {
			fprintf(stderr, "error in malloc: cannot get space for record\n");
			return -1;
		}
		rec->fields = NULL;
		rec->nfields = 0; 		

		sp = line;
		for (;;) {
			size_t len;
			char *tmp;

			len = (size_t)(ep - sp);
			tmp = (char *) calloc(len+1, sizeof(char));
			if (tmp == NULL) {
				fprintf(stderr, "error in calloc: cannot get space for tmp\n");
				return -1;
			}
			(void)strncpy(tmp, sp, len);
			tmp[len] = '\0';

			rec->fields = realloc(rec->fields, (size_t)(rec->nfields + 1) * sizeof(char *));
			rec->fields[rec->nfields++] = tmp;

			sp = ep;
			if (*sp == '\0')
				break;
			sp++;

			if ((ep = strchr(sp, delim)) == NULL)
				ep = strchr(sp, '\0');
		}
		if (first_line) {
			tbl->names = rec;
			first_line = 0;
			continue;
		} 
		tbl->records = realloc(tbl->records, (size_t)(1 + tbl->nrecords) * sizeof(struct record *));
		tbl->records[tbl->nrecords++] = rec;
	}
	free(line);

	/* haven't read column names */
	if (first_line) {
		fprintf(stderr, "%s: missing column names\n", tbl->filename);
		return -1;
	} 
	if (!tbl->nrecords) {
		fprintf(stderr, "%s: no rows to read\n", tbl->filename);
		return -1;
	}

	return (int)tbl->nrecords;
}

void
free_table(struct table *tbl)
{
	free_record(tbl->names);
	for (size_t i = 0; i < tbl->nrecords; i++) {
		free_record(tbl->records[i]);
	}
	free(tbl->filename);
	free(tbl->records);
}

void
free_record(struct record *r)
{
	for (int i = 0; i < r->nfields; i++) 
		free(r->fields[i]);
	free(r->fields);
	free(r);
}

struct record *
find_record(const struct table *tbl,  char *s)
{
	size_t i;
	long id;
	struct record *r;

	id = tbl->idfield;
	for (i = 0; i < tbl->nrecords; i++) {
		r = tbl->records[i];
		if (!strcmp(r->fields[id], s))
			return r;
	}
	return NULL;
}

int
assert_uniq_ids(const struct table *tbl)
{
	int errs = 0;
	size_t i, j;
	long id;
	char *tmp_id;

	id = tbl->idfield;
	for (i = 0; i < tbl->nrecords; i++) {
		tmp_id = tbl->records[i]->fields[id];
		for (j = i + 1; j < tbl->nrecords; j++) {
			if (!strcmp(tmp_id, tbl->records[j]->fields[id]) && i != j) {
				fprintf(stderr, "%s: id %s is not unique\n", tbl->filename, tmp_id);
				errs += 1;
			}
		}
	}
	return errs;
}

int 
assert_no_new_ids(const struct table *tbl1, const struct table *tbl2)
{
	/* 
	 * iterate over all ids from table 2 and make sure that
	 * all of them are present in table 1 
	 */
	int errors = 0;
	size_t i;
	long id;
	struct record *r;

	id = tbl1->idfield;

	for (i = 0; i < tbl2->nrecords; i++) {
		r = tbl2->records[i];
		if ((find_record(tbl1, r->fields[id])) == NULL) {
			fprintf(stderr, "%s: id %s not found in %s\n", 
				tbl2->filename, r->fields[id], tbl1->filename);
			errors += 1;
		}
	}
	return errors;
}

void
find_changes(struct table *tbl1, struct table *tbl2)
{
	/*
	 *  Find modifications that need to be applied to table 1
	 */
	size_t i;
	char *tmp_id;
	struct record *r1, *r2;		/* record input (from table 2), record target (tbl 1) */
	int col1, col2; 		/* column index for records 1 and 2 */
	char *f1; 			/* pointer to field for records 1 */

	/* 
	 *  Iterate over records from table 2 (input table).
	 *  For each one retrieve the record from table 1 (target table).
	 *  Then iterate over each column from r_input (rec from table 2), find the 
	 *  corresponding value in r_target (record from table 1).
	 */
	for (i = 0; i < tbl2->nrecords; i++) {
		r2 = tbl2->records[i];
		tmp_id = r2->fields[tbl2->idfield];

		r1 = find_record(tbl1, tmp_id);
		if (r1 == NULL) {
			fprintf(stderr, "error: can't find id %s\n", tmp_id);
			return;
		}

		/* 
		 * iterate over columns from table 2 and get values
		 * from each field in record
		 */
		for (col2 = 0; col2 < tbl2->names->nfields; col2++) {
			if (col2 == tbl2->idfield)
				continue;

			// TODO: check not -1. although this must be assert in main 
			col1 = column_index(tbl1, tbl2->names->fields[col2]);
			f1 = r1->fields[col1];

			// TODO: handle error if not empty
			if (!strcmp(f1, "") || !strcmp(f1, "NA") || !strcmp(f1, "NaN")) {
				free(r1->fields[col1]);
				r1->fields[col1] = strdup(r2->fields[col2]);
			}
		}
	}
}

int
column_index(struct table *tbl, char *s)
{
	for (int i = 0; i < tbl->names->nfields; i++) {
		if (!strcmp(tbl->names->fields[i], s)) {
			return i;
		}
	}
	return -1;
}

void
write_table(struct table *tbl, FILE *fp)
{
	struct record *r;
	char c;

	for (int i = 0; i < tbl->names->nfields; i++) {
		c = (i != tbl->names->nfields - 1) ? delim : '\n';
		fprintf(fp, "%s%c", tbl->names->fields[i], c);
	}

	for (size_t i = 0; i < tbl->nrecords; i++) {
		r = tbl->records[i];
		for (int j = 0; j < tbl->names->nfields; j++) {
			c = (j != tbl->names->nfields - 1) ? delim : '\n';
			fprintf(fp, "%s%c", r->fields[j], c);
		}
	}
}

int
assert_columns_exist(struct table *tbl1, struct table *tbl2)
{
	int ret = 0;
	int found; 
	char *fname1, *fname2;
	for (int i = 0; i < tbl2->names->nfields; i++) {
		found = 0; 
		fname2 = tbl2->names->fields[i];
		for (int j = 0; j < tbl1->names->nfields; j++) {
			fname1 = tbl1->names->fields[j];
			if (!strcmp(fname1, fname2)) {
				found = 1;
				break;
			}
		}

		if (!found) {
			ret -= 1;
			fprintf(stderr, "col not found: %s\n", fname2);
		}
	}
	return ret;
}

int
assert_fields_number(struct table *tbl)
{
	int err = 0;
	for (size_t i = 0; i < tbl->nrecords; i++) {
		if (tbl->records[i]->nfields != tbl->names->nfields) {
			fprintf(stderr, "%s: id: %s -- record with different number of rows\n", 
				tbl->filename, tbl->records[i]->fields[tbl->idfield]);
			err += 1;
		}
	}
	return err;
}

void
usage(void)
{
	fprintf(stderr, "usage: csvfill [-d delim] [-1 COL1] [-2 COL2] FILE1 FILE2\n");
	fprintf(stderr, "\nUpdate FILE1 with data from FILE2\n");
	fprintf(stderr, " -d  change delimiter. comma by default\n");
	fprintf(stderr, " -1  number of column with ID in FILE1. Default is 1\n");
	fprintf(stderr, " -2  number of column with ID in FILE2. Default is 1\n");
}
