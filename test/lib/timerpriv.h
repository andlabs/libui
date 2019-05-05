// 5 may 2019

typedef struct timerprivInt128 timerprivInt128;

struct timerprivInt128 {
	int neg;
	uint64_t high;
	uint64_t low;
};

extern void timerprivMulDiv64(int64_t x, int64_t y, int64_t z, timerprivInt128 *quot);
extern void timerprivMulDivU64(uint64_t x, uint64_t y, uint64_t z, timerprivInt128 *quot);
