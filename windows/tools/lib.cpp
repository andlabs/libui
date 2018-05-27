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
#include <map>
#include <algorithm>
#include <string.h>
#include <stdint.h>
#include "lib.hpp"

class eofError : public Error {
public:
	virtual ~eofError(void);

	virtual const char *String(void) const;
};

eofError::~eofError(void)
{
	// do nothing
}

const char *eofError::String(void) const
{
	return "EOF";
}

class shortWriteError : public Error {
public:
	virtual ~shortWriteError(void);

	virtual const char *String(void) const;
};

shortWriteError::~shortWriteError(void)
{
	// do nothing
}

const char *shortWriteError::String(void) const
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
	// typeid does not work directly with pointers, alas (see https://stackoverflow.com/questions/4202877/typeid-for-polymorphic-types)
	return typeid (*e) == typeid (eofError);
}

namespace {

struct sliceAlloc {
	char *b;
	size_t n;
	uintmax_t refcount;
};

std::map<uintptr_t, struct sliceAlloc> sliceAllocs;

char *sliceAlloc(size_t n)
{
	struct sliceAlloc sa;

	sa.b = new char[n];
	sa.n = n;
	sa.refcount = 1;
	sliceAllocs[(uintptr_t) (sa.b)] = sa;
	return sa.b;
}

uintptr_t sliceLookup(char *b)
{
	return *(sliceAllocs.lower_bound((uintptr_t) b));
}

void sliceRetain(char *b)
{
	if (b == NULL)
		return;
	sliceAllocs[sliceLookup(b)].refcount++;
}

void sliceRelease(char *b)
{
	uintptr_t key;

	if (b == NULL)
		return;
	key = sliceLookup(b);
	sliceAllocs[key].refcount--;
	if (sliceAllocs[key].refcount == 0) {
		delete[] sliceAllocs[key].b;
		sliceAllocs.erase(key);
	}
}

}

ByteSlice::ByteSlice(void)
{
	this->data = NULL;
	this->len = 0;
	this->cap = 0;
}

ByteSlice::ByteSlice(const ByteSlice &b)
{
	this->data = b.data;
	sliceRetain(this->data);
	this->len = b.len;
	this->cap = b.cap;
}

ByteSlice::ByteSlice(ByteSlice &&b)
{
	this->data = b.data;
	b.data = NULL;
	this->len = b.len;
	b.len = 0;
	this->cap = b.cap;
	b.cap = 0;
}

ByteSlice::ByteSlice(const char *b, size_t n)
{
	this->data = sliceAlloc(n);
	memcpy(this->data, b, n * sizeof (char));
	this->len = n;
	this->cap = n;
}

ByteSlice::ByteSlice(size_t len, size_t cap)
{
	this->data = sliceAlloc(cap);
	memset(this->data, 0, len * sizeof (char));
	this->len = len;
	this->cap = cap;
}

ByteSlice::~ByteSlice(void)
{
	sliceRelease(this->data);
}

ByteSlice &ByteSlice::operator=(const ByteSlice &b)
{
	this->data = b.data;
	sliceRetain(this->data);
	this->len = b.len;
	this->cap = b.cap;
	return *this;
}

ByteSlice &ByteSlice::operator=(ByteSlice &&b)
{
	this->data = b.data;
	b.data = NULL;
	this->len = b.len;
	b.len = 0;
	this->cap = b.cap;
	b.cap = 0;
	return *this;
}

ByteSlice ByteSlice::Slice(size_t start, size_t end)
{
	ByteSlice b;

	b.data = this->data + start;
	sliceRetain(b.data);
	b.len = end - start;
	b.cap = this->cap - start;
	return b;
}

char *ByteSlice::Data(void)
{
	return this->data;
}

const char *ByteSlice::Data(void) const
{
	return this->data;
}

size_t ByteSlice::Len(void) const
{
	return this->len;
}

size_t ByteSlice::Cap(void) const
{
	return this->cap;
}

ByteSlice ByteSlice::Slice(size_t start, size_t end)
{
	ByteSlice b;

	b.data = this->data + start;
	sliceRetain(b.data);
	b.len = end - start;
	b.cap = this->cap - start;
	return b;
}

ByteSlice ByteSlice::Append(const char *b, size_t n)
{
	ByteSlice s;

	if (this->len + n < this->cap) {
		s.data = this->data;
		sliceRetain(s.data);
		s.len = this->len + n;
		s.cap = this->cap;
		memcpy(s.data + this->len, b, n * sizeof (char));
		return s;
	}
	s.data = sliceAlloc(this->len + n);
	memcpy(s.data, this->data, this->len * sizeof (char));
	memcpy(s.data + this->len, b, n * sizeof (char));
	s.len = this->len + n;
	s.cap = this->len + n;
	return s;
}

ByteSlice ByteSlice::Append(const ByteSlice &b)
{
	return this->Append(b.data, b.len);
}

void ByteSlice::CopyFrom(const char *b, size_t n)
{
	n = std::min(this->len, n);
	memcpy(this->data, b, n);
}

void ByteSlice::CopyFrom(const ByteSlice &b)
{
	this->CopyFrom(b.data, b.len);
}

Error *WriteVector(WriteCloser *w, std::vector<char> *v)
{
	return w->Write(v->data(), v->size());
}

#define nbuf 1024

Scanner::Scanner(ReadCloser *r)
{
	this->r = r;
	this->buf = new char[nbuf];
	this->p = this->buf;
	this->n = 0;
	this->line = new std::vector<char>;
	this->err = NULL;
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
	size_t n;

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

Error *Scanner::Err(void) const
{
	if (!IsEOF(this->err))
		return this->err;
	return NULL;
}

void AppendString(std::vector<char> *v, const char *str)
{
	v->insert(v->end(), str, str + strlen(str));
}

Slice::Slice(const char *p, size_t n)
{
	this->p = p;
	this->n = n;
}

const char *Slice::Data(void) const
{
	return this->p;
}

size_t Slice::Len(void) const
{
	return this->n;
}

std::vector<Slice *> *TokenizeWhitespace(const char *buf, size_t n)
{
	std::vector<Slice *> *ret;
	const char *p, *q;
	const char *end;

	ret = new std::vector<Slice *>;
	p = buf;
	end = buf + n;
	while (p < end) {
		if (*p == ' ' || *p == '\t') {
			p++;
			continue;
		}
		for (q = p; q < end; q++)
			if (*q == ' ' || *q == '\t')
				break;
		ret->push_back(new Slice(p, q - p));
		p = q;
	}
	return ret;
}

void FreeTokenized(std::vector<Slice *> *v)
{
	std::for_each(v->begin(), v->end(), [](Slice *s) {
		delete s;
	});
	delete v;
}

void AppendSlice(std::vector<char> *v, Slice *s)
{
	v->insert(v->end(), s->Data(), s->Data() + s->Len());
}
