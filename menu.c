// 20 april 2015
#include "uipriv.h"

// dummy items for various predefined menu identifiers
// these are so we can get a definite unique pointer value
static volatile char dummyQuit = 'q';
static volatile char dummyPreferences = 'p';
static volatile char dummyAbout = 'a';
static volatile char dummySeparator = 's';

const char *const uiMenuItemQuit = &dummyQuit;
const char *const uiMenuItemPreferences = &dummyPreferences;
const char *const uiMenuItemAbout = &dummyAbout;
const char *const uiMenuItemSeparator = &dummySeparator;
