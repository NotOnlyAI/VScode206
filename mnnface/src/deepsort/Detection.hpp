

#ifndef Detection_hpp
#define Detection_hpp

#include <stdio.h>
#include <opencv2/tracking.hpp>
#include <map>
#include <cmath>
#include <cfloat>


typedef struct _STRU_PointF_T
{
	float x;//坐标X
	float y;//坐标Y

	_STRU_PointF_T():x(0),y(0){}
	_STRU_PointF_T(float inx, float iny):x(inx), y(iny){}
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
	_STRU_PointF_T operator*( float rhs) 
	{
		_STRU_PointF_T result;
		result.x = x * rhs;
		result.y = y * rhs;
    	return result;
	}
	_STRU_PointF_T operator/(float rhs)
	{
		_STRU_PointF_T result;
		result.x = x / rhs;
		result.y = y / rhs;
		return result;
	}

	float operator^(const _STRU_PointF_T&rhs) 
	{
    	float dx = x - rhs.x;
    	float dy = y - rhs.y;
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
    Line(float a, float b, float c)
            : a(a), b(b), c(c) {}

    Line(const _STRU_PointF_T &a, const _STRU_PointF_T &b)
	 {
        float x1 = a.x;
        float y1 = a.y;
        float x2 = b.x;
       	float y2 = b.y;
        // for (y2-y1)x-(x2-x1)y-x1(y2-y1)+y1(x2-x1)=0
        this->a = y2 - y1;
        this->b = x1 - x2;
        this->c = y1 * (x2 - x1) - x1 * (y2 - y1);
    }

    float distance(const _STRU_PointF_T &p) const 
	{
        return std::fabs(a * p.x + b * p.y + c) / std::sqrt(a * a + b * b);
    }

    float fdistance(const _STRU_PointF_T &p) const
	{
        return (a * p.x + b * p.y + c) / std::sqrt(a * a + b * b);
    }

    static bool near_zero(float f) {
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

    float a = 0;
    float b = 0;
    float c = 0;
};



class Detection
{
public:

	Detection(float &xmin,float &ymin,
			  float &w,float &h,
			  float &score,
			  float &x0,float &y0,
			  float &x1,float &y1,
			  float &x2,float &y2,
			  float &x3,float &y3,
			  float &x4,float &y4,
			  int frame_w,int frame_h,
			  int face_size,int face_position,
			  float pose_roll,float pose_yaw,float pose_pitch);
	~Detection();
	void get_features();
	bool good_quality();

	float m_score;
	float m_xmin;
	float m_ymin;
	float m_width;
	float m_height;
	float m_x0;
	float m_y0;
	float m_x1;
	float m_y1;
	float m_x2;
	float m_y2;
	float m_x3;
	float m_y3;
	float m_x4;
	float m_y4;

	int m_frame_w;
	int m_frame_h;

	int m_face_size;
	int m_face_position;
	float m_pose_roll;
	float m_pose_yaw;
	float m_pose_pitch;

    std::vector<float> m_features;

};

#endif /* Detection_hpp */
