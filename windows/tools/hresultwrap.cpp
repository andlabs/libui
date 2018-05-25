// 21 may 2018
#include <vector>
#include <stdio.h>
#include <string.h>
#include "scanner.hpp"

bool generate(const char *line, size_t n, FILE *fout)
{
	std::vector<char> genout;
	size_t nw;

	genout.push_back('/');
	genout.push_back('/');
	genout.push_back(' ');
	genout.insert(genout.end(), line, line + n);
	genout.push_back('\n');

	genout.push_back('\n');
	nw = fwrite(genout.data(), sizeof (char), genout.size(), fout);
	return nw == genout.size();
}

bool process(const char *line, size_t n, FILE *fout)
{
	size_t nw;

	if (n > 0 && line[0] == '@')
		return generate(line + 1, n - 1, fout);
	nw = fwrite(line, sizeof (char), n, fout);
	if (nw != n)
		return false;
	return fwrite("\n", sizeof (char), 1, fout) == 1;
}

int main(int argc, char *argv[])
{
	int fin = -1;
	FILE *fout = NULL;
	Scanner *s = NULL;
	int ret = 1;
	int err;

	if (argc != 3) {
		fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
		return 1;
	}

	err = OpenForScanner(argv[1], &fin);
	if (err != 0) {
		fprintf(stderr, "error opening %s: %s\n", argv[1], strerror(err));
		goto done;
	}
	fout = fopen(argv[2], "wb");
	if (fout == NULL) {
		fprintf(stderr, "error creating %s\n", argv[2]);
		goto done;
	}

	s = new Scanner(fin);
	while (s->Scan()) {
		const char *line;
		size_t n;

		line = s->Bytes();
		n = s->Len();
		if (!process(line, n, fout)) {
			fprintf(stderr, "error writing to %s\n", argv[2]);
			goto done;
		}
	}
	if (s->Error() != 0) {
		fprintf(stderr, "error reading from %s: %s\n", argv[1], strerror(s->Error()));
		goto done;
	}

	ret = 0;
done:
	if (s != NULL)
		delete s;
	if (fout != NULL)
		fclose(fout);
	if (fin >= 0)
		CloseForScanner(fin);
	return ret;
}
