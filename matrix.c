// 11 october 2015
#include "ui.h"
#include "uipriv.h"

void setIdentity(uiDrawMatrix *m)
{
	m->M11 = 1;
	m->M12 = 0;
	m->M21 = 0;
	m->M22 = 1;
	m->M31 = 0;
	m->M32 = 0;
}

// TODO skew

// see windows/draw.c for more information
