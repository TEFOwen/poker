#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "card.h"
#include "equity.h"
#include "evaluator.h"
#include "handrange.h"
#include "tests.h"

char *format_time(double time) {
    char *str = calloc(11, sizeof(char));
    const char *prefix[] = {"s", "ms", "us", "ns", "ps"};

    int i = 0;
    for (; i < 5; ++i) {
        if (time < 1.0f)
            time *= 1000.0f;
        else
            break;
    }

    sprintf(str, "%.2f%s", time, prefix[i]);

    return str;
}

void calc_and_print_equity(evaluator_t *evaluator, card_t *hands, size_t nHands,
                           card_t *community, size_t nCommunity) {
    equityinfo_t *equity =
        equity_calc(evaluator, hands, nHands, community, nCommunity);

    char *timeStr = format_time(equity->time);
    printf("Analysed %lu possibilities in %s\n", equity->total, timeStr);
    free(timeStr);

    handequity_t *equities = equity->equities;
    for (int i = 0; i < nHands; ++i) {
        printf("\tPlayer %d: Equity: %.2lf%%, Win: %.2f%%", i + 1,
               equities[i].equity * 100.0, equities[i].win * 100.0f);
        if (equities[i].chop != 0.0f)
            printf(", Chop: %.2f%%", equities[i].chop * 100.0f);

        uint32_t outs = equities[i].winOuts + equities[i].chopOuts;
        if (outs <= 10 && outs != 0) {
            printf(", Outs: %u", outs);
            if (equities[i].winOuts != 0 && equities[i].chopOuts != 0)
                printf(" (%lu win / %lu chop)", equities[i].winOuts,
                       equities[i].chopOuts);
        }
        printf("\n");
    }

    equity_destroy(equity);
}

