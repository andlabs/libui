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
#include <errno.h>
#include "scanner.hpp"

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

int OpenForScanner(const char *filename, int *fd)
{
	*fd = openfunc(filename, openflags, openmode);
	if (*fd < 0)
		return errno;
	return 0;
}

int CloseForScanner(int fd)
{
	if (closefunc(fd) < 0)
		return errno;
	return 0;
}
