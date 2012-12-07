#include "object.h"

#include "space.h"
#include "scene.h"

#include <iostream>
#include <map>

#include <FL/gl.h>

bool
Object::valid()
{
	return (line.size() > 1);
}

bool
Object::inBounds()
{
	static struct Scene::bounds b = Scene::instance()->getBounds();
	if (body->p.y < b.b-b.t)
		return false;
	return true;
}

Object::Object(Type t) : is_finalized(false), body(0), width(3), mass(1), type(t)
{
}

Object::~Object()
{
	cpSpace *s = Space::instance()->to_cpSpace();
	for (int i=0; i<shapes.size(); i++)
	{
		if (type == STATIC)
			cpSpaceRemoveStaticShape(s, shapes[i]);
		else
			cpSpaceRemoveShape(s, shapes[i]);
		cpShapeFree(shapes[i]);
	}
	if (body && type != STATIC)
	{
		cpSpaceRemoveBody(s, body);
		cpBodyFree(body);
	}
}

void
Object::addPoint(const Point &v)
{
	if (is_finalized)
		return;
	line.push_back(v);
	if (line.size() > 1) // build a box for this
	{
		int last = line.size()-1;
		Point a = line[last-1];
		Point b = line[last];
		Point d(a.x-b.x,a.y-b.y);

		cpVect c;
		cpVect *box = new cpVect[4];

		// tail end
		c = cpvrotate(d, cpv(0,1));
		c = cpvmult(cpvnormalize(c),width);
		box[3] = cpvadd(c,a);
		box[0] = cpvadd(cpvneg(c),a);

		if (boxes.size()) {
		cpVect *old = boxes[boxes.size()-1];
		box[3] = old[2] = cpvmult(cpvadd(old[2],box[3]),0.5);
		box[0] = old[1] = cpvmult(cpvadd(old[1],box[0]),0.5);
		}

		// head end
		c = cpvrotate(d, cpv(0,1));
		c = cpvmult(cpvnormalize(c),width);
		box[2] = cpvadd(c,b);
		box[1] = cpvadd(cpvneg(c),b);

		boxes.push_back(box);
	}
}

void
Object::finalize()
{
	if (is_finalized)
		return;

	Point av(0,0);
	for (int i=0; i<line.size(); i++)
	{
		Point p = line[i];
		av.x += p.x;
		av.y += p.y;
	}
	av.x /= line.size();
	av.y /= line.size();

	std::vector<cpVect> offsets;
	float moment = 0;
	cpVect *v;

	if (type == DYNAMIC | type == ERASER)
	{
		for (int i=0; i<boxes.size(); i++)
		{
			v = boxes[i];

			cpVect c = cpvzero;

			for (int j=0; j<4; j++)
			{
				v[j] = cpvsub(v[j], av); // move to 0,0 coordinates

				c = cpvadd(c,v[j]);
			}
			c = cpvmult(c,0.25); // average
			offsets.push_back(c);
	
			moment += cpMomentForPoly(mass, 4, boxes[i], c);
		}

		body = cpBodyNew(mass*line.size(), moment);
		body->p = cpvzero;
		cpSpaceAddBody(Space::instance()->to_cpSpace(), body);
	}
	else if (type == STATIC)
	{
		body = staticBody;
	}
	
	for (int i=0; i<boxes.size(); i++)
	{
		cpVect *v = boxes[i];

		cpShape *shape = cpPolyShapeNew(body, 4, v, cpvzero);
		shape->data = this;
		shape->collision_type = type;
		shape->u = 0.9;
		shapes.push_back(shape);
		shape_map[shape] = this;

		if (type == DYNAMIC | type == ERASER)
		{
			//shape->u = 1.5;
			cpSpaceAddShape(Space::instance()->to_cpSpace(), shape);
		}
		else if (type == STATIC)
		{
			//shape->u = 1.0;
			cpSpaceAddStaticShape(Space::instance()->to_cpSpace(), shape);
		}
	}

	if (type == DYNAMIC | type == ERASER)
	{
		body->p = av;
	}

	is_finalized = true;
}

int
Object::draw(int s)
{
	if (s == 0 || s != stamp)
	{
		stamp = s;

		if (is_finalized)
		{
			draw_shape();
		}
		else
		{
			draw_line();
		}
		return 1;
	}

	return 0;
}

void
Object::draw_shape()
{
	//std::cout << "Draw at " << body->p.x << ',' << body->p.y << std::endl;
	glBegin(GL_QUADS);
	if (type==DYNAMIC) {
		glColor3f(0,0,1);
	}
	else if (type==STATIC) {
		glColor3f(0,0,0);
	}
	else if (type==ERASER) {
		glColor3f(1,1,0);
	}
	std::vector<cpShape*>::iterator i;
	for (i=shapes.begin(); i!=shapes.end(); i++)
	{
		cpPolyShape *poly = (cpPolyShape *)(*i);
		cpVect *verts = poly->verts;
		for (int j=0; j<4; j++)
		{
			cpVect v = cpvadd(body->p, cpvrotate(verts[j], body->rot));
			glVertex2f(v.x,v.y);
		}
	}
	glEnd();
}

void
Object::draw_line()
{
	glBegin(GL_QUADS);
	if (type==DYNAMIC) {
		glColor3f(0,0,1);
	}
	else if (type==STATIC) {
		glColor3f(0,0,0);
	}
	else if (type==ERASER) {
		glColor3f(1,1,0);
	}
	std::vector<cpVect*>::iterator i;
	for (i=boxes.begin(); i!=boxes.end(); i++)
	{
		cpVect *v = (*i);
		for (int j=0; j<4; j++)
		{
			glVertex2f(v[j].x,v[j].y);
		}
	}
	glEnd();
}

std::map<cpShape*,Object*> Object::shape_map;

Object *Object::shapeToObject(cpShape *s)
{
	return shape_map[s];
}
