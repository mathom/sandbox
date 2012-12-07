#ifndef __BASE_H__
#define __BASE_H__

// the size of the world, defined in statics.cpp
struct bounds
{
	int l, b, r, t;
};

extern struct bounds world_bounds;

#endif
