#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "card.h"
#include "handrange.h"

char rankChars[] = "23456789TJQKA";

int get_two_ranks(const char *str, size_t *idx, rank_t *ranks) {
    bool found = false;
    for (int i = 0; i < sizeof(rankChars); ++i)
        if (str[*idx] == rankChars[i]) {
            ranks[0] = i;
            (*idx)++;
            found = true;
            break;
        }

    if (!found)
        return 1;

    found = false;
    for (int i = 0; i < sizeof(rankChars); ++i)
        if (str[*idx] == rankChars[i]) {
            ranks[1] = i;
            (*idx)++;
            found = true;
            break;
        }

    return 0;
}

bool card_filter(card_t *cards, bool suited, bool offsuit, bool pocketPairs,
                 rank_t *rangeLow, rank_t *rangeHigh) {
    bool allow =
        pocketPairs ? (CARD_RANK(cards[0]) == CARD_RANK(cards[1])) : true;
    if (suited)
        allow = allow && CARD_SUIT(cards[0]) == CARD_SUIT(cards[1]);
    if (offsuit)
        allow = allow && CARD_SUIT(cards[0]) != CARD_SUIT(cards[1]);

    if (rangeLow[0] != -1 && rangeLow[1] != -1)
        allow = allow && (CARD_RANK(cards[0]) > rangeLow[0] ||
                          (CARD_RANK(cards[0]) == rangeLow[0] &&
                           CARD_RANK(cards[1]) >= rangeLow[1]));

    if (rangeHigh[0] != -1 && rangeHigh[1] != -1)
        allow = allow && (CARD_RANK(cards[0]) <= rangeHigh[0] &&
                          CARD_RANK(cards[1]) <= rangeHigh[1]);

    return allow;
}

void resize_cards(handrange_t *handRange, size_t *allocated) {
    *allocated += 10;
    handRange->hands =
        realloc(handRange->hands, sizeof(handentry_t) * *allocated);
}

void push_hand(handrange_t *handRange, card_t *cards, float frequency,
               size_t *allocated) {
    if (handRange->size >= *allocated)
        resize_cards(handRange, allocated);
    handRange->hands[handRange->size].frequency = frequency;
    memcpy(handRange->hands[handRange->size].cards, cards, sizeof(card_t) * 2);
    ++handRange->size;
}

int compare_2_hands(const void *a_, const void *b_) {
    const handentry_t *a = (const handentry_t *)a_,
                      *b = (const handentry_t *)b_;

    if (a->cards[0] > b->cards[0])
        return 1;
    if (a->cards[0] < b->cards[0])
        return -1;

    return (a->cards[1] > b->cards[1]) - (a->cards[1] < b->cards[1]);
}

#define SWAP(x, y)                                                             \
    {                                                                          \
        x ^= y;                                                                \
        y ^= x;                                                                \
        x ^= y;                                                                \
    }

#define SWAP_RANKS(x, y)                                                       \
    SWAP(x[0], y[0]);                                                          \
    SWAP(x[1], y[1]);

