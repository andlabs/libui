// 2 may 2019
#include <string.h>
#include "timer.h"

// This is based on the algorithm that Go uses for time.Duration.
// Of course, we're not expressing it the same way...
struct timerStringPart {
	char suffix;
	char suffix2;
	int mode;
	uint32_t maxOrMod;
	int precision;
};

enum {
	modeMaxAndStop,
	modeFracModContinue,
};

static const struct timerStringPart parts[] = {
	{ 'n', 's', modeMaxAndStop, 1000, 0 },
	{ 'u', 's', modeMaxAndStop, 1000000, 3 },
	{ 'm', 's', modeMaxAndStop, 1000000000, 6 },
	{ 's', 0, modeFracModContinue, 60, 9 },
	{ 'm', 0, modeFracModContinue, 60, 0 },
	{ 'h', 0, modeFracModContinue, 60, 0 },
	{ 0, 0, 0, 0, 0 },
};

static int fillFracPart(char *buf, int precision, int start, uint64_t *unsec)
{
	int i;
	int print;
	uint64_t digit;

	print = 0;
	for (i = 0; i < precision; i++) {
		digit = *unsec % 10;
		print = print || (digit != 0);
		if (print) {
			buf[start - 1] = "0123456789"[digit];
			start--;
		}
		*unsec /= 10;
	}
	if (print) {
		buf[start - 1] = '.';
		start--;
	}
	return start;
}

static int fillIntPart(char *buf, int start, uint64_t unsec)
{
	if (unsec == 0) {
		buf[start - 1] = '0';
		start--;
		return start;
	}
	while (unsec != 0) {
		buf[start - 1] = "0123456789"[unsec % 10];
		start--;
		unsec /= 10;
	}
	return start;
}

void timerDurationString(timerDuration d, char buf[timerDurationStringLen])
{
	uint64_t unsec;
	int neg;
	int start;
	const struct timerStringPart *p;

	memset(buf, 0, timerTimeStringLen * sizeof (char));
	start = 32;

	if (d == 0) {
		buf[0] = '0';
		buf[1] = 's';
		return;
	}
	unsec = (uint64_t) d;
	neg = 0;
	if (d < 0) {
#ifdef _MSC_VER
// TODO figure out a more explicit way to do this; until then, just go with what the standard says should happen, because it's what we want (TODO verify this)
#pragma warning(push)
#pragma warning(disable: 4146)
#endif
		unsec = -unsec;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
		neg = 1;
	}

	for (p = parts; p->suffix != 0; p++) {
		if (p->mode == modeMaxAndStop && unsec < p->maxOrMod) {
			if (p->suffix2 != 0) {
				buf[start - 1] = p->suffix2;
				start--;
			}
			buf[start - 1] = p->suffix;
			start--;
			start = fillFracPart(buf, p->precision, start, &unsec);
			start = fillIntPart(buf, start, unsec);
			break;
		}
		if (p->mode == modeFracModContinue && unsec != 0) {
			if (p->suffix2 != 0) {
				buf[start - 1] = p->suffix2;
				start--;
			}
			buf[start - 1] = p->suffix;
			start--;
			start = fillFracPart(buf, p->precision, start, &unsec);
			start = fillIntPart(buf, start, unsec % p->maxOrMod);
			unsec /= p->maxOrMod;
			// and move on to the next one
		}
	}

	if (neg) {
		buf[start - 1] = '-';
		start--;
	}
	memmove(buf, buf + start, 33 - start);
}
