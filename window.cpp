#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl.H>

#include <vector>
#include <iostream>
#include <math.h>

#include "point.h"
#include "object.h"
#include "space.h"
#include "scene.h"

#define DEBUG

#define SCRIBBLER_VERSION_MAJOR 0
#define SCRIBBLER_VERSION_MINOR 0
#define SCRIBBLER_VERSION_PATCH 2

#define FRAME_TIME (1.0f/60.0f)

class AppWindow : public Fl_Gl_Window
{
public:
	AppWindow(int width, int height, char* title);

	void initGL();
	void initMatrix();
	void initPhysics();
	void updatePhysics();
	
	virtual void draw();
	virtual int handle(int event);
	virtual void resize(int x, int y, int w, int h);
	
	void delayedRemove(Object *o);

private:
	Object *obj; // holds an object as it is being drawn

	bool help;

	struct m_winfo {
		int x, y, w, h, f;
	} winfo;

	Scene *scene;
	std::vector<Object*> removed_objects;
};

AppWindow::AppWindow(int w, int h, char* title) : Fl_Gl_Window(w, h, title), obj(0), help(false)
{
	mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);
	winfo.f = 0;
	scene = Scene::instance();
	scene->setScreenInfo(w,h);

	struct Scene::bounds b = scene->getBounds();
	size_range(100,100,b.r-b.l,b.t-b.b);
}

void
AppWindow::initGL()
{
	glClearColor(1.0,1.0,1.0,1);
	//glEnable(GL_DEPTH_TEST);
	gl_font(FL_HELVETICA,12);
}

void
AppWindow::initMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glFrustum(0,w(),h(),0,1,100); // left right bottom top near far
	ortho();
}

// a is the eraser...
int collide_eraser_dynamic(cpShape *a, cpShape *b, 
                           cpContact *contacts, int numContacts, 
						   cpFloat normal_coef, void *data)
{
	cpSpace *space = Space::instance()->to_cpSpace();
	AppWindow *aw = (AppWindow*)data;
	
	Object *o;
	o = Object::shapeToObject(a);
	aw->delayedRemove(o);
	o = Object::shapeToObject(b);
	aw->delayedRemove(o);
	
	return 0;
}
int collide_eraser_static(cpShape *a, cpShape *b, 
                           cpContact *contacts, int numContacts, 
						   cpFloat normal_coef, void *data)
{
	cpSpace *space = Space::instance()->to_cpSpace();
	AppWindow *aw = (AppWindow*)data;
	
	Object *o;
	o = Object::shapeToObject(a);
	aw->delayedRemove(o);
	o = Object::shapeToObject(b);
	aw->delayedRemove(o);
	
	return 0;
}

void
AppWindow::initPhysics()
{
	cpInitChipmunk();
	cpResetShapeIdCounter();
	//cpSpaceResizeStaticHash(Space::instance()->to_cpSpace(), 20.0, 999); // space, av size, count
	cpSpace *s = Space::instance()->to_cpSpace();
	s->gravity = cpv(0, -900);
	cpSpaceAddCollisionPairFunc(s,Object::ERASER,Object::DYNAMIC,
								collide_eraser_dynamic,this);
	cpSpaceAddCollisionPairFunc(s,Object::ERASER,Object::STATIC,
								collide_eraser_static,this);
}

void
AppWindow::updatePhysics()
{
	int steps = 2;
	float dt = FRAME_TIME/steps;

	for (int i=0; i<steps; i++)
	{
		cpSpaceStep(Space::instance()->to_cpSpace(), dt);
	}

	std::map<Object*,int> processed_object;
	for (int i=0; i<removed_objects.size(); i++)
	{
		Object *o = removed_objects[i];
		if (!processed_object[o])
		{
			processed_object[o] = 1;
			delete o;
		}
	}
	removed_objects.clear();
}

void
draw_frame(void* o)
{
	((Fl_Gl_Window*)o)->redraw();
	Fl::repeat_timeout(FRAME_TIME, draw_frame, o);
}

void
AppWindow::draw()
{
	static char version_string[16];
	if (!valid())
	{
		snprintf(version_string,16,"%d.%d%d",SCRIBBLER_VERSION_MAJOR,SCRIBBLER_VERSION_MINOR,SCRIBBLER_VERSION_PATCH);
		initPhysics();
		initGL();
		initMatrix();
		Fl::add_timeout(FRAME_TIME, draw_frame, this);
	}

	updatePhysics();

	#ifndef MESA
	//glDrawBuffer(GL_FRONT_AND_BACK);
	#endif // !MESA

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	scene->transform(); // setup camera

	scene->draw();

	draw_check_objects();

	if (obj)
		obj->draw();

	glPushMatrix();
	Point p = scene->getPos();
	glTranslatef(p.x,p.y,0);
	static char status[128];
	glColor3f(1,0,0);
	snprintf(status,128,"Scribbler sandbox v%s by Matt Thompson, 2007", version_string);
	gl_draw(status,10,h()-20);

	glColor3f(0,1,0);
	snprintf(status,128,"<F1> to toggle help");
	gl_draw(status,10,h()-40);

	if (help) {
		glColor3f(1,0,1);
		snprintf(status,128,"<ESC> to quit");
		gl_draw(status,30,h()-60);
		glColor3f(0,0,1);
		snprintf(status,128,"<left mouse> to draw dynamic objects");
		gl_draw(status,30,h()-80);
		glColor3f(0,0,0);
		snprintf(status,128,"<right mouse> to draw static objects");
		gl_draw(status,30,h()-100);
		glColor3f(0,1,0);
		snprintf(status,128,"<SHIFT> + <any mouse> to move the camera");
		gl_draw(status,30,h()-120);
		glColor3f(1,1,0);
		snprintf(status,128,"<CTRL> + <any mouse> to spawn erasers");
		gl_draw(status,30,h()-140);
	}

	#ifdef DEBUG
	glColor3f(0,0,0);
	snprintf(status,128,"%d live bodies", num_objects());
	gl_draw(status,10,10);
	#endif

	glPopMatrix();

	#ifndef MESA
	//glDrawBuffer(GL_BACK);
	#endif // !MESA

}

