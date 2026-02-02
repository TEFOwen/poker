#define _POSIX_C_SOURCE 199309L
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "card.h"
#include "equity.h"
#include "evaluator.h"
#include "handrange.h"
#include "tests.h"

#define test_assert(expr)                                                      \
    if (!(expr)) {                                                             \
        printf("Test failed, file %s:%d\n", __FILE_NAME__, __LINE__);          \
        return 1;                                                              \
    }

int test_flush(evaluator_t *evaluator) {
    test_assert(
        evaluator_evaluate(
            evaluator,
            (card_t[]){create_card(Ace, Spades), create_card(Two, Spades),
                       create_card(Three, Spades), create_card(Four, Spades),
                       create_card(Five, Spades), create_card(Eight, Hearts),
                       create_card(Nine, Diamonds)},
            7) == 36865);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Spades),
                                              create_card(King, Spades),
                                              create_card(Ten, Spades),
                                              create_card(Queen, Spades),
                                              create_card(Jack, Spades)},
                                   5) == 36874);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Spades),
                                              create_card(King, Spades),
                                              create_card(Nine, Spades),
                                              create_card(Queen, Spades),
                                              create_card(Jack, Spades)},
                                   5) == 25853);

    return 0;
}

int test_unique(evaluator_t *evaluator) {
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ten, Hearts),
                                              create_card(King, Spades),
                                              create_card(Nine, Clubs),
                                              create_card(Queen, Spades),
                                              create_card(Jack, Diamonds)},
                                   5) == 20489);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Two, Hearts),
                                              create_card(Three, Spades),
                                              create_card(Seven, Clubs),
                                              create_card(Four, Spades),
                                              create_card(Five, Diamonds)},
                                   5) == 4097);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Two, Hearts),
                                              create_card(Three, Diamonds),
                                              create_card(Five, Clubs),
                                              create_card(Four, Clubs),
                                              create_card(Eight, Spades)},
                                   5) == 4101);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Hearts),
                                              create_card(Nine, Diamonds),
                                              create_card(King, Clubs),
                                              create_card(Queen, Clubs),
                                              create_card(Jack, Spades)},
                                   5) == 5373);

    return 0;
}

int test_primes(evaluator_t *evaluator) {
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Diamonds),
                                              create_card(Ace, Hearts),
                                              create_card(Ace, Clubs),
                                              create_card(Ace, Spades),
                                              create_card(King, Hearts)},
                                   5) == 32924);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Two, Diamonds),
                                              create_card(Five, Hearts),
                                              create_card(Two, Clubs),
                                              create_card(Four, Spades),
                                              create_card(Three, Hearts)},
                                   5) == 8193);

    return 0;
}

int test_all_5card(evaluator_t *evaluator) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint32_t count = 0, invalid = 0;
    for (uint32_t n = 0; n < 50; ++n)
        for (uint32_t i = 0; i < 48; ++i)
            for (uint32_t j = i + 1; j < 49; ++j)
                for (uint32_t k = j + 1; k < 50; ++k)
                    for (uint32_t l = k + 1; l < 51; ++l)
                        for (uint32_t m = l + 1; m < 52; ++m)
                            if (evaluator_evaluate(evaluator,
                                                   (card_t[]){card_from_idx(i),
                                                              card_from_idx(j),
                                                              card_from_idx(k),
                                                              card_from_idx(l),
                                                              card_from_idx(m)},
                                                   5))
                                count += 1;
                            else
                                invalid += 1;

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    test_assert(invalid == 0);
    printf("Evaluated %d 5 card hands in %.2fs (%.2f hands per second)\n",
           count, time, (double)count / time);

    return 0;
}

int test_all_7card(evaluator_t *evaluator) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint64_t count = 0, invalid = 0;
    for (uint32_t a = 0; a < 46; ++a)
        for (uint32_t b = a + 1; b < 47; ++b)
            for (uint32_t c = b + 1; c < 48; ++c)
                for (uint32_t d = c + 1; d < 49; ++d)
                    for (uint32_t e = d + 1; e < 50; ++e)
                        for (uint32_t f = e + 1; f < 51; ++f)
                            for (uint32_t g = f + 1; g < 52; ++g)
                                if (evaluator_evaluate(
                                        evaluator,
                                        (card_t[]){
                                            card_from_idx(a), card_from_idx(b),
                                            card_from_idx(c), card_from_idx(d),
                                            card_from_idx(e), card_from_idx(f),
                                            card_from_idx(g)},
                                        7))
                                    count += 1;
                                else
                                    invalid += 1;

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    test_assert(invalid == 0);
    printf("Evaluated %lu 7 card hands in %.2lfs (%.2lf hands per second)\n",
           count, time, (double)count / time);

    return 0;
}

#define RANDOM(min, max) (rand() % ((max) - (min)) + (min))

int test_7card_rand(evaluator_t *evaluator) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint64_t count = 0, invalid = 0;
    for (; count < 10000000; ++count) {
        uint32_t a = RANDOM(0, 46), b = RANDOM(a + 1, 47),
                 c = RANDOM(b + 1, 48), d = RANDOM(c + 1, 49),
                 e = RANDOM(d + 1, 50), f = RANDOM(e + 1, 51),
                 g = RANDOM(f + 1, 52);
        if (evaluator_evaluate(evaluator,
                               (card_t[]){card_from_idx(a), card_from_idx(b),
                                          card_from_idx(c), card_from_idx(d),
                                          card_from_idx(e), card_from_idx(f),
                                          card_from_idx(g)},
                               7))
            count += 1;
        else
            invalid += 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    test_assert(invalid == 0);
    printf("Evaluated %lu 7 card hands in %.2lfs (%.2lf hands per second)\n",
           count, time, (double)count / time);

    return 0;
}

