// 16 may 2019
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "uipriv.h"

#define sharedbitsPrefix uipriv
#include "../sharedbits/alloc_impl.h"
#include "../sharedbits/array_impl.h"
#include "../sharedbits/strsafe_impl.h"
#undef sharedbitsPrefix
