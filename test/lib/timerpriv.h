// 5 may 2019

typedef struct timerprivInt128 timerprivInt128;

struct timerprivInt128 {
	int neg;
	uint64_t high;
	uint64_t low;
};

extern void timerprivMulDivInt64(int64_t x, int64_t y, int64_t z, timerprivInt128 *quot);
extern void timerprivMulDivUint64(uint64_t x, uint64_t y, uint64_t z, timerprivInt128 *quot);
extern int64_t timerprivInt128ToInt64(const timerprivInt128 *n, int64_t min, int64_t minCap, int64_t max, int64_t maxCap);
extern uint64_t timerprivInt128ToUint64(const timerprivInt128 *n, uint64_t max, uint64_t maxCap);