int test_equity(evaluator_t *evaluator) {
    card_t cards[4] = {create_card(Ace, Spades), create_card(Ace, Clubs),
                       create_card(King, Spades), create_card(King, Clubs)};
    card_t community[4] = {create_card(Queen, Hearts), create_card(Two, Spades),
                           create_card(Ace, Diamonds),
                           create_card(King, Hearts)};
    printf("Hand 1: As Ac\nHand 2: Ks Kc\n");
    {
        equityinfo_t *equity = equity_calc(evaluator, cards, 2, NULL, 0);

        printf("Evaluated %lu 7 card hands in %.2fs\n", equity->total,
               equity->time);
        for (int i = 0; i < 2; ++i) {
            printf("Hand %d: win %.2f%%, chop %.2f%%\n", i + 1,
                   equity->equities[i].win * 100,
                   equity->equities[i].chop * 100);
        }

        test_assert(equity->equities[0].win < 0.83 &&
                    equity->equities[0].win > 0.82);

        equity_destroy(equity);
    }
    printf("Flop: Qh 2s Ad\n");
    {
        equityinfo_t *equity = equity_calc(evaluator, cards, 2, community, 3);

        printf("Evaluated %lu 7 card hands in %.2fs\n", equity->total,
               equity->time);
        for (int i = 0; i < 2; ++i) {
            printf("Hand %d: win %.2f%%, chop %.2f%%\n", i + 1,
                   equity->equities[i].win * 100,
                   equity->equities[i].chop * 100);
        }

        test_assert(equity->equities[0].win > 0.98);

        equity_destroy(equity);
    }
    printf("Turn: Kh\n");
    {
        equityinfo_t *equity = equity_calc(evaluator, cards, 2, community, 4);

        printf("Evaluated %lu 7 card hands in %.2fs\n", equity->total,
               equity->time);
        for (int i = 0; i < 2; ++i) {
            printf("Hand %d: win %.2f%%, chop %.2f%%\n", i + 1,
                   equity->equities[i].win * 100,
                   equity->equities[i].chop * 100);
        }

        test_assert(equity->equities[1].win > 0.022 &&
                    equity->equities[1].win < 0.023);

        equity_destroy(equity);
    }

    return 0;
}

#define TEST_HANDRANGE_LENGTH(str, len)                                        \
    {                                                                          \
        handrange_t *handRange = handrange_create(str);                        \
        test_assert(handRange->size == (len));                                 \
        handrange_destroy(handRange);                                          \
    }

int test_handrange(evaluator_t *) {
    TEST_HANDRANGE_LENGTH("AA 55 77 27", 6 * 3 + 16);
    TEST_HANDRANGE_LENGTH("AA-22", 6 * 13);
    TEST_HANDRANGE_LENGTH("AA", 6);
    TEST_HANDRANGE_LENGTH("AK", 16);
    TEST_HANDRANGE_LENGTH("KK+", 6 * 2 + 12 * 16);
    TEST_HANDRANGE_LENGTH("38-", 6 * 2 + 16 * 11);
    TEST_HANDRANGE_LENGTH("AKs", 4);
    TEST_HANDRANGE_LENGTH("AQs+", 4 * 2);
    TEST_HANDRANGE_LENGTH("AKo", 12);
    TEST_HANDRANGE_LENGTH("AQo+", 12 * 2 + 6);

    return 0;
}

int test_handrange_freq(evaluator_t *) {
    TEST_HANDRANGE_LENGTH("AA:50", 6);
    TEST_HANDRANGE_LENGTH("AK:25", 16);
    TEST_HANDRANGE_LENGTH("KK+:13", 6 * 2 + 12 * 16);
    TEST_HANDRANGE_LENGTH("38-:57.5", 6 * 2 + 16 * 11);
    TEST_HANDRANGE_LENGTH("AKs:98.12", 4);
    TEST_HANDRANGE_LENGTH("AQs+:21", 4 * 2);
    TEST_HANDRANGE_LENGTH("AKo:.2 77", 12 + 6);

    return 0;
}

const struct Test {
    const char *name;
    int (*func)(evaluator_t *);
} tests[] = {{"flush", test_flush},
             {"unique", test_unique},
             {"primes", test_primes},
             {"handrange", test_handrange},
             {"all_5card", test_all_5card},
             {"all_7card", test_all_7card},
             {"rand_7card", test_7card_rand},
             {"equity", test_equity},
             {"handrange_freq", test_handrange_freq}};
const size_t nTests = sizeof(tests) / sizeof(struct Test);

void print_tests(const char *fmt) {
    printf(fmt, "all");
    for (int i = 0; i < nTests; ++i)
        printf(fmt, tests[i].name);
}

int run_tests(int argc, char *argv[]) {
    evaluator_t *evaluator = evaluator_load("handranks.dat");
    if (!evaluator) {
        printf("Failed to load evaluator\n");
        return 0;
    }

    int failed = 0, succeeded = 0;
    for (int i = 0; i < nTests; ++i) {
        if (strcmp(argv[2], tests[i].name) == 0 ||
            strcmp(argv[2], "all") == 0) {
            printf("Running test %s...\n", tests[i].name);
            if (tests[i].func(evaluator)) {
                printf("Test %s failed\n", tests[i].name);
                failed++;
            } else {
                printf("Test %s succeeded\n", tests[i].name);
                succeeded++;
            }
        }
    }

    if (failed == 0 && succeeded == 0)
        return 1;

    printf("\n%d succeeded, %d failed\n", succeeded, failed);

    evaluator_destroy(evaluator);

    return 0;
}
