#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <GL/gl.h>
#include <vector>
#include <map>

#include "point.h"
#include "chipmunk.h"

class Object
{
public:
	enum Type
	{
		DYNAMIC,
		STATIC,
		ERASER
	};

	Object(Type t=DYNAMIC);
	~Object();

	void addPoint(const Point &v);
	void finalize();

	int draw(int stamp = 0);

	bool valid();
	bool inBounds();
	
	static Object *shapeToObject(cpShape *s);

protected:
	bool is_finalized;
	std::vector<Point> line;
	std::vector<cpVect*> boxes;

	cpBody *body;
	std::vector<cpShape*> shapes;
	
	float width, mass;
	int stamp;

	Type type;

private:
	static std::map<cpShape*,Object*> shape_map;

	void draw_line();
	void draw_shape();
};

#endif

