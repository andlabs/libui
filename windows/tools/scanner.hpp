// 21 may 2018
#include <vector>

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

extern int OpenForScanner(const char *filename, int *fd);
extern int CloseForScanner(int fd);
