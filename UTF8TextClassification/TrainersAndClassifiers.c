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
    llu truePositive = 0;
    llu trueNegative = 0;
    llu falsePositive = 0;
    llu falseNegative = 0;
    llu PTotal = 0;
    llu NTotal = 0;
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
        bool pflag = false;
        for (type_t j = 0; j < TYPE_COUNT; ++j) {
            if (p_tl->list[i].types[j] == 1) {
                pflag = true;
            }
        }
        if (pflag) {
            ++PTotal;
        }
        else {
            ++NTotal;
        }
        {
            bool cflag = false;
            for (type_t j = 0; j < TYPE_COUNT; ++j) {
                if (test[j] == 1) {
                    cflag = true;
                }
            }
            if (pflag && cflag) {
                ++truePositive;
            }
            if (!pflag && !cflag) {
                ++trueNegative;
            }
            if (!pflag && cflag) {
                ++falsePositive;
            }
            if (pflag && !cflag) {
                ++falseNegative;
            }
        }
        clear_hash_vector(&temp);
    }
    // TODO: 把TYPE_COUNT个分类的命中率也输出出来
    printf("\n%llu Total\n"
           "\tPositive Total: %llu\n"
           "\t\tTrue Positive: %llu\n"
           "\t\t\trate: %Lf%%\n"
           "\t\tFalse Negative: %llu\n"
           "\t\t\trate: %Lf%%\n"
           "\tNegative Total: %llu\n"
           "\t\tTrue Negative: %llu\n"
           "\t\t\trate: %Lf%%\n"
           "\t\tFalse Positive: %llu\n"
           "\t\t\trate: %Lf%%\n", p_tl->len, PTotal, truePositive, (Lf)truePositive / (Lf)PTotal * 100, falseNegative, (Lf)falseNegative / (Lf)PTotal * 100, NTotal, trueNegative, (Lf)trueNegative / (Lf)NTotal * 100, falsePositive, (Lf)falsePositive / (Lf)NTotal * 100);
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

