// 20 april 2015
#include "uipriv.h"

// dummy items for various predefined menu identifiers
// these are so we can get a definite unique pointer value
static volatile char dummyQuit = 'q';
static volatile char dummyPreferences = 'p';
static volatile char dummyAbout = 'a';

const char *const uiMenuQuitItem = &dummyQuit;
const char *const uiMenuPreferencesItem = &dummyPreferences;
const char *const uiMenuAboutItem = &dummyAbout;
