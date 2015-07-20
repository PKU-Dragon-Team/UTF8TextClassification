#ifndef TEXTUTIL
#define TEXTUTIL
#include "UStringVector.h"

#define TYPE_COUNT 3    // The number of predefine classes of text

// The struct of a text.
struct text {
    struct ustring * us;
    int8_t types[TYPE_COUNT];
};

// The struct of a list of text
struct text_list {
    struct text * list;
    llu len;
};

int init_text(struct text ** pp_text, struct ustring * us, int8_t types[TYPE_COUNT]);
int clear_text(struct text ** pp_text);

int init_text_list(struct text_list ** pp_tlist, const struct text a_text[], llu len);
int resize_text_list(struct text_list * p_tlist, llu len);
int clear_text_list(struct text_list ** pp_tlist);

int get_char_analysis(const struct text_list * cp_tlist, struct uchar_analysis * uca);
int output_char_analysis(FILE * out, const struct uchar_analysis * uca);

int save_vectors(FILE * output, struct hash_vector * const ap_hv[TYPE_COUNT + 1]);
int load_vectors(FILE * input, struct hash_vector * ap_hv[TYPE_COUNT + 1]);
#endif