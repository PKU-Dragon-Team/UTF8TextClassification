#ifndef TRAINER_CLASSIFIER
#define TRAINER_CLASSIFIER

#include "UStringVector.h"
#include "TextUtilities.h"

// The trainer which simply add the vector when the class meets and sub the vector when not
int naive_trainer(struct hash_vector * ap_hv[TYPE_COUNT + 1], const struct text_list * p_tl, Parser parser, Checker checker);

// The tester using K-Nearest Neighbors algorithm
int KNN_tester(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

// The classifier using K-Nearest Neighbors algorithm
int KNN_classifier(FILE * out, struct text_list * p_tl, struct hash_vector * const statistic[TYPE_COUNT + 1], Parser parser, Checker checker);

// Trainer that add the vector
int NaiveBayes_trainer(struct hash_vector * ap_hv[TYPE_COUNT + 1], const struct text_list * p_tl, Parser parser, Checker checker);

#endif