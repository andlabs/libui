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
#endif
#include <typeinfo>
#include "lib.hpp"

class eofError : public Error {
public:
	virtual ~eofError(void);

	virtual const char *String(void) const;
};

virtual ~eofError::eofError(void)
{
	// do nothing
}

virtual const char *eofError::String(void) const
{
	return "EOF";
}

class shortWriteError : public Error {
public:
	virtual ~shortWriteError(void);

	virtual const char *String(void) const;
};

virtual ~shortWriteError::shortWriteError(void)
{
	// do nothing
}

virtual const char *shortWriteError::String(void) const
{
	return "short write";
}

Error *NewEOF(void)
{
	return new eofError;
}

Error *NewErrShortWrite(void)
{
	return new shortWriteError;
}

bool IsEOF(Error *e)
{
	return typeid (e) == typeid (eofError *);
}

#define nbuf 1024

Scanner::Scanner(ReadCloser *r)
{
	this->r = r;
	this->buf = new char[nbuf];
	this->p = this->buf;
	this->n = 0;
	this->line = new std::vector<char>;
	this->error = NULL;
}

Scanner::~Scanner(void)
{
	if (this->err != NULL)
		delete this->err;
	delete this->line;
	delete[] this->buf;
}

bool Scanner::Scan(void)
{
	readtype n;

	if (this->err != NULL)
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
		this->err = this->r->Read(this->buf, nbuf * sizeof (char), &n);
		if (this->err != NULL)
			return false;
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

int Scanner::Err(void) const
{
	if (!IsEOF(this->err))
		return this->err;
	return NULL;
}
