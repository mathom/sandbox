#include "chipmunk.h"
#include "object.h"
#include "space.h"

cpBody *staticBody = cpBodyNew(INFINITY, INFINITY);

void draw_shape(void *ptr, void *s)
{
	long int stamp = (long int)s;
	cpShape *shape = (cpShape*)ptr;
	Object *o = (Object*)shape->data;
	if (o->draw(stamp)) // if true it was drawn, happens only once per Object
	{
		if (!o->inBounds())
			delete o;
	}
}

void draw_check_objects()
{
	cpSpace *space = Space::instance()->get();

	cpSpaceHashEach(space->staticShapes, &draw_shape, (void*)space->stamp);
	cpSpaceHashEach(space->activeShapes, &draw_shape, (void*)space->stamp);
	
}

int num_objects()
{
	return Space::instance()->get()->bodies->num;
}
