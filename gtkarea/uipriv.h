typedef struct clickCounter clickCounter;
// you should call Reset() to zero-initialize a new instance
// it doesn't matter that all the non-count fields are zero: the first click will fail the curButton test straightaway, so it'll return 1 and set the rest of the structure accordingly
struct clickCounter {
	uintmax_t curButton;
	intmax_t rectX0;
	intmax_t rectY0;
	intmax_t rectX1;
	intmax_t rectY1;
	uintptr_t prevTime;
	uintmax_t count;
};
extern uintmax_t clickCounterClick(clickCounter *, uintmax_t, intmax_t, intmax_t, uintptr_t, uintptr_t, intmax_t, intmax_t);
extern void clickCounterReset(clickCounter *);
