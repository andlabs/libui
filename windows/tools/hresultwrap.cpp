// 21 may 2018
#include <vector>
#include <stdio.h>
#include "lib.hpp"

bool generate(const char *line, size_t n, FILE *fout)
{
	std::vector<char> genout;
	std::vector<Slice *> *tokens;
	std::vector<Slice *>::const_iterator i;
	size_t nw;

	tokens = TokenizeWhitespace(line, n);
	for (i = tokens->begin(); i < tokens->end(); i++) {
		genout.push_back('/');
		genout.push_back('/');
		genout.push_back(' ');
		AppendSlice(&genout, *i);
		genout.push_back('\n');
	}
	FreeTokenized(tokens);

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
	ReadCloser *fin = NULL;
	FILE *fout = NULL;
	Scanner *s = NULL;
	int ret = 1;
	Error *err = NULL;

	if (argc != 3) {
		fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
		return 1;
	}

	err = OpenRead(argv[1], &fin);
	if (err != NULL) {
		fprintf(stderr, "error opening %s: %s\n", argv[1], err->String());
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
	if (s->Err() != 0) {
		fprintf(stderr, "error reading from %s: %s\n", argv[1], s->Err()->String());
		goto done;
	}

	ret = 0;
done:
	if (s != NULL)
		delete s;
	if (fout != NULL)
		fclose(fout);
	if (fin != NULL)
		delete fin;
	if (err != NULL)
		delete err;
	return ret;
}
