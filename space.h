#ifndef __SPACE_H__
#define __SPACE_H__

#include "chipmunk.h"

#define ITERATIONS 10

class Space
{
public:
	static Space *instance();
	cpSpace *to_cpSpace() { return s; }
	cpSpace *get() { return s; }

private:
	Space();
	~Space();

	cpSpace *s;
};

#endif

