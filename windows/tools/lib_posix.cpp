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

	virtual Error *Read(void *buf, size_t n, size_t *actual);
};

posixReadCloser::posixReadCloser(int fd)
{
	this->fd = fd;
}

posixReadCloser::~posixReadCloser(void)
{
	close(this->fd);
}

Error *posixReadCloser::Read(void *buf, size_t n, size_t *actual)
{
	ssize_t ret;

	*actual = 0;
	ret = read(this->fd, buf, n);
	if (ret < 0)
		return new posixError(errno);
	if (ret == 0)
		return NewEOF();
	*actual = ret;
	return NULL;
}

class posixWriteCloser : public WriteCloser {
	int fd;
public:
	posixWriteCloser(int fd);
	virtual ~posixWriteCloser(void);

	virtual Error *Write(void *buf, size_t n);
};

posixWriteCloser::posixWriteCloser(int fd)
{
	this->fd = fd;
}

posixWriteCloser::~posixWriteCloser(void)
{
	close(this->fd);
}

Error *posixWriteCloser::Write(void *buf, size_t n)
{
	ssize_t ret;

	ret = write(this->fd, buf, n);
	if (ret < 0)
		return new posixError(errno);
	if (((size_t) ret) != n)
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
