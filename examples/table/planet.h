/* vim: set noet ts=4 sw=4 sts=4: */
#ifndef PLANET_H_INCLUDED
#define PLANET_H_INCLUDED

#include <string>

// dummy data generation (with apologies to Bell and Braben ;-)
struct Planet
{
	std::string name;
	int economy;
	int govt;
	int techLevel;
	int turnover;
	int radius;
	int population;

	static const char* econDesc[8];
	static const char* govtDesc[8];

	// set fields to random values
	void Randomise();
};

#endif // PLANET_H_INCLUDED

