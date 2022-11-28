#ifndef FD_QUALITY_H_
#define FD_QUALITY_H_
#include <string>
#include <map>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <typeinfo>
#include <time.h>
#include <cmath>
#include <cfloat>


#include "sample_comm_nnie.h"
#include "sdc.h"

// int evaluate_facesize(const SDC_SSD_OBJECT_INFO_S &object,float &size);

// int evaluate_position(const SDC_SSD_OBJECT_INFO_S &object, int origin_image_height,int origin_image_width,float &border);

// int evaluate_pose(const  SDC_SSD_OBJECT_INFO_S &object, float &roll, float &yaw, float &pitch);
typedef struct _STRU_Point_T
{
	int x;//坐标X
	int y;//坐标Y

	_STRU_Point_T():x(0),y(0){}
	_STRU_Point_T(int inx, int iny):x(inx), y(iny){}
	_STRU_Point_T& operator =(const _STRU_Point_T&other)
	{
		x = other.x;
		y = other.y;
		return*this;
	}
	_STRU_Point_T(const _STRU_Point_T&other)
	{
		x = other.x;
		y = other.y;
	}
}STRU_Point_T,wsPoint;

typedef struct _STRU_PointF_T
{
	double x;//坐标X
	double y;//坐标Y

	_STRU_PointF_T():x(0),y(0){}
	_STRU_PointF_T(double inx, double iny):x(inx), y(iny){}
	_STRU_PointF_T& operator =(const _STRU_PointF_T&other)
	{
		x = other.x;
		y = other.y;
		return*this;
	}
	_STRU_PointF_T operator+(const _STRU_PointF_T&lhs) 
	{
		_STRU_PointF_T result;
		result.x = lhs.x + x;
		result.y = lhs.y + y;
		return result;
 	}
	_STRU_PointF_T operator-(const _STRU_PointF_T&rhs) 
	{
		_STRU_PointF_T result;
		result.x = x - rhs.x;
		result.y = y - rhs.y;
		return result;
	}
	_STRU_PointF_T operator*( double rhs) 
	{
		_STRU_PointF_T result;
		result.x = x * rhs;
		result.y = y * rhs;
    	return result;
	}
	_STRU_PointF_T operator/(double rhs)
	{
		_STRU_PointF_T result;
		result.x = x / rhs;
		result.y = y / rhs;
		return result;
	}

	double operator^(const _STRU_PointF_T&rhs) 
	{
    	double dx = x - rhs.x;
    	double dy = y - rhs.y;
    	return std::sqrt(dx * dx + dy * dy);
	}

	_STRU_PointF_T(const _STRU_PointF_T&other)
	{
		x = other.x;
		y = other.y;
	}
}STRU_PointF_T;


class Line {
public:
    Line() = default;
    Line(double a, double b, double c)
            : a(a), b(b), c(c) {}

    Line(const _STRU_PointF_T &a, const _STRU_PointF_T &b)
	 {
        double x1 = a.x;
        double y1 = a.y;
        double x2 = b.x;
       	double y2 = b.y;
        // for (y2-y1)x-(x2-x1)y-x1(y2-y1)+y1(x2-x1)=0
        this->a = y2 - y1;
        this->b = x1 - x2;
        this->c = y1 * (x2 - x1) - x1 * (y2 - y1);
    }

    double distance(const _STRU_PointF_T &p) const 
	{
        return std::fabs(a * p.x + b * p.y + c) / std::sqrt(a * a + b * b);
    }

    double fdistance(const _STRU_PointF_T &p) const
	{
        return (a * p.x + b * p.y + c) / std::sqrt(a * a + b * b);
    }

    static bool near_zero(double f) {
        return f <= DBL_EPSILON && -f <= DBL_EPSILON;
    }

    _STRU_PointF_T projection(const _STRU_PointF_T &p) const {
        if (near_zero(a)) {
            _STRU_PointF_T result;
            result.x = p.x;
            result.y = -c / b;
            return  result;
        }
        if (near_zero(b)) {
            _STRU_PointF_T result;
            result.x = -c / a;
            result.y = p.y;
            return result;
        }
        // y = kx + b  <==>  ax + by + c = 0
        auto k = -a / b;
        _STRU_PointF_T o(0, -c / b);
        _STRU_PointF_T project(0, 0);
        project.x = (float) ((p.x / k + p.y - o.y) / (1 / k + k));
        project.y = (float) (-1 / k * (project.x - p.x) + p.y);
        return project;
    }

    double a = 0;
    double b = 0;
    double c = 0;
};


int face_quality(sdc_yuv_frame_s &yuv_frame,const SDC_SSD_OBJECT_INFO_S &object,
                int &size, 
                int &border,
                float &roll,
                float &yaw,
                float &pitch
                );

#endif