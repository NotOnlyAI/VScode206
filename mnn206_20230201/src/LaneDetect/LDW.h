#ifndef _LDW_H_
#define _LDW_H_

#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <stdio.h>
// #include "cuda_runtime_api.h"
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <fstream>
#include <cmath>
#include <unistd.h>
// #include "lane_det.h"
#include "LaneDetect.hpp"

// #include "utilsClass.h"

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/io/io.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <cmath>

using namespace std;
using namespace boost::geometry::model;
using namespace boost::geometry;
namespace bg_model = boost::geometry::model;
namespace bg = boost::geometry;


class LDW {
public:
	LDW(); 
	int Init(int FrontDistance, int BackDistance);
	int Run(std::vector<lane_DECODE> final_lane, cv::Mat vis_img);
	double CalculateAngle(cv::Point Mar1Point, cv::Point Mar2Point);
	void departure_Vis(int mDeparture_type, cv::Mat& Vis_Img);
	int departure_type(std::vector<cv::Point> bev_points);
	void lane_fit(std::vector<cv::Point>& one_lane, int length);
	void Initialization();


public: // utils
    void polynomial_curve_fit(std::vector<cv::Point2f>& key_point, int n, cv::Mat& A, int mode);
    float polyfit_X2Y_3A(float x, cv::Mat coefficient);
    float polyfit_X2Y_1A(float x, cv::Mat coefficient);
    void img_to_BEV(cv::Mat& img, cv::Mat& Bev_img);
    void _2D_to_BEV(std::vector<cv::Point2f>& _2D_one_lane_points, std::vector<cv::Point2f>& _bev_one_lane_points);

private:
	int mFrontDistance;
	int mBackDistance;
	std::vector<cv::Point2f> img_pts{ cv::Point2f(0, 668),cv::Point2f(1280,668),
                                cv::Point2f(565,432),cv::Point2f(715,432) };
    std::vector<cv::Point2f> bev_pts{ cv::Point2f(540,710),cv::Point2f(740,710),
                                        cv::Point2f(540,0),cv::Point2f(740,0) };
	std::vector<cv::Point> mDrivableArea_Vis{ cv::Point(600, 832),cv::Point(600, 784),cv::Point(680, 784),cv::Point(680, 832)};
	polygon<d2::point_xy<double>> mDrivableArea_Cal;
	cv::Mat H1; 
};

#endif

