/*
macros2.h - Useful macros for simulation

These macros are based on the ones presented in the FAQ for the
usenet group comp.lang.c and are considered to be public domain.
*/

#define E_BITMASK(b) (1 << ((b) % 8))
#define E_BITSLOT(b) ((b) / 8)
#define E_BITSET(a, b) ((a)[E_BITSLOT(b)] |= E_BITMASK(b))
#define E_BITCLEAR(a, b) ((a)[E_BITSLOT(b)] &= ~E_BITMASK(b))
#define E_BITTEST(a, b) ((a)[E_BITSLOT(b)] & E_BITMASK(b))

#define E_BITSET2(a, b) do { E_BITSET(a,b);   E_BITSET(a,b+1); } while(0)

#define E_BITSET4(a, b) do { E_BITSET(a,b);   E_BITSET(a,b+1); \
			     E_BITSET(a,b+2); E_BITSET(a,b+3); } while(0)

#define E_BITCLEAR2(a, b) do { E_BITCLEAR(a,b);   E_BITCLEAR(a,b+1); } while(0)

#define E_BITCLEAR4(a, b) do { E_BITCLEAR(a,b);   E_BITCLEAR(a,b+1); \
			       E_BITCLEAR(a,b+2); E_BITCLEAR(a,b+3); } while(0)