handrange_t *handrange_create(char *range) {
    handrange_t *handRange = malloc(sizeof(handrange_t));
    handRange->size = 0;
    size_t allocated = 52;
    handRange->hands = malloc(sizeof(handentry_t) * allocated);

    size_t len = strlen(range);
    while (range[len - 1] == '\n' || range[len - 1] == ' ')
        range[--len] = 0;

    for (size_t i = 0; i < len;) {
        if (range[i] == ' ') {
            ++i;
            continue;
        }

        if (len - i < 2) {
            handrange_destroy(handRange);
            return 0;
        }

        bool suited = false, offsuit = false, pocketPairs = false;
        rank_t rangeLow[] = {-1, -1}, rangeHigh[] = {-1, -1};

        if (get_two_ranks(range, &i, rangeLow)) {
            handrange_destroy(handRange);
            return 0;
        }

        if (len - i > 0) {
            switch (range[i++]) {
            case '+':
                break;
            case '-':
                if (range[i] == 0 || range[i] == ' ' || range[i] == ':') {
                    SWAP_RANKS(rangeLow, rangeHigh);
                    break;
                }

                if (len - i < 2) {
                    handrange_destroy(handRange);
                    return 0;
                }

                get_two_ranks(range, &i, rangeHigh);

                pocketPairs =
                    rangeLow[0] == rangeLow[1] && rangeHigh[0] == rangeHigh[1];
                break;
            case 's':
                if (rangeLow[0] == rangeLow[1]) {
                    handrange_destroy(handRange);
                    return 0;
                }

                suited = true;
                switch (range[i++]) {
                case '+':
                    break;
                case '-':
                    if (range[i] == 0 || range[i] == ' ' || range[i] == ':') {
                        SWAP_RANKS(rangeLow, rangeHigh);
                        break;
                    }

                    if (len - i < 2) {
                        handrange_destroy(handRange);
                        return 0;
                    }

                    get_two_ranks(range, &i, rangeHigh);
                    break;
                case 0:
                case ' ':
                case ':':
                    memcpy(rangeHigh, rangeLow, sizeof(rangeLow));
                    break;
                default:
                    handrange_destroy(handRange);
                    return 0;
                }
                break;
            case 'o':
                offsuit = true;

                switch (range[i++]) {
                case '+':
                    break;
                case '-':
                    if (range[i] == 0 || range[i] == ' ' || range[i] == ':') {
                        SWAP_RANKS(rangeLow, rangeHigh);
                        break;
                    }

                    if (len - i < 2) {
                        handrange_destroy(handRange);
                        return 0;
                    }

                    get_two_ranks(range, &i, rangeHigh);

                    pocketPairs = rangeLow[0] == rangeLow[1] &&
                                  rangeHigh[0] == rangeHigh[1];
                    break;
                case 0:
                case ' ':
                case ':':
                    memcpy(rangeHigh, rangeLow, sizeof(rangeLow));
                    break;
                default:
                    handrange_destroy(handRange);
                    return 0;
                }
                break;
            case 0:
            case ' ':
            case ':':
                memcpy(rangeHigh, rangeLow, sizeof(rangeLow));
                break;
            default:
                handrange_destroy(handRange);
                return 0;
            }
        } else {
            memcpy(rangeHigh, rangeLow, sizeof(rangeLow));
        }

        static const char *numChars = "0123456789.";
        float frequency = 100.0f;

        if (range[i] == ':')
            i++;

        if (range[i - 1] == ':' && strchr(numChars, range[i]) != NULL) {
            char *endPtr;
            frequency = strtof(&range[i], &endPtr);
            if (endPtr == &range[i] || frequency < 0.0f || frequency > 100.0f) {
                handrange_destroy(handRange);
                return 0;
            }

            i += (endPtr - &range[i]) / sizeof(char);
        }

        if (rangeLow[0] != -1 && rangeLow[1] != -1 && rangeHigh[0] != -1 &&
            rangeHigh[1] != -1) {
            if (rangeLow[0] > rangeHigh[0] ||
                (rangeLow[0] == rangeHigh[0] && rangeLow[1] > rangeHigh[1]))
                SWAP_RANKS(rangeLow, rangeHigh);
        }

        card_t cards[2] = {0};
        for (int i = 0; i < 1326 * 2; ++i) {
            card_two_from_idx(i % 1326, cards);
            if (i >= 1326)
                SWAP(cards[0], cards[1]);

            if (card_filter(cards, suited, offsuit, pocketPairs, rangeLow,
                            rangeHigh)) {
                if (CARD_RANK(cards[1]) > CARD_RANK(cards[0]) ||
                    (CARD_RANK(cards[1]) == CARD_RANK(cards[0]) &&
                     CARD_SUIT(cards[1]) > CARD_SUIT(cards[0])))
                    SWAP(cards[0], cards[1]);

                push_hand(handRange, cards, frequency / 100.0f, &allocated);
            }
        }
    }

    if (handRange->size == 0)
        return handRange;

    qsort(handRange->hands, handRange->size, sizeof(handentry_t),
          compare_2_hands);

    size_t j = 0;
    for (size_t i = 1; i < handRange->size; ++i) {
        const handentry_t *hand1 = handrange_get(handRange, i);
        const handentry_t *hand2 = handrange_get(handRange, j);
        if (hand1->cards[0] == hand2->cards[0] &&
            hand1->cards[1] == hand2->cards[1])
            continue;

        memcpy(&handRange->hands[++j], &handRange->hands[i],
               sizeof(handentry_t));
    }
    handRange->size = j + 1;
    handRange->hands =
        realloc(handRange->hands, sizeof(handentry_t) * handRange->size);

    return handRange;
}

const handentry_t *handrange_get(handrange_t *handRange, size_t idx) {
    if (idx < handRange->size) {
        return &handRange->hands[idx];
    }
    return 0;
}

void handrange_destroy(handrange_t *handRange) {
    if (handRange->hands)
        free(handRange->hands);
    free(handRange);
}
