// 21 may 2018
#ifdef _WIN32
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#include <io.h>
#include <sys/stat.h>
#define openfunc _open
#define openflags (_O_RDONLY | _O_BINARY)
#define openmode (_S_IREAD)
#define readfunc _read
#define readtype int
#define closefunc _close
#else
#include <fcntl.h>
#include <unistd.h>
#define openfunc open
#define openflags (O_RDONLY)
#define openmode 0644
#define readfunc read
#define readtype ssize_t
#define closefunc close
#endif
#include <vector>
#include <stdio.h>
#include <string.h>
#include <errno.h>

class Scanner {
	int fd;
	char *buf;
	const char *p;
	size_t n;
	std::vector<char> *line;
	bool eof;
	int error;
public:
	Scanner(int fd);
	~Scanner(void);

	bool Scan(void);
	const char *Bytes(void) const;
	size_t Len(void) const;
	int Error(void) const;
};

#define nbuf 1024

Scanner::Scanner(int fd)
{
	this->fd = fd;
	this->buf = new char[nbuf];
	this->p = this->buf;
	this->n = 0;
	this->line = new std::vector<char>;
	this->eof = false;
	this->error = 0;
}

Scanner::~Scanner(void)
{
	delete this->line;
	delete[] this->buf;
}

bool Scanner::Scan(void)
{
	readtype n;

	if (this->eof || this->error != 0)
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
		n = readfunc(this->fd, this->buf, nbuf * sizeof (char));
		if (n < 0) {
			this->error = errno;
			return false;
		}
		if (n == 0) {
			this->eof = true;
			return false;
		}
		this->p = this->buf;
		this->n = n;
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

int Scanner::Error(void) const
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
	int fin = -1;
	FILE *fout = NULL;
	Scanner *s = NULL;
	int ret = 1;

	if (argc != 3) {
		fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
		return 1;
	}

	fin = openfunc(argv[1], openflags, openmode);
	if (fin < 0) {
		fprintf(stderr, "error opening %s: %s\n", argv[1], strerror(errno));
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
		closefunc(fin);
	return ret;
}
