#include "scene.h"

#include <math.h>
#include <GL/gl.h>

#include <iostream>

Scene*
Scene::instance()
{
	static Scene s;
	return &s;
}


Point 
Scene::getPos() { return pos; }

void
Scene::setPos(Point p) 
{
	pos = p;
}

void 
Scene::shiftPos(Point p)
{
	pos += p;
	if (pos.x < b.l) pos.x = b.l;
	else if (pos.x > b.r-w) pos.x = b.r-w;
	if (pos.y < b.b) pos.y = b.b;
	else if (pos.y > b.t-h) pos.y = b.t-h;
}

void
Scene::prepare()
{
	float d = 0.5;

	list = glGenLists(1);
	glNewList(list, GL_COMPILE_AND_EXECUTE);

	// draw the sky
	glBegin(GL_QUADS);
	glColor3f(.2,.4,.6);
	glVertex3f(b.r,b.t,d*2);
	glVertex3f(b.l,b.t,d*2);
	glColor3f(.6,.8,1);
	glVertex3f(b.l,b.b,d*2);
	glVertex3f(b.r,b.b,d*2);
	glEnd();

	// draw bumpy ground
	glBegin(GL_TRIANGLE_STRIP);

	int step = (b.r - b.l)/100;
	int width = b.r - b.l;

	glColor3f(.3,1,.2);
	glVertex3f(b.l-step,0,d);
	for (int i=b.l; i<=b.r; i+=step)
	{
		float r = (rand()%20)-10;
		float s1 = sin((float)i/(step*2))*40+200;
		float s2 = -cos((float)i/width*2)*700+700;
		float h = s1 + s2 + r;
		glColor3f(.9,1,.7);
		glVertex3f(i,h,d);
		glColor3f(.3,1,.2);
		glVertex3f(i,0,d);
		//glVertex3f(i+,0
	}
	
	glEnd();

	glEndList();
}

void
Scene::transform()
{
	glTranslatef(-pos.x,-pos.y,-1.0);
}

void
Scene::draw()
{
	if (!list)
		prepare();
	else
		glCallList(list);
}

void
Scene::setScreenInfo(int sw, int sh)
{
	w = sw;
	h = sh;
}

Scene::Scene() : list(0)
{
	b.l = -1600;
	b.r = 1600;
	b.b = 0;
	b.t = 2000;
}

Scene::~Scene()
{
	glDeleteLists(list,1);
}
