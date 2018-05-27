// 21 may 2018
#include <vector>
#include <stdio.h>
#include <string.h>
#include "lib.hpp"

namespace {

class items {
public:
	ByteSlice name;
	ByteSlice callingConvention;
	std::vector<ByteSlice> params;
	ByteSlice returns;
	bool keepReturn;
	ByteSlice cond[2];
};

#define noutbuf 2048

bool generate(ByteSlice line, FILE *fout)
{
	ByteSlice genout;
	std::vector<ByteSlice> tokens;
	size_t i, j;
	items item;
	size_t nw;

	tokens = ByteSliceFields(line);

	i = 0;
	item.returns = tokens.at(i);
	item.keepReturn = false;
	if (item.returns.Data()[0] == '*') {
		item.returns = item.returns.Slice(1, item.returns.Len());
		item.keepReturn = true;
	}
	i++;
	if (tokens.size() % 2 == 1) {
		item.callingConvention = tokens.at(i);
		i++;
	}
	item.name = tokens.at(i);
	i++;
	item.cond[0] = tokens.at(tokens.size() - 2);
	item.cond[1] = tokens.at(tokens.size() - 1);
	item.params.reserve((tokens.size() - 2) - i);
	for (j = 0; j < item.params.capacity(); j++) {
		item.params.push_back(tokens.at(i));
		i++;
	}

	genout = ByteSlice(0, noutbuf);
	genout = genout.AppendString("HRESULT ");
	if (item.callingConvention.Len() != 0) {
		genout = genout.Append(item.callingConvention);
		genout = genout.AppendString(" ");
	}
	genout = genout.Append(item.name);
	genout = genout.AppendString("(");
	for (i = 0; i < item.params.size(); i += 2) {
		genout = genout.Append(item.params[i]);
		genout = genout.AppendString(" ");
		genout = genout.Append(item.params[i + 1]);
		genout = genout.AppendString(", ");
	}
	if (item.keepReturn) {
		genout = genout.Append(item.returns);
		genout = genout.AppendString(" *ret");
	} else if (item.params.size() != 0)
		// remove the trailing comma and space
		genout = genout.Slice(0, genout.Len() - 2);
	else
		genout = genout.AppendString("void");
	genout = genout.AppendString(")\n");

	genout = genout.AppendString("\n");
	nw = fwrite(genout.Data(), sizeof (char), genout.Len(), fout);
	return nw == genout.Len();
}

bool process(ByteSlice line, FILE *fout)
{
	size_t nw;

	if (line.Len() > 0 && line.Data()[0] == '@')
		return generate(line.Slice(1, line.Len()), fout);
	nw = fwrite(line.Data(), sizeof (char), line.Len(), fout);
	if (nw != line.Len())
		return false;
	return fwrite("\n", sizeof (char), 1, fout) == 1;
}

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
	while (s->Scan())
		if (!process(s->Bytes(), fout)) {
			fprintf(stderr, "error writing to %s\n", argv[2]);
			goto done;
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
