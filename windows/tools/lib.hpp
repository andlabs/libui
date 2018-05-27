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

// super lightweight (can be passed by value without any data being copied) wrapper around an array of bytes that can be sliced further without copying (std::vector can't do that easily)
// this is modelled after Go's slices
class ByteSlice {
	char *data;
	size_t len;
	size_t cap;
public:
	ByteSlice(void);					// default constructor; equivalent to Go's nil slice
	ByteSlice(const ByteSlice &b);		// copy constructor
	ByteSlice(ByteSlice &&b);			// move constructor; sets b to ByteSlice()
	ByteSlice(const char *b, size_t n);
	ByteSlice(size_t len, size_t cap);
	ByteSlice(int len, size_t cap);		// deal with stupid rule about 0 (see https://stackoverflow.com/a/4610586/3408572)
	~ByteSlice(void);

	// note: copy assignment does not use copy-and-swap because I get neither copy-and-swap nor ADL public friend swap functions (https://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom, https://stackoverflow.com/questions/5695548/public-friend-swap-member-function)
	// (and also [14:55:04]  <discord>	<devin> i don't see why you'd need swap-on-copy semantics for anything if you're really just trying to make copy assignments create two references to the same memory)
	ByteSlice &operator=(const ByteSlice &b);		// copy assignment
	ByteSlice &operator=(ByteSlice &&b);		// move assignment; sets b to ByteSlice()

	char *Data(void);
	const char *Data(void) const;
	size_t Len(void) const;
	size_t Cap(void) const;

	ByteSlice Slice(size_t start, size_t end);
	ByteSlice Append(const char *b, size_t n);
	ByteSlice Append(const ByteSlice &b);
	ByteSlice AppendString(const char *str);
	void CopyFrom(const char *b, size_t n);
	void CopyFrom(const ByteSlice &b);
};

class ReadCloser {
public:
	virtual ~ReadCloser(void) = default;

	virtual Error *Read(ByteSlice b, size_t *n) = 0;
};

class WriteCloser {
public:
	virtual ~WriteCloser(void) = default;

	virtual Error *Write(const ByteSlice b) = 0;
};

extern Error *OpenRead(const char *filename, ReadCloser **r);
extern Error *CreateWrite(const char *filename, WriteCloser **w);

class Scanner {
	ReadCloser *r;
	ByteSlice buf;
	ByteSlice p;
	ByteSlice line;
	Error *err;
public:
	Scanner(ReadCloser *r);
	~Scanner(void);

	bool Scan(void);
	ByteSlice Bytes(void) const;
	Error *Err(void) const;
};

extern std::vector<ByteSlice> ByteSliceFields(ByteSlice s);
