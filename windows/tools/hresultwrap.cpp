// 21 may 2018
#include <vector>
#include <stdio.h>
#include <string.h>

class Scanner {
	FILE *fin;
	char *buf;
	const char *p;
	size_t n;
	std::vector<char> *line;
	bool eof;
	bool eofNextTime;
	bool error;
public:
	Scanner(FILE *fin);
	~Scanner(void);

	bool Scan(void);
	const char *Bytes(void) const;
	size_t Len(void) const;
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
		if (this->eofNextTime) {
			this->eof = true;
			return false;
		}
		this->n = fread(this->buf, sizeof (char), nbuf, this->fin);
		if (this->n < nbuf) {
			// TODO what if this->eofNextTime && this->error? the C standard does not expressly disallow this
			this->eofNextTime = feof(this->fin) != 0;
			this->error = ferror(this->fin) != 0;
			// according to various people in irc.freenode.net/##c, feof() followed by fread() can result in ferror(), so we must be sure not to read twice on a feof()
			// however, because a partial (nonzero) fread() may or may not set feof(), we have to do this whole delayed check acrobatics
			if (this->eofNextTime && this->n == 0)
				this->eof = true;
			if (this->eof || this->error)
				return false;
			// otherwise process this last chunk of the file
		}
		this->p = this->buf;
	}
}

const char *Scanner::Bytes(void) const
{
	return this->line->data();
}

size_t Scanner::Len(void) const
{
	return this->line->size();
}

bool Scanner::Error(void) const
{
	return this->error;
}

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
	FILE *fin = NULL, *fout = NULL;
	Scanner *s = NULL;
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
	if (s->Error()) {
		fprintf(stderr, "error reading from %s\n", argv[1]);
		goto done;
	}

	ret = 0;
done:
	if (s != NULL)
		delete s;
	if (fout != NULL)
		fclose(fout);
	if (fin != NULL)
		fclose(fin);
	return ret;
}
