#ifndef __POINT_H__
#define __POINT_H__

#include "chipmunk.h"

class Point
{
public:
	Point() : x(0), y(0) {}
	Point(float X, float Y) : x(X), y(Y) {}

	Point &operator = (const Point &v)
        {
		x = v.x;
		y = v.y;
		return *this;
	}

	operator cpVect ()
	{
		return cpv(x,y);
	}

	Point &operator += (const Point &v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}
	Point operator + (float s)
	{
		return Point(x+s,y+s);
	}
	Point operator + (const Point &p)
	{
		return Point(x+p.x,y+p.y);
	}

	Point &operator -= (const Point &v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}
	Point operator - (float s)
	{
		return Point(x-s,y-s);
	}
	Point operator - (const Point &p)
	{
		return Point(x-p.x,y-p.y);
	}

	Point &operator *= (float s)
	{
		x *= s;
		y *= s;
		return *this;
	}
	Point operator * (float s)
	{
		return Point(x*s,y*s);
	}
	Point operator * (const Point &p)
	{
		return Point(x*p.x,y*p.y);
	}

	Point &operator /= (float s)
	{
		x /= s;
		y /= s;
		return *this;
	}
	Point operator / (float s)
	{
		return Point(x/s,y/s);
	}
	Point operator / (const Point &p)
	{
		return Point(x/p.x,y/p.y);
	}


	float x, y;
};

#endif

