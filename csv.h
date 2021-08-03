/* public domain Simple, Minimalistic, CSV library
 *	©2021 Yuichiro Nakada
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//#define CSV_DELIM	"\t"
#define CSV_DELIM	","

typedef struct row_tag {
	int index;
	double *data;
} row_t;

// -1 error else 0 - 86399
/*long tsconvert(const char *timestam)
{
	unsigned h, m, s;
	int n = 0;
	int cnt = sscanf(timestam, "%2u:%2u:%2u %n", &h, &m, &s, &n);
	if (cnt != 3 || timestam[n] != '\0') return -1; // Format Error
	if (h >= 24 || m >= 60 || s >= 60) return -1; // Range Error
	// 0 - 86400-1
	return ((h*60 + m)*60L + s;
}*/
struct tm convert_YMD(const char *timestam)
{
	struct tm t = {0};
	int n = 0;
	int cnt = sscanf(timestam, "%d/%d/%d %n", &t.tm_year, &t.tm_mon, &t.tm_mday, &n);
	if (cnt != 3 || timestam[n] != '\0') { t.tm_year=0; return t; }
//	strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
//	strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
	return t;
}

size_t csv_get_col_count(FILE *is)
{
	size_t col_count = 0;
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
		csv[current_row].data = calloc(*cols, sizeof(double));

		int n = 0;
		const char* tok;
		for (tok = strtok(line, CSV_DELIM); tok && *tok; n++, tok = strtok(NULL, CSV_DELIM"\n")) {
			struct tm t = convert_YMD(tok);
			if (!t.tm_year) {
				csv[current_row].data[n] = atof(tok);
//				printf("%f ", csv[current_row].data[n]);
			} else { // date
				csv[current_row].data[n] = t.tm_year*10000 +t.tm_mon*100 +t.tm_mday;
			}
		}
//		printf("\n");
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
