// 25 may 2018
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "lib.hpp"

class posixError : public Error {
	int error;
public:
	posixError(int error);
	virtual ~posixError(void);

	virtual const char *String(void) const;
};

posixError::posixError(int error)
{
	this->error = error;
}

posixError::~posixError(void)
{
	// do nothing
}

const char *posixError::String(void) const
{
	return strerror(this->error);
}

class posixReadCloser : public ReadCloser {
	int fd;
public:
	posixReadCloser(int fd);
	virtual ~posixReadCloser(void);

	virtual Error *Read(ByteSlice b, size_t *n);
};

posixReadCloser::posixReadCloser(int fd)
{
	this->fd = fd;
}

posixReadCloser::~posixReadCloser(void)
{
	close(this->fd);
}

Error *posixReadCloser::Read(ByteSlice b, size_t *n)
{
	ssize_t ret;

	*n = 0;
	ret = read(this->fd, b.Data(), b.Len());
	if (ret < 0)
		return new posixError(errno);
	if (ret == 0)
		return NewEOF();
	*n = ret;
	return NULL;
}

class posixWriteCloser : public WriteCloser {
	int fd;
public:
	posixWriteCloser(int fd);
	virtual ~posixWriteCloser(void);

	virtual Error *Write(const ByteSlice b);
};

posixWriteCloser::posixWriteCloser(int fd)
{
	this->fd = fd;
}

posixWriteCloser::~posixWriteCloser(void)
{
	close(this->fd);
}

Error *posixWriteCloser::Write(const ByteSlice b)
{
	ssize_t ret;

	ret = write(this->fd, b.Data(), b.Len());
	if (ret < 0)
		return new posixError(errno);
	if (((size_t) ret) != b.Len())
		return NewErrShortWrite();
	return NULL;
}

Error *OpenRead(const char *filename, ReadCloser **r)
{
	int fd;

	*r = NULL;
	fd = open(filename, O_RDONLY, 0644);
	if (fd < 0)
		return new posixError(errno);
	*r = new posixReadCloser(fd);
	return NULL;
}

Error *CreateWrite(const char *filename, WriteCloser **w)
{
	int fd;

	*w = NULL;
	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return new posixError(errno);
	*w = new posixWriteCloser(fd);
	return NULL;
}
