// 25 may 2018
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
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

virtual posixError::~posixError(void)
{
	// do nothing
}

virtual const char *posixError::String(void) const
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

virtual posixReadCloser::~posixReadCloser(void)
{
	close(this->fd);
}

virtual Error *posixReadCloser::Read(void *buf, size_t n, size_t *actual)
{
	ssize_t ret;

	*actual = 0;
	ret = read(this->fd, buf, n);
	if (ret < 0)
		return new posixError(errno);
	if (ret == 0)
		return NewEOF(void);
	*actual = ret;
	return NULL;
}

class posixWriteCloser : public WriteCloser {
	int fd;
public:
	posixWriteCloser(int fd);
	virtual ~posixWriteCloser(void);

	virtual Error *Write(void *buf, size_t n, size_t *actual);
};

posixWriteCloser::posixWriteCloser(int fd)
{
	this->fd = fd;
}

virtual posixWriteCloser::~posixWriteCloser(void)
{
	close(this->fd);
}

virtual Error *posixWriteCloser::Write(void *buf, size_t n)
{
	ssize_t ret;

	ret = write(this->fd, buf, n);
	if (ret < 0)
		return new posixError(errno);
	if (ret != n)
		return NewErrShortWrite(void);
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
