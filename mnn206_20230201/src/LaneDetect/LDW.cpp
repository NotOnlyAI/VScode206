#include "LDW.h"


#define PI 3.1415926

LDW::LDW()
{

}

int LDW::Init(int FrontDistance, int BackDistance) 
{
    // cv::namedWindow("ldw", 0);
    Initialization();
    H1 = cv::findHomography(img_pts, bev_pts);
    mFrontDistance = FrontDistance;
    mBackDistance = BackDistance;
    return 0;
}


int LDW::Run(std::vector<lane_DECODE> final_lane,cv::Mat vis_img) 
{
    int mDepartureType = 0;
    if (final_lane.size() == 0) //无车道线
        return 1;
    // cv::Mat BEv_img;
    // img_to_BEV(vis_img, BEv_img);
    // cv::Mat img_pad = cv::Mat::zeros(mBackDistance - 720, 1280, CV_8UC3);
    // cv::vconcat(BEv_img, img_pad, BEv_img);
    // cv::polylines(BEv_img, mDrivableArea_Vis, true, cv::Scalar(255, 255, 0), 5);
    // cout<<final_lane.size()<<endl;
    for (int i = 0; i < final_lane.size(); i++) 
    {
        std::vector<cv::Point2f> points(final_lane[i].Lane.begin(), final_lane[i].Lane.end());
        std::vector<cv::Point2f> bev_points;

        _2D_to_BEV(points, bev_points);

        std::vector<cv::Point> bev_points_11(bev_points.begin(), bev_points.end());

        lane_fit(bev_points_11, mBackDistance);
       

        int type = departure_type(bev_points_11);

        


        if (type == 2) {
            // cv::polylines(BEv_img, bev_points_11, false, cv::Scalar(0, 0, 255), 5);
            mDepartureType = type;
        }
        else {
            // cv::polylines(BEv_img, bev_points_11, false, cv::Scalar(0, 255, 0), 5);
        }
    }
    // departure_Vis(mDepartureType, BEv_img);
    // cv::imshow("ldw", BEv_img);
    // cv::waitKey(0);
    // cv::imwrite("ldw.jpg", BEv_img);
    return mDepartureType;

}
int LDW::departure_type(std::vector<cv::Point> bev_points)
{
    linestring<d2::point_xy<double>> singleline;
    std::vector<d2::point_xy<double>> insectResult;
    for (int i = 0; i < bev_points.size(); i++)
    {
        singleline.push_back(d2::point_xy<double>(bev_points[i].x, bev_points[i].y));
    }
    bg::intersection(mDrivableArea_Cal, singleline, insectResult);

    if (insectResult.size() > 0){
        return 2;
    }
    else{
        return 1;
    }
    // float theta = CalculateAngle(bev_points.front(), bev_points.back());
    // cout << "theta " << theta << "\n";

    // if (insectResult.size() > 0) {
    //     if (theta > 0) {
    //         departure_type = 1;
    //     }
    //     if (theta <= 0) {
    //         departure_type = 2;
    //     }
    // }
}

// void LDW::departure_Vis(int mDeparture_type, cv::Mat& vis_img) {

//     std::string text_0 = " No departure !! ";
//     std::string text_1 = " right departure warning !!";
//     std::string text_2 = " left departure warning !!";
//     std::string text_3 = " no lane detected !!";
//     std::vector<std::string> Text{ text_0 ,text_1 ,text_2 ,text_3 };
//     cv::putText(vis_img, Text[mDeparture_type] , cv::Point(100, 200), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 3, 8, 0);
// }

