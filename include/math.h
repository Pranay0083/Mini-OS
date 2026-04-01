/*
 * math.h — Arithmetic Engine Interface
 *
 * Integer-based arithmetic, spatial boundary helpers, and PRNG.
 * No <math.h> dependency.
 */

#ifndef MATH_H
#define MATH_H

/* ── Integer Arithmetic ───────────────────────────────────────────────── */

int  m_abs(int x);
int  m_min(int a, int b);
int  m_max(int a, int b);
int  m_clamp(int val, int lo, int hi);
int  m_mod(int a, int b);
int  m_div(int a, int b);
int  m_mul(int a, int b);

/* ── Spatial / Boundary Helpers ───────────────────────────────────────── */

/**
 * AABB intersection test.
 * Returns 1 if box A (x1,y1)-(x2,y2) overlaps box B (x1,y1)-(x2,y2).
 */
int  m_aabb_intersect(int ax1, int ay1, int ax2, int ay2,
                      int bx1, int by1, int bx2, int by2);

/**
 * Point-in-rectangle test.
 * Returns 1 if point (px,py) is inside rect at (rx,ry) with size (rw,rh).
 */
int  m_point_in_rect(int px, int py, int rx, int ry, int rw, int rh);

/**
 * Manhattan distance between two points.
 */
int  m_distance(int x1, int y1, int x2, int y2);

/* ── Pseudo-Random Number Generator (LCG) ─────────────────────────────── */

void m_srand(unsigned int seed);
int  m_rand(void);
int  m_rand_range(int min, int max);

#endif /* MATH_H */
