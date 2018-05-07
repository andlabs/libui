/* vim: set noet ts=4 sw=4 sts=4: */
#include <cstdio>
#include <cstring>

#include "planet.h"



static std::string rndName()
{
	const char* pairs = "..lexegezacebisousesarmaindirea.eratenberalavetiedorquanteisrion";
	int i;

	// seed
	uint16_t s[3];
	for( i=0; i<3; ++i ) {
		s[i] = rand();
	}

	// some are longer (4 pairs instead of 3)
	int len = (s[0] & 64) ? 4:3;

	// pick our pairs
	int idx[4];
	for (i=0;i<4;++i) {
		idx[i] = 2*((s[2]>>8) & 0x1f);
		// shuffle
		uint16_t tmp = s[0] + s[1] + s[2];
		s[0] = s[1];
		s[1] = s[2];
		s[2] = tmp;
	}

	// compose
	char buf[4*2 + 1];
	char *p = buf;
	for (i=0; i<len; ++i )
	{
		char c = pairs[idx[i]];
		if (c!='.') {
			*p++ = c;
		}
		c = pairs[idx[i]+1];
		if (c!='.') {
			*p++ = c;
		}
	}
	buf[0] = toupper(buf[0]);
	*p++ = '\0';
	return std::string(buf);
}



void Planet::Randomise() {
	name = rndName();
	economy = rand()&7;
	govt = rand()&7;
	if (govt <=1 ) {
		economy = economy | 2;
	}
	techLevel = 1 + (rand()&3) + (economy^7);
	techLevel += govt/2;
	if (govt & 1) {
		++techLevel;
	}
	population = 8 * 4*techLevel + economy;
	population += govt + 1;
	turnover = ((economy^7)+3)*(govt+4);
	turnover *= population*8;
	radius = 256*((rand()&15) + 11);
}

const char* Planet::econDesc[8] = {"Rich Ind","Average Ind","Poor Ind","Mainly Ind", "Mainly Agri","Rich Agri","Average Agri","Poor Agri"};
const char* Planet::govtDesc[8] = {"Anarchy","Feudal","Multi-gov","Dictatorship","Communist","Confederacy","Democracy","Corporate State"};


