// 21 may 2018
#include <vector>
#include <stdio.h>
#include <string.h>

#define nbuf 1024

bool generate(std::vector<char> *genline, FILE *fout)
{
	std::vector<char> genout(nbuf);
	size_t nw;

	genout.push_back('/');
	genout.push_back('/');
	genout.push_back(' ');
	genout.insert(genout.end(), genline->begin(), genline->end());
	genout.push_back('\n');

	genout.push_back('\n');
	nw = fwrite(genout.data(), sizeof (char), genout.size(), fout);
	return nw != genout.size();
}

struct process {
	// each of these returns the number of bytes in buf that were processed
	size_t (*state)(struct process *p, const char *buf, size_t n, FILE *fout);
	bool error;
	std::vector<char> *genline;
};

static size_t stateError(struct process *p, const char *buf, size_t n, FILE *fout)
{
	p->error = true;
	return n;
}

size_t stateCopyLine(struct process *p, const char *buf, size_t n, FILE *fout);
size_t stateGenerate(struct process *p, const char *buf, size_t n, FILE *fout);

size_t stateNewLine(struct process *p, const char *buf, size_t n, FILE *fout)
{
	if (n > 0 && buf[0] == '@') {
		p->state = stateGenerate;
		return 1;		// skip the @
	}
	p->state = stateCopyLine;
	return 0;			// don't skip anything
}

size_t stateCopyLine(struct process *p, const char *buf, size_t n, FILE *fout)
{
	size_t nw;
	size_t j;

	for (j = 0; j < n; j++)
		if (buf[j] == '\n') {
			// include the newline; that's being copied too
			j++;
			break;
		}
	nw = fwrite(buf, sizeof (char), j, fout);
	if (nw != j) {
		p->state = stateError;
		return 0;
	}
	// and on to the next line
	p->state = stateNewLine;
	return j;
}

size_t stateGenerate(struct process *p, const char *buf, size_t n, FILE *fout)
{
	size_t j;

	if (p->genline == NULL)
		p->genline = new std::vector<size_t>(n);
	for (j = 0; j < n; j++)
		if (buf[j] == '\n')
			// do NOT include the newline this time
			break;
	p->genline->insert(p->genline->end(), buf, buf + j);
	if (j == n)		// '\n' not found; not finished with the line yet
		return j;
	// finished with the line; process it and continue
	p->state = stateNewLine;
	if (!generate(p->genline, fout))
		p->state = stateError;
	delete p->genline;
	p->genline = NULL;
	// buf[j] == '\n' and generate() took care of printing a newline
	return j + 1;
}

void processInit(struct process *p)
{
	memset(p, 0, sizeof (struct process));
	p->state = stateNewLine;
}

bool process(struct process *p, const char *buf, size_t n, FILE *fout)
{
	size_t np;

	while (n != 0) {
		np = (*(p->state))(p, buf, n, fout);
		buf += np;
		n -= np;
	}
	return p->error;
}

int main(int argc, char *argv[])
{
	FILE *fin = NULL, *fout = NULL;
	char buf[nbuf];
	size_t n;
	struct process p;
	int ret = 1;

	if (argc != 3) {
		fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
		return 1;
	}

	fin = fopen(argv[1], "rb");
	if (fin == NULL) {
		fprintf(stderr, "error opening %s\n", argv[1]);
		goto done;
	}
	fout = fopen(argv[2], "wb");
	if (fout == NULL) {
		fprintf(stderr, "error creating %s\n", argv[2]);
		goto done;
	}

	processInit(&p);
	for (;;) {
		n = fread(buf, sizeof (char), nbuf, fin);
		if (n == 0)
			break;
		if (!process(&p, buf, n, fout)) {
			fprintf(stderr, "error writing to %s\n", argv[2]);
			goto done;
		}
	}
	if (!feof(fin)) {
		fprintf(stderr, "error reading from %s\n", argv[1]);
		goto done;
	}

	ret = 0;
done:
	if (fin != NULL)
		fclose(fin);
	if (fout != NULL)
		fclose(fout);
	return ret;
}
