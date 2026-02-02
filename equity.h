#ifndef EQUITY_H
#define EQUITY_H

#include <stddef.h>
#include <stdint.h>

#include "card.h"
#include "evaluator.h"

typedef struct HandEquity {
    float win, chop;
    uint64_t winOuts, chopOuts;
    double equity;
} handequity_t;

typedef struct EquityInfo {
    uint64_t total;
    float time;
    handequity_t *equities;
} equityinfo_t;

equityinfo_t *equity_calc(evaluator_t *evaluator, card_t *hands, size_t nHands,
                          card_t *community, size_t nCommunity);
void equity_destroy(equityinfo_t *equity);

#endif // EQUITY
