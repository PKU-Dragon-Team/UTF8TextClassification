#include "UTF8TextClassification.h"

int load_texts(const struct text_list * l, FILE * in) {
	if (l == NULL || in == NULL) {
		return -1;
	}
	else {
		uchar * s[BUF_SIZE] = { 0 };
		uchar * ts[TYPE_COUNT] = { 0 };

		while (fscanf_s(in, "***%*d") != EOF) {
			fgets(s, BUF_SIZE, in);
			fscanf_s(in, "***(%s)", ts, TYPE_COUNT);
		}
	}
	return 0;
}

int parse_type(uchar * ts, int8_t types[TYPE_COUNT]) {
	return 0;
}

int output_texts(const struct text_list * l, FILE * out) {
	return 0;
}