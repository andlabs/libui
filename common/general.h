#ifndef H_COMMON_GENERAL
#define H_COMMON_GENERAL

#ifdef __cplusplus
extern "C" {
#endif

inline int max(int first, int second)
{
	return (first < second) ? second : first;
}

#ifdef __cplusplus
}
#endif

#endif
