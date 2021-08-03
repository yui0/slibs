/* public domain Simple, Minimalistic, CSV library
 *	©2021 Yuichiro Nakada
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define CSV_DELIM	"\t"
#define CSV_DELIM	","

typedef struct row_tag {
	int index;
	double *data;
} row_t;

size_t csv_get_col_count(FILE *is)
{
	size_t col_count = 1;
	int ch;
	while ((ch = fgetc(is)) != EOF && ch != '\n') {
		if (ch == ',') ++col_count;
	}
	rewind(is);
	return col_count;
}

row_t* csv_read(FILE *is, size_t *cols, size_t *rows)
{
	*cols = csv_get_col_count(is);
	*rows = 0;
	row_t *csv = NULL;

	size_t current_row = 0;
	char line[4098];
	while (fgets(line, 4098, is)) {
		csv = (row_t*)realloc(csv, (current_row +1)* sizeof(row_t));
		csv[current_row].data = calloc(*cols /*-1*/, sizeof(double));

		int n = 0;
		const char* tok;
		for (tok = strtok(line, CSV_DELIM); tok && *tok; n++, tok = strtok(NULL, CSV_DELIM"\n")) {
			csv[current_row].data[n] = atof(tok);
			printf("%f ", csv[current_row].data[n]);
		}
		printf("\n");
		current_row++;
	}
	*rows = current_row;

	return csv;
}

void csv_free(row_t *csv, size_t rows)
{
	for (size_t row = 0; row < rows; ++row) {
		free(csv[row].data);
	}
	free(csv);
}

double csv_get_value(row_t *csv, int col_index, size_t cols, int row_index, size_t rows)
{
	size_t col;
	for (col = 1; csv[0].data[col] != col_index && col < cols; ++col);
	if (col >= cols || csv[0].data[col] != col_index) {
		return 0.;
	}

	size_t row;
	for (row = 1; csv[row].index != row_index && row < rows; ++row);
	if (row >= rows || csv[row].index != row_index) {
		return 0.;
	}

	return csv[row].data[col];
}
