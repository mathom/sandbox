#include "space.h"

Space*
Space::instance()
{
	static Space s;
	return &s;
}

Space::Space()
{
	//s = cpSpaceAlloc();
	s = cpSpaceNew();
}

Space::~Space()
{
	//cpSpaceFreeChildren(s);
	//cpSpaceDestroy(s);
	cpSpaceFree(s);
}
