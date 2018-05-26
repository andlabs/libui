// 21 may 2018
#include <vector>

class Error {
public:
	virtual ~Error(void) = default;

	virtual const char *String(void) const = 0;
};

extern Error *NewEOF(void);
extern Error *NewErrShortWrite(void);
extern bool IsEOF(Error *e);

class ReadCloser {
public:
	virtual ~ReadCloser(void) = default;

	virtual Error *Read(void *buf, size_t n, size_t *actual) = 0;
};

class WriteCloser {
public:
	virtual ~WriteCloser(void) = default;

	virtual Error *Write(void *buf, size_t n) = 0;
};

extern Error *OpenRead(const char *filename, ReadCloser **r);
extern Error *CreateWrite(const char *filename, WriteCloser **w);
extern Error *WriteVector(WriteCloser *w, std::vector<char> *v);

class Scanner {
	ReadCloser *r;
	char *buf;
	const char *p;
	size_t n;
	std::vector<char> *line;
	Error *err;
public:
	Scanner(ReadCloser *r);
	~Scanner(void);

	bool Scan(void);
	const char *Bytes(void) const;
	size_t Len(void) const;
	Error *Err(void) const;
};

class Slice {
	const char *p;
	size_t n;
public:
	Slice(const char *p, size_t n);

	const char *Data(void) const;
	size_t Len(void) const;
};

extern std::vector<Slice *> *TokenizeWhitespace(const char *buf, size_t n);
extern void FreeTokenized(std::vector<Slice *> *v);
extern void AppendSlice(std::vector<char> *v, Slice *s);