int
AppWindow::handle(int event)
{
	static int drawing = 0, moving = 0, erasing = 0, moving_alt = 0;
	static int x, y, ox, oy, tx, ty;
	bool finalizing = false;

	switch (event) {
		case FL_PUSH:
			//mouse down...
			// pos in Fl::event_x() Fl::event_y()
			if (obj == 0 && !drawing && !moving)
			{
				if (erasing || Fl::event_button() == 2) // middle or ctrl
				{
					obj = new Object(Object::ERASER);
					drawing = 1;
					erasing = 0;
				}
				else if (Fl::event_button() == 3) //right
				{
					obj = new Object(Object::STATIC);
					drawing = 1;
				}
				else if (Fl::event_button() == 1) //left
				{
					obj = new Object(Object::DYNAMIC);
					drawing = 1;
				}
			}
			x = Fl::event_x();
			y = h()-Fl::event_y();

			if (drawing)
			{
				obj->addPoint(Point(x,y)+scene->getPos());
			}
			ox = x;
			oy = y;

			return 1;
		case FL_MOVE:
			x = Fl::event_x();
			y = h()-Fl::event_y();
			tx = x-ox;
			ty = y-oy;
			if (moving_alt)
			{
				ox = x;
				oy = y;
				scene->shiftPos(Point(-(float)tx,-(float)ty));			
			}
			return 1;
		case FL_RELEASE:
			if (drawing)
				finalizing = true;
		case FL_DRAG:
			x = Fl::event_x();
			y = h()-Fl::event_y();
			tx = x-ox;
			ty = y-oy;
			if (drawing && tx*tx+ty*ty > 300)
			{
				obj->addPoint(Point(x,y)+scene->getPos());
				ox = x;
				oy = y;
			}
			else if (moving)
			{
				ox = x;
				oy = y;
				scene->shiftPos(Point(-(float)tx,-(float)ty));
			}
			else if (erasing)
			{
				// remove objects
				//scene->remove(Point((float)x,(float)y));
			}

			if (finalizing)
			{
				if (obj->valid())
				{
					obj->finalize();
				}
				else
				{
					delete obj;
				}
				obj = 0;
				drawing = 0;
				erasing = 0;
			}
			return 1;
		case FL_FOCUS:
		case FL_UNFOCUS:
			return 1; // 0 for no keyboard events
		case FL_KEYDOWN:
			// keys in Fl::event_key()
			// ascii in Fl::event_text()
			if (Fl::event_key() == FL_Escape) {
				hide();
			}
			else if (Fl::event_key() == 'f') {
				if (!winfo.f) {
					winfo.f = 1;
					winfo.x = this->x();
					winfo.y = this->y();
					winfo.w = w();
					winfo.h = h();
					fullscreen();
				}
				else {
					winfo.f = 0;
					fullscreen_off(winfo.x,winfo.y,winfo.w,winfo.h);
				}
			}
			else if (!drawing && !moving && Fl::event_key() == FL_Shift_L || Fl::event_key() == FL_Shift_R)
			{
				moving = 1;
			}
			else if (!drawing && !erasing && Fl::event_key() == FL_Control_L || Fl::event_key() == FL_Control_R)
			{
				erasing = 1;
			}
			else if (Fl::event_key() == FL_F+1) {
				help = !help;
			}
			else if (Fl::event_key() == ' ')
			{
				moving_alt = 1;
			}
			return 1;
		case FL_KEYUP:
			if (Fl::event_key() == FL_Shift_L || Fl::event_key() == FL_Shift_R) {
				moving = 0;
				erasing = 0;
			}
			else if (Fl::event_key() == ' ')
			{
				moving_alt = 0;
			}
		case FL_SHORTCUT:
			return 1;
		default:
			return Fl_Gl_Window::handle(event);
	}
}

void
AppWindow::delayedRemove(Object *o)
{
	removed_objects.push_back(o);
}

void
AppWindow::resize(int x, int y, int w, int h)
{
	static int ow=0, oh=0;
	if (ow != w || oh != h) // skip it if we just move the window and not resize
	{
		ow = w;
		oh = h;
		scene->setScreenInfo(w,h);
		scene->shiftPos(Point()); // make sure the camera is on the canvas
	}
	Fl_Gl_Window::resize(x,y,w,h);
}

int main(int argc, char** argv)
{
	AppWindow* aw = new AppWindow(800,600,"Scribbler");

	aw->show(argc, argv);

	return Fl::run();
}
