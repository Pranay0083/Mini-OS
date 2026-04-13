#ifndef MATH_H
#define MATH_H

/*
 * math.h — Arithmetic Engine Interface
 *
 * Implements integer math utilities and helpers.
 * No <math.h> dependency.
 */

/* ── Basic Arithmetic ───────────────────────────────────── */

int  m_add(int a, int b);
int  m_sub(int a, int b);
int  m_mul(int a, int b);
int  m_div(int a, int b);
int  m_mod(int a, int b);
int  m_abs(int x);
int  m_min(int a, int b);
int  m_max(int a, int b);
int  m_clamp(int val, int lo, int hi);

/* ── Spatial Helpers ───────────────────────────────────── */

int  m_aabb_intersect(int ax1, int ay1, int ax2, int ay2,
                      int bx1, int by1, int bx2, int by2);

int  m_point_in_rect(int px, int py, int rx, int ry, int rw, int rh);

int  m_distance(int x1, int y1, int x2, int y2);

/* ── Random Number Generator ───────────────────────────── */

void m_srand(unsigned int seed);
int  m_rand(void);
int  m_rand_range(int min, int max);

#endif /* MATH_H */
