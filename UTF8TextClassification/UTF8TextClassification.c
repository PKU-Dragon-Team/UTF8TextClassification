#include "UTF8TextClassification.h"

void load_texts(const struct text_list * l, FILE * in) {
	if (l != NULL && in != NULL) {
		uchar * s[BUF_SIZE] = {};
		while (fgets(s, BUF_SIZE, in) != EOF) {
			
		}
	}
}

void output_texts(const struct text_list * l, FILE * out) {
	
}