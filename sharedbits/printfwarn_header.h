// 1 june 2019

#ifdef __GNUC__
#ifdef _WIN32
#define sharedbitsPrintfFunc(decl, fmtpos, appos) \
	decl __attribute__((format(ms_printf, fmtpos, appos)))
#else
#define sharedbitsPrintfFunc(decl, fmtpos, appos) \
	decl __attribute__((format(printf, fmtpos, appos)))
#endif
#else
#define sharedbitsPrintfFunc(decl, fmtpos, appos) \
	decl
#endif
