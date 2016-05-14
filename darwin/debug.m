// 13 may 2016
#import "uipriv_darwin.h"

// TODO don't halt on release builds

static void bug(const char *file, const char *line, const char *func, const char *prefix, const char *format, va_list ap)
{
	NSMutableString *str;
	NSString *formatted;

	str = [NSMutableString new];
	[str appendString:[NSString stringWithFormat:@"[libui] %s:%s:%s %s", file, line, func, prefix]];
	formatted = [[NSString alloc] initWithFormat:[NSString stringWithUTF8String:format] arguments:ap];
	[str appendString:formatted];
	[formatted release];
	NSLog(@"%@", str);
	[str release];
	__builtin_trap();
}

void _implbug(const char *file, const char *line, const char *func, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	bug(file, line, func, "POSSIBLE IMPLEMENTATION BUG; CONTACT ANDLABS:\n", format, ap);
	va_end(ap);
}

void _userbug(const char *file, const char *line, const char *func, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	bug(file, line, func, "You have a bug: ", format, ap);
	va_end(ap);
}
