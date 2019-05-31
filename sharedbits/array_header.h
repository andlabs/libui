// 30 may 2019
// requires: alloc_header.h

#ifndef sharedbitsPrefix
#error you must define sharedbitsPrefix before including this
#endif
#define sharedbitsPrefixMakeName(x, y) x ## y
#define sharedbitsPrefixExpand(x) x
#define sharedbitsPrefixName(Name) sharedbitsPrefixMakeName(sharedbitsPrefixExpand(sharedbitsPrefix), Name)

#undef sharedbitsPrefixName
#undef sharedbitsPrefixExpand
#undef sharedbitsPrefixMakeName
