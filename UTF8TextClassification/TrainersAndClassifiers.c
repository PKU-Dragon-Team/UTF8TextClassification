#include "TrainersAndClassifiers.h"

int naive_trainer(struct hash_vector * ap_hv[TYPE_COUNT + 1], const struct text_list * p_tl, Parser parser, Checker checker) {
    if (ap_hv == NULL || p_tl == NULL) {
        return -1;
    }

    for (type_t i = 0; i < TYPE_COUNT; ++i) {
        if (ap_hv[i] == NULL) {
            return -1;
        }
    }

    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        add_hash_vector(ap_hv[TYPE_COUNT], temp);
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tl->list[i].types[j] == 1) {
                add_hash_vector(ap_hv[j], temp);
            }
            else if (p_tl->list[i].types[j] == 0) {
                sub_hash_vector(ap_hv[j], temp);
            }
        }
        clear_hash_vector(&temp);
    }

    for (type_t i = 0; i < TYPE_COUNT + 1; ++i) {
        low_cut_hash_vector(ap_hv[i], 1);
    }
    return 0;
}

int KNN_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker) {
    llu correct = 0;
    llu total = 0;
    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        Lf cos[TYPE_COUNT] = { 0 };

        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            cos[j] = cos_hash_vector(statistic[j], temp);
            fprintf(out, "%Lf%s", cos[j], (j == TYPE_COUNT - 1) ? "" : "\t");
        }
        fprintf(out, "%s", (i == p_tl->len - 1) ? "" : "\n");

        int8_t test[TYPE_COUNT] = { 0 };
        {
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (cos[j] > 0) {
                    test[j] = 1;
                }
            }
        }
        bool tflag = false;
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tl->list[i].types[j] == 1) {
                tflag = true;
            }
        }
        if (tflag) {
            ++total;
        }
        {
            bool cflag = true;
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (test[j] != p_tl->list[i].types[j]) {
                    cflag = false;
                }
            }
            if (tflag && cflag) {
                ++correct;
            }
        }
        clear_hash_vector(&temp);
    }
    printf("\n%llu Corrects\n%llu Total\n%Lf%% Right\n", correct, total, (Lf)correct / (Lf)total * 100);
    return 0;
}

int KNN_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker) {
    for (llu i = 0; i < p_tl->len; ++i) {
        struct hash_vector * temp;
        struct ustring_parse_list * p_list;
        Lf cos[TYPE_COUNT] = { 0 };

        init_hash_vector(&temp);
        init_uspl(&p_list);
        parser(p_list, p_tl->list[i].us, checker);
        append_hash_vector(temp, p_tl->list[i].us, p_list);

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            cos[j] = cos_hash_vector(statistic[j], temp);
        }

        int8_t a_class[TYPE_COUNT] = { 0 };
        {
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (cos[j] > 0) {
                    a_class[j] = 1;
                }
            }
        }

        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            fprintf(out, "%d", a_class[j]);
        }
        fprintf(out, "\n");
        clear_hash_vector(&temp);
    }
    return 0;
}

