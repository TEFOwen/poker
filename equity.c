#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "card.h"
#include "equity.h"

uint32_t choose(uint32_t n, uint32_t k) {
    if (k == 0)
        return 1;
    else
        return (n * choose(n - 1, k - 1)) / k;
}

void equity_inner(evaluator_t *evaluator, equityinfo_t *equity,
                  handrank_t *handRanks, size_t nHands, int32_t n,
                  card_t *usedCards, size_t nUsedCards, card_t minCard) {
    if (n == 0) {
        equity->total++;

        handrank_t best = handRanks[0];
        int winners = 1;

        for (int i = 1; i < nHands; ++i) {
            if (handRanks[i] > best) {
                best = handRanks[i];
                winners = 1;
            } else if (handRanks[i] == best)
                winners++;
        }

        for (int i = 0; i < nHands; ++i) {
            if (handRanks[i] == best) {
                if (winners > 1)
                    equity->equities[i].chopOuts++;
                else
                    equity->equities[i].winOuts++;
                equity->equities[i].equity += 1.0 / (double)winners;
            }
        }

        return;
    }

    handrank_t newHandRanks[nHands];
    card_t newUsedCards[nUsedCards + 1];
    memcpy(newUsedCards, usedCards, sizeof(card_t) * nUsedCards);
    for (int32_t card = minCard; card < 53 - n + 1; card++) {
        bool duplicate = false;
        for (int j = 0; j < nUsedCards; ++j)
            if (card == usedCards[j]) {
                duplicate = true;
                break;
            }
        if (duplicate)
            continue;

        newUsedCards[nUsedCards] = card;
        for (int i = 0; i < nHands; ++i) {
            newHandRanks[i] = evaluator->handRanks[handRanks[i] + card];
        }
        equity_inner(evaluator, equity, newHandRanks, nHands, n - 1,
                     newUsedCards, nUsedCards + 1, card + 1);
    }
}

equityinfo_t *equity_calc(evaluator_t *evaluator, card_t *hands, size_t nHands,
                          card_t *community, size_t nCommunity) {
    assert(nCommunity == 0 || nCommunity == 3 || nCommunity == 4);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    size_t nUsedCards = nHands * 2 + nCommunity;
    card_t usedCards[nUsedCards];
    memcpy(usedCards, hands, sizeof(card_t) * nHands * 2);
    if (community)
        memcpy(usedCards + (nHands * 2), community,
               sizeof(card_t) * nCommunity);

    handrank_t handRanks[nHands];

    for (int i = 0; i < nHands; ++i) {
        handRanks[i] = evaluator->handRanks[53 + hands[i * 2]];
        handRanks[i] = evaluator->handRanks[handRanks[i] + hands[i * 2 + 1]];

        for (int j = 0; j < nCommunity; ++j) {
            handRanks[i] = evaluator->handRanks[handRanks[i] + community[j]];
        }
    }

    equityinfo_t *equity = malloc(sizeof(equityinfo_t));
    equity->equities = calloc(nHands, sizeof(handequity_t));
    equity->total = 0;

    equity_inner(evaluator, equity, handRanks, nHands, 5 - nCommunity,
                 usedCards, nUsedCards, 1);

    for (int i = 0; i < nHands; ++i) {
        equity->equities[i].win =
            (double)equity->equities[i].winOuts / (double)equity->total;
        equity->equities[i].chop =
            (double)equity->equities[i].chopOuts / (double)equity->total;
        equity->equities[i].equity /= (double)equity->total;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    equity->time = time;

    return equity;
}

void equity_destroy(equityinfo_t *equity) {
    free(equity->equities);
    free(equity);
}
