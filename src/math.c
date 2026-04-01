/*
 * math.c — Arithmetic Engine Implementation
 *
 * All integer-based. No <math.h> dependency.
 */

#include "../include/math.h"

/* ── Integer Arithmetic ───────────────────────────────────────────────── */

int m_abs(int x)
{
    return (x < 0) ? -x : x;
}

int m_min(int a, int b)
{
    return (a < b) ? a : b;
}

int m_max(int a, int b)
{
    return (a > b) ? a : b;
}

int m_clamp(int val, int lo, int hi)
{
    return m_max(lo, m_min(val, hi));
}

int m_mod(int a, int b)
{
    if (b == 0) return 0;
    int result = a % b;
    /* Ensure non-negative result for positive b */
    if (result < 0 && b > 0) result += b;
    return result;
}

int m_div(int a, int b)
{
    if (b == 0) return 0;
    return a / b;
}

int m_mul(int a, int b)
{
    return a * b;
}

/* ── Spatial / Boundary Helpers ───────────────────────────────────────── */

int m_aabb_intersect(int ax1, int ay1, int ax2, int ay2,
                     int bx1, int by1, int bx2, int by2)
{
    return (ax1 < bx2) && (ax2 > bx1) && (ay1 < by2) && (ay2 > by1);
}

int m_point_in_rect(int px, int py, int rx, int ry, int rw, int rh)
{
    return (px >= rx) && (px < rx + rw) && (py >= ry) && (py < ry + rh);
}

int m_distance(int x1, int y1, int x2, int y2)
{
    return m_abs(x1 - x2) + m_abs(y1 - y2);
}

/* ── Pseudo-Random Number Generator (LCG) ─────────────────────────────── */

static unsigned int rng_state = 1;

void m_srand(unsigned int seed)
{
    rng_state = seed;
}

int m_rand(void)
{
    rng_state = rng_state * 1103515245u + 12345u;
    return (int)((rng_state >> 16) & 0x7FFF);
}

int m_rand_range(int min, int max)
{
    if (min >= max) return min;
    return min + m_mod(m_rand(), (max - min + 1));
}