void LDW::Initialization() {
    for (int i = 0; i < mDrivableArea_Vis.size(); i++) {
        append(mDrivableArea_Cal, d2::point_xy<double>(mDrivableArea_Vis[i].x, mDrivableArea_Vis[i].y));
    }
    append(mDrivableArea_Cal, d2::point_xy<double>(mDrivableArea_Vis[0].x, mDrivableArea_Vis[0].y));
}
void LDW::lane_fit(std::vector<cv::Point>& one_lane, int length) {


    // cout<<"11"<<endl;
    reverse(one_lane.begin(), one_lane.end());
    std::vector<cv::Point2f> one_lane_float(one_lane.end()-mFrontDistance, one_lane.end());  //取前几个点
    cv::Mat A;
    // cout<<"22"<<endl;
    polynomial_curve_fit(one_lane_float, 1, A, 0);
    // cout<<"33"<<endl;
    std::vector<cv::Point> one_lane_fit;
    for (int y = one_lane.back().y; y < length; y = y + 10) {
        int x = polyfit_X2Y_1A(y, A);
        cv::Point fit_point(x, y);
        one_lane.push_back(fit_point);
    }
    // cout<<"44"<<endl;
}

double LDW::CalculateAngle(cv::Point Mar1Point, cv::Point Mar2Point)
{
    double k = (double)(Mar2Point.y - Mar1Point.y) / (Mar2Point.x - Mar1Point.x); 
    double arcLength1 = atan(k);   
    double current_angle = arcLength1 * 180 / PI;  
    return current_angle;
}

void LDW::polynomial_curve_fit(std::vector<cv::Point2f>& key_point, int n, cv::Mat& A,int mode)
{
    int N = key_point.size();

    cv::Mat X = cv::Mat::zeros(n + 1, n + 1, CV_64FC1);
    float X_value = 0;
    float Y_value = 0;
    for (int i = 0; i < n + 1; i++)
    {
        for (int j = 0; j < n + 1; j++)
        {
            for (int k = 0; k < N; k++)
            {
                if (mode == 1) {
                    X_value = key_point[k].x;
                }
                else {
                    X_value = key_point[k].y;
                }
                X.at<double>(i, j) = X.at<double>(i, j) +
                    std::pow(X_value, i + j);
            }
        }
    }

    //�������Y
    cv::Mat Y = cv::Mat::zeros(n + 1, 1, CV_64FC1);
    for (int i = 0; i < n + 1; i++)
    {
        for (int k = 0; k < N; k++)
        {
            if (mode == 1) {
                X_value = key_point[k].x;
                Y_value = key_point[k].y;
            }
            else {
                X_value = key_point[k].y;
                Y_value = key_point[k].x;
            }
            Y.at<double>(i, 0) = Y.at<double>(i, 0) +
                std::pow(X_value, i) * Y_value;
        }
    }

    A = cv::Mat::zeros(n + 1, 1, CV_64FC1);
    //������A
    cv::solve(X, Y, A, cv::DECOMP_LU);
}


float LDW::polyfit_X2Y_3A(float x,cv::Mat coefficient) {
    float y = coefficient.at<double>(0, 0) + coefficient.at<double>(1, 0) * x +
        coefficient.at<double>(2, 0) * std::pow(x, 2) +
        coefficient.at<double>(3, 0) * std::pow(x, 3); //+3
    return y;
}

float LDW::polyfit_X2Y_1A(float x, cv::Mat coefficient) {
    float y = coefficient.at<double>(0, 0) + coefficient.at<double>(1, 0) * x;
    return y;
}


void LDW::_2D_to_BEV(std::vector<cv::Point2f>& _2D_one_lane_points, std::vector<cv::Point2f>& _bev_one_lane_points)
{
    cv::perspectiveTransform(_2D_one_lane_points, _bev_one_lane_points, H1);


    std::vector<cv::Point2f>::iterator iter = _bev_one_lane_points.begin();
    // for (; iter != _bev_one_lane_points.end();)
    // {
    //     cout<<"x:"<<(*iter).x<<endl;
    //     cout<<"y:"<<(*iter).y<<endl;
    //     if ((*iter).x < 0 || (*iter).x >1279 || (*iter).y>720 || (*iter).y < 0)
    //         iter = _bev_one_lane_points.erase(iter); 
    //     else
    //         iter++;
    // }


}

void LDW::img_to_BEV(cv::Mat& img, cv::Mat& Bev_img) {
    cv::warpPerspective(img, Bev_img, H1, img.size());
}