int equity() {
    evaluator_t *evaluator = evaluator_load("handranks.dat");

    int nHands = -1;
    char nHandsStr[10] = {0};
    while (nHands < 0) {
        printf("Enter number of hands: ");
        if (fgets(nHandsStr, 9, stdin) == NULL) {
            printf("Failed to read stdin\n");
            evaluator_destroy(evaluator);
            return 1;
        }
        if (sscanf(nHandsStr, "%d", &nHands) != 1)
            printf("Failed to parse integer\n");
        if (nHands <= 1 || nHands >= 22) {
            printf("Number of hands must be between 1 and 22\n");
            nHands = -1;
        }
    }

    card_t *hands = malloc(sizeof(card_t) * 2 * nHands);
    for (int i = 0; i < nHands; ++i) {
        for (;;) {
            printf("Enter hand for player %d (2 cards): ", i + 1);
            char cardStr[10] = {0};
            if (fgets(cardStr, 9, stdin) == NULL) {
                printf("Failed to read stdin\n");
                free(hands);
                evaluator_destroy(evaluator);
                return 1;
            }

            card_t card1, card2;
            if (!(card1 = card_from_str(cardStr)) ||
                !(card2 = card_from_str(cardStr + 3)))
                printf("Failed to parse cards\n");
            else {
                bool duplicate = false;
                for (int j = 0; j < i * 2; ++j)
                    if (hands[j] == card1 || hands[j] == card2) {
                        printf("One or more cards already in use\n");
                        duplicate = true;
                        break;
                    }

                if (!duplicate) {
                    hands[i * 2] = card1;
                    hands[i * 2 + 1] = card2;
                    break;
                }
            }
        }
    }

    calc_and_print_equity(evaluator, hands, nHands, NULL, 0);

    card_t community[7] = {0}; // Use community for final eval, so size 7
    for (;;) {
        printf("Enter flop: ");
        char flopStr[15] = {0};
        if (fgets(flopStr, 14, stdin) == NULL) {
            printf("Failed to read stdin\n");
            free(hands);
            evaluator_destroy(evaluator);
            return 1;
        }

        bool failed = false;
        for (int i = 0; i < 3; ++i) {
            community[i] = card_from_str(flopStr + i * 3);
            if (community[i] == 0) {
                printf("Failed to parse cards\n");
                failed = true;
                break;
            }

            for (int j = 0; j < nHands * 2; ++j)
                if (hands[j] == community[i]) {
                    printf("One or more cards already in use\n");
                    failed = true;
                    break;
                }
            if (!failed)
                for (int j = 0; j < i; ++j)
                    if (community[j] == community[i]) {
                        printf("One or more cards already in use\n");
                        failed = true;
                        break;
                    }
        }

        if (!failed)
            break;

        memset(community, 0, sizeof(community));
    }

    calc_and_print_equity(evaluator, hands, nHands, community, 3);

    for (;;) {
        printf("Enter turn: ");
        char turnStr[10] = {0};
        if (fgets(turnStr, 9, stdin) == NULL) {
            printf("Failed to read stdin\n");
            free(hands);
            evaluator_destroy(evaluator);
            return 1;
        }

        community[3] = card_from_str(turnStr);
        if (community[3] == 0) {
            printf("Failed to parse card\n");
            continue;
        }

        bool duplicate = false;
        for (int i = 0; i < nHands * 2; ++i)
            if (hands[i] == community[3]) {
                printf("Card already in use\n");
                duplicate = true;
                break;
            }
        if (!duplicate)
            for (int i = 0; i < 3; ++i)
                if (community[i] == community[3]) {
                    printf("Card already in use\n");
                    duplicate = true;
                    break;
                }

        if (!duplicate)
            break;

        community[3] = 0;
    }

    calc_and_print_equity(evaluator, hands, nHands, community, 4);

    for (;;) {
        printf("Enter river: ");
        char riverStr[10] = {0};
        if (fgets(riverStr, 9, stdin) == NULL) {
            printf("Failed to read stdin\n");
            free(hands);
            evaluator_destroy(evaluator);
            return 1;
        }

        community[4] = card_from_str(riverStr);
        if (community[4] == 0) {
            printf("Failed to parse card\n");
            continue;
        }

        bool duplicate = false;
        for (int i = 0; i < nHands * 2; ++i)
            if (hands[i] == community[4]) {
                printf("Card already in use\n");
                duplicate = true;
                break;
            }
        if (!duplicate)
            for (int i = 0; i < 4; ++i)
                if (community[i] == community[4]) {
                    printf("Card already in use\n");
                    duplicate = true;
                    break;
                }

        if (!duplicate)
            break;

        community[4] = 0;
    }

    printf("Hands:\n");
    handrank_t *ranks = malloc(sizeof(handrank_t) * nHands), bestRank = 0;
    int winners = 1;
    for (int i = 0; i < nHands; ++i) {
        memcpy(community + 5, hands + (i * 2), sizeof(card_t) * 2);
        ranks[i] = evaluator_evaluate(evaluator, community, 7);

        char handStr[6] = {0};
        card_to_string(hands[i * 2], handStr);
        card_to_string(hands[i * 2 + 1], handStr + 3);
        handStr[2] = ' ';
        char *handRankStr = handrank_to_str(ranks[i]);
        printf("\tPlayer %d: %s (%s %X)\n", i + 1, handStr, handRankStr,
               ranks[i]);
        free(handRankStr);
        if (ranks[i] > bestRank) {
            bestRank = ranks[i];
            winners = 1;
        } else if (ranks[i] == bestRank)
            winners++;
    }

    printf("Board:\n\t");
    for (int i = 0; i < 5; ++i) {
        char cardStr[3] = {0};
        card_to_string(community[i], cardStr);
        printf("%s ", cardStr);
    }
    printf("\n");

    char *rankStr = handrank_to_str(bestRank);
    if (winners > 1) {
        printf("Chop (%s) between:\n", rankStr);
    }

    for (int i = 0; i < nHands; ++i) {
        if (ranks[i] == bestRank) {
            if (winners > 1)
                printf("\tPlayer %d\n", i + 1);
            else
                printf("Player %d wins with %s\n", i + 1, rankStr);
        }
    }

    free(ranks);
    free(hands);
    free(rankStr);
    evaluator_destroy(evaluator);

    return 0;
}

