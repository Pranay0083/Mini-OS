/*
 * test_math.c — Unit tests for the Arithmetic Engine
 */

#include "../include/math.h"
#include <stdio.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg)                                           \
    do {                                                            \
        if (cond) {                                                 \
            tests_passed++;                                         \
            printf("  [PASS] %s\n", msg);                          \
        } else {                                                    \
            tests_failed++;                                         \
            printf("  [FAIL] %s  (line %d)\n", msg, __LINE__);    \
        }                                                           \
    } while (0)

static void test_abs(void)
{
    printf("\n--- Absolute Value ---\n");
    ASSERT(m_abs(5)  == 5,  "abs(5) == 5");
    ASSERT(m_abs(-5) == 5,  "abs(-5) == 5");
    ASSERT(m_abs(0)  == 0,  "abs(0) == 0");
}

static void test_min_max(void)
{
    printf("\n--- Min / Max ---\n");
    ASSERT(m_min(3, 7) == 3, "min(3,7) == 3");
    ASSERT(m_min(7, 3) == 3, "min(7,3) == 3");
    ASSERT(m_max(3, 7) == 7, "max(3,7) == 7");
    ASSERT(m_max(-1, -5) == -1, "max(-1,-5) == -1");
}

static void test_clamp(void)
{
    printf("\n--- Clamp ---\n");
    ASSERT(m_clamp(5, 0, 10) == 5,  "clamp(5, 0..10) == 5");
    ASSERT(m_clamp(-3, 0, 10) == 0, "clamp(-3, 0..10) == 0");
    ASSERT(m_clamp(15, 0, 10) == 10, "clamp(15, 0..10) == 10");
}

static void test_mod_div(void)
{
    printf("\n--- Mod / Div ---\n");
    ASSERT(m_mod(10, 3) == 1,  "10 %% 3 == 1");
    ASSERT(m_mod(0, 5)  == 0,  "0 %% 5 == 0");
    ASSERT(m_mod(7, 0)  == 0,  "7 %% 0 == 0 (safe)");
    ASSERT(m_div(10, 3) == 3,  "10 / 3 == 3");
    ASSERT(m_div(5, 0)  == 0,  "5 / 0 == 0 (safe)");
}

static void test_aabb(void)
{
    printf("\n--- AABB Intersection ---\n");
    /* Overlapping boxes */
    ASSERT(m_aabb_intersect(0,0,10,10, 5,5,15,15) == 1,
           "Overlapping boxes intersect");
    /* Non-overlapping */
    ASSERT(m_aabb_intersect(0,0,5,5, 10,10,20,20) == 0,
           "Separated boxes don't intersect");
    /* Touching edge (not overlapping in strict AABB) */
    ASSERT(m_aabb_intersect(0,0,5,5, 5,0,10,5) == 0,
           "Edge-touching boxes don't intersect");
    /* Contained */
    ASSERT(m_aabb_intersect(0,0,20,20, 5,5,10,10) == 1,
           "Contained box intersects");
}

static void test_point_in_rect(void)
{
    printf("\n--- Point in Rect ---\n");
    ASSERT(m_point_in_rect(5, 5, 0, 0, 10, 10) == 1, "Point inside");
    ASSERT(m_point_in_rect(0, 0, 0, 0, 10, 10) == 1, "Point on corner");
    ASSERT(m_point_in_rect(15, 5, 0, 0, 10, 10) == 0, "Point outside");
}

static void test_distance(void)
{
    printf("\n--- Manhattan Distance ---\n");
    ASSERT(m_distance(0,0,3,4) == 7, "dist(0,0 → 3,4) == 7");
    ASSERT(m_distance(5,5,5,5) == 0, "dist(same point) == 0");
}

static void test_rand(void)
{
    printf("\n--- Random Number Generator ---\n");
    m_srand(42);
    int a = m_rand();
    int b = m_rand();
    ASSERT(a != b, "Sequential rands differ");
    ASSERT(a >= 0 && a <= 0x7FFF, "rand() in range [0, 32767]");

    /* Seeded determinism */
    m_srand(42);
    int c = m_rand();
    ASSERT(a == c, "Same seed produces same sequence");

    /* Range test */
    int in_range = 1;
    for (int i = 0; i < 100; i++) {
        int r = m_rand_range(5, 10);
        if (r < 5 || r > 10) { in_range = 0; break; }
    }
    ASSERT(in_range, "rand_range(5, 10) stays in [5, 10] over 100 samples");
}

int main(void)
{
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║        math.c — Unit Test Suite           ║\n");
    printf("╚═══════════════════════════════════════════╝\n");

    test_abs();
    test_min_max();
    test_clamp();
    test_mod_div();
    test_aabb();
    test_point_in_rect();
    test_distance();
    test_rand();

    printf("\n════════════════════════════════════════════\n");
    printf("Results: %d passed, %d failed, %d total\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    printf("════════════════════════════════════════════\n");

    return tests_failed > 0 ? 1 : 0;
}
