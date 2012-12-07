#ifndef __SCENE_H__
#define __SCENE_H__

#include "point.h"

class Scene
{
public:
	static Scene *instance();

	Point getPos();
	void setPos(Point p);
	void shiftPos(Point p);

	void setScreenInfo(int w, int h);

	void transform();
	void draw();

	struct bounds
	{ int l,b,r,t; };

	void setBounds(struct bounds bnd) { b = bnd; }
	struct bounds getBounds() { return b; }

private:
	Scene();
	~Scene();

	void prepare();

	int w,h;
	Point pos;

	unsigned int list;

	struct bounds b;
};

#endif
