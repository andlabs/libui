// 21 may 2018
#include <vector>
#include <stdio.h>
#include <string.h>

bool generate(std::vector<char> *genline, FILE *fout)
{
	std::vector<char> genout;
	size_t nw;

	genout.push_back('/');
	genout.push_back('/');
	genout.push_back(' ');
	genout.insert(genout.end(), genline->begin(), genline->end());
	genout.push_back('\n');

	genout.push_back('\n');
	nw = fwrite(genout.data(), sizeof (char), genout.size(), fout);
	return nw == genout.size();
}

class Scanner {
	FILE *fin;
	char *buf;
	const char *p;
	size_t n;
	std::vector<char> *line;
	bool eof;
	bool error;
public:
	Scanner(FILE *fin);
	~Scanner(void);

	bool Scan(void);
	std::vector<char>::const_iterator BytesBegin(void) const;
	std::vector<char>::const_iterator BytesEnd(void) const;
	bool Error(void) const;
};

#define nbuf 1024

Scanner::Scanner(FILE *fin)
{
	this->fin = fin;
	this->buf = new char[nbuf];
	this->p = this->buf;
	this->n = 0;
	this->line = new std::vector<char>;
	this->eof = false;
	this->error = false;
}

Scanner::~Scanner(void)
{
	delete this->line;
	delete[] this->buf;
}

bool Scanner::Scan(void)
{
	if (this->eof || this->error)
		return false;
	this->line->clear();
	for (;;) {
		if (this->n > 0) {
			size_t j;
			bool haveline;

			haveline = false;
			for (j = 0; j < this->n; j++)
				if (this->p[j] == '\n') {
					haveline = true;
					break;
				}
			this->line->insert(this->line->end(), this->p, this->p + j);
			this->p += j;
			this->n -= j;
			if (haveline) {
				// swallow the \n for the next time through
				this->p++;
				this->n--;
				return true;
			}
			// otherwise, the buffer was exhausted in the middle of a line, so fall through
		}
		// need to refill the buffer
		this->n = fread(this->buf, sizeof (char), nbuf, this->fin);
		if (this->n < nbuf) {
			// TODO what if this->eof && this->error? the C standard does not expressly disallow this
			this->eof = feof(this->fin) != 0;
			this->error = ferror(this->fin) != 0;
			if (this->eof || this->error)
				return false;
			// otherwise process this last chunk of the file
		}
		this->p = this->buf;
	}
}

std::vector<char>::const_iterator Scanner::BytesBegin(void) const
{
	return this->line->cbegin();
}

std::vector<char>::const_iterator Scanner::BytesEnd(void) const
{
	return this->line->cend();
}

bool Scanner::Error(void) const
{
	return this->error;
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
