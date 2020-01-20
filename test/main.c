// 19 january 2020
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s [-list|TestName]\n", argv[0]);
		return 1;
	}
	if (strcmp(argv[1], "-list") == 0) {
		// TODO
		return 0;
	}
	// TODO
	return 0;
}
