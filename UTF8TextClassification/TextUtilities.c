#include "TextUtilities.h"

int init_text(struct text ** pp_text, struct ustring * us, int8_t types[TYPE_COUNT]) {
    if (pp_text == NULL) {
        return -1;
    }

    *pp_text = malloc(sizeof(struct text));
    if (*pp_text == NULL) {
        return -1;
    }

    (*pp_text)->us = us;

    for (type_t i = 0; i < TYPE_COUNT; ++i) {
        (*pp_text)->types[i] = types[i];
    }
    return 0;
}

int clear_text(struct text ** pp_text) {
    if (pp_text == NULL) {
        return -1;
    }

    if (*pp_text != NULL) {
        if ((*pp_text)->us != NULL) {
            clear_ustring(&(*pp_text)->us);
        }
        free(*pp_text);
        *pp_text = NULL;
    }
    return 0;
}

int init_text_list(struct text_list ** pp_tlist, const struct text a_text[], llu len) {
    if (pp_tlist == NULL) {
        return -1;
    }

    *pp_tlist = malloc(sizeof(struct text_list));
    if (*pp_tlist == NULL) {
        return -1;
    }

    if (a_text == NULL) {
        (*pp_tlist)->list = NULL;
        (*pp_tlist)->len = 0;
    }
    else {
        struct text * p = calloc((size_t)len, sizeof(struct text));
        if (p == NULL) {
            return -1;
        }
        memcpy(p, a_text, (size_t)len);
        (*pp_tlist)->list = p;
        (*pp_tlist)->len = len;
    }
    return 0;
}

int resize_text_list(struct text_list * p_tlist, llu len) {
    if (p_tlist == NULL) {
        return -1;
    }

    struct text * p = realloc(p_tlist->list, (size_t)(len * sizeof(struct text)));
    if (p == NULL) {
        return -1;
    }
    else {
        p_tlist->list = p;
        p_tlist->len = len;
    }
    return 0;
}

int clear_text_list(struct text_list ** pp_tlist) {
    if (pp_tlist == NULL) {
        return -1;
    }
    for (llu i = 0; i < (*pp_tlist)->len; ++i) {
        clear_ustring(&(*pp_tlist)->list[i].us);
    }
    free((*pp_tlist)->list);
    free(*pp_tlist);
    *pp_tlist = NULL;

    return 0;
}

int get_char_analysis(const struct text_list * cp_tlist, struct uchar_analysis * uca) {
    if (cp_tlist == NULL || uca == NULL) {
        return -1;
    }

    for (llu i = 0; i < cp_tlist->len; ++i) {
        for (llu j = 0; j < cp_tlist->list[i].us->string_len; ++j) {
            ++uca->uchar_list[cp_tlist->list[i].us->string[j]];
            ++uca->total_count;
        }
    }
    return 0;
}

int output_char_analysis(FILE * out, const struct uchar_analysis * uca) {
    if (out == NULL || uca == NULL) {
        return -1;
    }

    fprintf(out, "Total Characters: %llu\n", uca->total_count);
    for (llu i = 0; i < MAX_UNICODE; ++i) {
        if (uca->uchar_list[i] != 0) {
            fprintf(out, "0x%llX\t%lld\n", i, uca->uchar_list[i]);
        }
    }
    return 0;
}

int parse_type(const uchar a_type[], int8_t types[TYPE_COUNT]) {
    if (a_type == NULL) {
        return -1;
    }

    for (type_t i = 0; i < TYPE_COUNT; ++i) {
        types[i] = -1;
    }

    llu n = strlen(a_type);
    type_t ti = 0;
    for (llu i = 0; i < n; ++i) {
        if (ti < TYPE_COUNT && a_type[i] != '*' && a_type[i] != '(' && a_type[i] != ')' && a_type[i] != '\n') {
            types[ti] = a_type[i] - '0';
            ++ti;
        }
    }
    return 0;
}

int load_texts(FILE * input, struct text_list * p_tlist) {
    if (p_tlist == NULL || input == NULL) {
        return -1;
    }
    else {
        enum {
            init,
            first,
            text,
            type,
            error
        } state = init;

        int8_t types[TYPE_COUNT] = { 0 };
        struct ustring * us = NULL;
        llu i = 0;
        llu line = 0;
        bool flag = true;

        while (flag) {
            uchar * buf = calloc(BUF_SIZE, sizeof(uchar));
            bool newline = false;

            if (fgets(buf, BUF_SIZE, input) == NULL) {
                flag = false;
            }
            switch (state) {
            case init:
                if (strnlen(buf, BUF_SIZE) < 4 || !isdigit(buf[3])) {
                    state = error;
                }
                else {
                    state = first;
                }
                break;
            case first:
                if (buf[0] == '*') {
                    state = error;
                }
                else {
                    init_ustring(&us, index, buf, BUF_SIZE);
                    if (buf[BUF_SIZE - 1] == '\0' || buf[BUF_SIZE - 1] == '\n') {
                        newline = true;
                    }
                    state = text;
                }
                break;
            case text:
                if (buf[0] == '*') {
                    if (strnlen(buf, BUF_SIZE) < 5 || buf[3] != '(') {
                        state = error;
                    }
                    else {
                        parse_type(buf, types);
                        state = type;
                    }
                }
                else {
                    struct ustring * temp = NULL;
                    init_ustring(&temp, index, buf, BUF_SIZE);
                    cat_ustring(us, temp);
                    clear_ustring(&temp);
                }
                break;
            case type:
                if (buf[0] != '*') {
                    state = error;
                }
                else {
                    struct text * t = NULL;
                    init_text(&t, us, types);
                    if (i >= p_tlist->len) {	// Dynamic Expand
                        if (resize_text_list(p_tlist, p_tlist->len * 2 + 1) != 0) {
                            return -1;
                        }
                    }
                    p_tlist->list[i] = *t;
                    ++i;
                    state = first;
                }
                break;
            case error:
                printf("Load error at line %llu, %llu texts have been read.\n", line, i);
                return -1;
                break;
            default:
                break;
            }
            if (newline) {
                ++line;
            }
            free(buf);
        }
        resize_text_list(p_tlist, i);
    }
    return 0;
}

int output_texts(FILE * out, const struct text_list * p_tlist) {
    if (p_tlist == NULL || out == NULL) {
        return -1;
    }

    for (llu i = 0; i < p_tlist->len; ++i) {
        fprintf(out, "***%llu\n%s***(", i + 1, p_tlist->list[i].us->string);
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tlist->list[i].types[j] != -1) {
                fprintf(out, "%d", p_tlist->list[i].types[j]);
            }
        }
        fprintf(out, ")%s", (i == p_tlist->len - 1) ? "" : "\n");
    }
    return 0;
}

int save_vectors(FILE * output, struct hash_vector * const ap_hv[TYPE_COUNT + 1]) {
    if (output == NULL || ap_hv == NULL) {
        return -1;
    }

    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        save_vector(output, ap_hv[i]);
    }
    return 0;
}

int load_vectors(FILE * input, struct hash_vector * ap_hv[TYPE_COUNT + 1]) {
    if (input == NULL || ap_hv == NULL) {
        return -1;
    }

    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        if (load_vector(input, ap_hv[i]) != 0) {
            return -1;
        }
    }
    return 0;
}