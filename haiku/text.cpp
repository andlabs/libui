// 9 april 2015
#include <cstdlib>
#include <cstring>
#include "uipriv_haiku.hpp"
using namespace std;

char *uiHaikuStrdupText(const char *t)
{
	return strdup(t);
}

void uiFreeText(char *t)
{
	free(t);
}
