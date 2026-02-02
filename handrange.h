#ifndef HANDRANGE_H
#define HANDRANGE_H

#include <stddef.h>

#include "card.h"

typedef struct HandEntry {
    card_t cards[2];
    float frequency; // 0 < = frequency <= 1.0
} handentry_t;

typedef struct HandRange {
    handentry_t *hands;
    size_t size;
} handrange_t;

handrange_t *handrange_create(char *range);
const handentry_t *handrange_get(handrange_t *handRange, size_t idx);
void handrange_destroy(handrange_t *handRange);

#endif // HANDRANGE_H