int range_equity() {
    evaluator_t *evaluator = evaluator_load("handranks.dat");
    if (!evaluator)
        return 1;

    printf("Enter range 1: ");
    char range[30];
    if (fgets(range, 30, stdin) == NULL) {
        printf("Failed to read stdin\n");
        evaluator_destroy(evaluator);
        return 1;
    }

    handrange_t *handRange1 = handrange_create(range);
    if (!handRange1 || handRange1->size == 0) {
        printf("Invalid range\n");
        evaluator_destroy(evaluator);
        return 1;
    }

    printf("Enter range 2: ");
    memset(range, 0, sizeof(range));
    if (fgets(range, 30, stdin) == NULL) {
        printf("Failed to read stdin\n");
        evaluator_destroy(evaluator);
        return 1;
    }
    handrange_t *handRange2 = handrange_create(range);
    if (!handRange2 || handRange2->size == 0) {
        printf("Invalid range\n");
        handrange_destroy(handRange1);
        evaluator_destroy(evaluator);
        return 1;
    }

    card_t hands[4] = {0};
    equityinfo_t equity = {0};
    equity.equities = calloc(2, sizeof(handequity_t));

    for (int a = 0; a < handRange1->size; ++a) {
        memcpy(hands, handrange_get(handRange1, a)->cards, sizeof(card_t) * 2);
        for (int b = 0; b < handRange2->size; ++b) {
            memcpy(hands + 2, handrange_get(handRange2, b)->cards,
                   sizeof(card_t) * 2);

            bool duplicate = false;
            for (int i = 0; i < 4 - 1; ++i)
                for (int j = i + 1; j < 4; ++j)
                    if (hands[i] == hands[j]) {
                        duplicate = true;
                        break;
                    }

            if (duplicate)
                continue;

            equityinfo_t *tempEquity =
                equity_calc(evaluator, hands, 2, NULL, 0);

            equity.total += tempEquity->total;
            equity.time += tempEquity->time;

            for (int i = 0; i < 2; ++i) {
                equity.equities[i].winOuts += tempEquity->equities[i].winOuts;
                equity.equities[i].chopOuts += tempEquity->equities[i].chopOuts;
            }

            equity_destroy(tempEquity);
        }

        printf(
            "\r%.2lf%% %.2lf%%",
            (double)equity.equities[0].winOuts / (double)equity.total * 100.0f,
            (double)equity.equities[1].winOuts / (double)equity.total * 100.0f);
        fflush(stdout);
    }

    char *timeStr = format_time(equity.time);
    printf("\rTime: %s, total: %lu\n", timeStr, equity.total);
    free(timeStr);
    for (int i = 0; i < 2; ++i) {
        equity.equities[i].win =
            (double)equity.equities[i].winOuts / (double)equity.total;
        equity.equities[i].chop =
            (double)equity.equities[i].chopOuts / (double)equity.total;
        printf("Hand %d: Win: %.2f, Chop: %.2f\n", i + 1,
               equity.equities[i].win * 100.0f,
               equity.equities[i].chop * 100.0f);
    }

    free(equity.equities);
    evaluator_destroy(evaluator);
    handrange_destroy(handRange1);
    handrange_destroy(handRange2);

    return 0;
}

int main(int argc, char *argv[]) {
    bool showUsage = true;
    if (argc == 3 && strcmp(argv[1], "test") == 0)
        showUsage = run_tests(argc, argv) != 0;

    if (argc == 3 && strcmp(argv[1], "range") == 0) {
        showUsage = false;
        handrange_t *handRange = handrange_create(argv[2]);
        if (!handRange) {
            printf("Invalid hand range\n");
            return 1;
        }

        for (int i = 0; i < handRange->size; ++i) {
            const handentry_t *hand = handrange_get(handRange, i);
            char card1[3], card2[3];
            card_to_string(hand->cards[0], card1);
            card_to_string(hand->cards[1], card2);
            if (hand->frequency == 1.0f)
                printf("%s %s\n", card1, card2);
            else
                printf("%s %s : %.2f%%\n", card1, card2,
                       hand->frequency * 100.0f);
        }

        handrange_destroy(handRange);
    }

    if (argc == 2 && strcmp(argv[1], "equity-range") == 0)
        return range_equity();

    if (argc == 2 && strcmp(argv[1], "equity") == 0) {
        showUsage = false;
        return equity();
    }

    if (showUsage) {
        printf("Usage:\n\t%s <command> [<args>]\n\n"
               "Available commands:\n"
               "  test <testName>\tRun a specific test, available tests are:\n",
               argv[0]);
        print_tests("\t\t\t\t%s\n");
        printf("  range <handRange>\tCalculate hands within a range, e.g. "
               "AA-22, AKs+, 27o-\n  equity\t\tRun equity calculations\n");
    }

    return 0;
}
