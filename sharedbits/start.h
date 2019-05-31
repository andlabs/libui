// 30 may 2019

#ifndef sharedbitsPrefix
#error you must define sharedbitsPrefix before including this
#endif
#define sharedbitsPrefixMakeName(x, y) x ## y
#define sharedbitsPrefixExpandMakeName(x, y) sharedbitsPrefixMakeName(x, y)
#define sharedbitsPrefixName(Name) sharedbitsPrefixExpandMakeName(sharedbitsPrefix, Name)
