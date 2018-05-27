// 21 may 2018
#include <vector>
#include <stdio.h>
#include <string.h>
#include "lib.hpp"

struct item {
	Slice *name;
	Slice *callingConvention;
	Slice **params;
	size_t nParams;
	Slice *returns;
	bool keepReturn;
	Slice *cond[2];
};

bool generate(const char *line, size_t n, FILE *fout)
{
	std::vector<char> genout;
	std::vector<Slice *> *tokens;
	size_t i, j;
	struct item item;
	size_t nw;

	tokens = TokenizeWhitespace(line, n);

	memset(&item, 0, sizeof (struct item));
	i = 0;
	item.returns = tokens->at(i);
	if (item.returns->Data()[0] == '*') {
		item.returns = new Slice(item.returns->Data() + 1, item.returns->Len() - 1);
		item.keepReturn = true;
	}
	i++;
	if (tokens->size() % 2 == 1) {
		item.callingConvention = tokens->at(i);
		i++;
	}
	item.name = tokens->at(i);
	i++;
	item.cond[0] = tokens->at(tokens->size() - 2);
	item.cond[1] = tokens->at(tokens->size() - 1);
	item.nParams = (tokens->size() - 2) - i;
	item.params = new Slice *[item.nParams];
	for (j = 0; j < item.nParams; j++) {
		item.params[j] = tokens->at(i);
		i++;
	}

	AppendString(&genout, "HRESULT ");
	if (item.callingConvention != NULL) {
		AppendSlice(&genout, item.callingConvention);
		genout.push_back(' ');
	}
	AppendSlice(&genout, item.name);
	genout.push_back('(');
	for (i = 0; i < item.nParams; i += 2) {
		AppendSlice(&genout, item.params[i]);
		genout.push_back(' ');
		AppendSlice(&genout, item.params[i + 1]);
		genout.push_back(',');
		genout.push_back(' ');
	}
	if (item.keepReturn) {
		AppendSlice(&genout, item.returns);
		AppendString(&genout, " *ret");
	} else if (item.nParams != 0) {
		// remove the trailing comma and space
		genout.pop_back();
		genout.pop_back();
	} else
		AppendString(&genout, "void");
	genout.push_back(')');
	genout.push_back('\n');

	delete[] item.params;
	if (item.keepReturn)
		delete item.returns;
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
