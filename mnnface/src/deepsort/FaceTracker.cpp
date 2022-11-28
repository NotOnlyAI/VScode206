#include "FaceTracker.h"


using namespace cv;

namespace {
    // Convert bounding box from [cx,cy,s,r] to [x,y,w,h] style.
    cv::Rect2f get_rect_xysr(const Mat &xysr) {
        auto cx = xysr.at<float>(0, 0), cy = xysr.at<float>(1, 0), s = xysr.at<float>(2, 0), r = xysr.at<float>(3, 0);

        if(s<0.0 || r<0.0 || cx<0.0 || cy<0.0)
        {
            return cv::Rect2f(0, 0, 1, 1);
        }
        float w = sqrt(s * r);
        float h = s / w;
        float x = (cx - w / 2);
        float y = (cy - h / 2);

        return cv::Rect2f(x, y, w, h);
    }
}


FaceTracker::FaceTracker(uint64_t id,Detection &initDetection,int detection_idx) {
    int stateNum = 7;
    int measureNum = 4;
    kf = KalmanFilter(stateNum, measureNum, 0);

    cv::Rect2f initRect(initDetection.m_xmin,initDetection.m_ymin,initDetection.m_width,initDetection.m_height);

    measurement = Mat::zeros(measureNum, 1, CV_32F);

    kf.transitionMatrix = (Mat_<float>(stateNum, stateNum)
            <<
            1, 0, 0, 0, 1, 0, 0,
            0, 1, 0, 0, 0, 1, 0,
            0, 0, 1, 0, 0, 0, 1,
            0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 0, 1);

    setIdentity(kf.measurementMatrix);
    setIdentity(kf.processNoiseCov, Scalar::all(1e-2));
    setIdentity(kf.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(kf.errorCovPost, Scalar::all(1));

    kf.statePost.at<float>(0, 0) = initRect.x + initRect.width / 2;
    kf.statePost.at<float>(1, 0) = initRect.y + initRect.height / 2;
    kf.statePost.at<float>(2, 0) = initRect.area();
    kf.statePost.at<float>(3, 0) = initRect.width / initRect.height;

    m_id=id;
    m_detection_idx=detection_idx;
    m_rect=initRect;
    m_state = 0;

    m_xmin=m_rect.x;
    m_ymin=m_rect.y;
    m_width=m_rect.width;
    m_height=m_rect.height;

    m_x0=initDetection.m_x0;
    m_y0=initDetection.m_y0;
    m_x1=initDetection.m_x1;
    m_y1=initDetection.m_y1;
    m_x2=initDetection.m_x2;
    m_y2=initDetection.m_y2;
    m_x3=initDetection.m_x3;
    m_y3=initDetection.m_y3;
    m_x4=initDetection.m_x4;
    m_y4=initDetection.m_y4;
}


// Predict the estimated bounding box.
void FaceTracker::predict() {

    if(time_since_update>=1)
    {
        m_state = 0;
    }

    if(time_since_update>=n_loss_confirmed)
    {
        hits=0;
        m_state = 0;
    }


    ++time_since_update;
    kf.predict();
}

// Update the state vector with observed bounding box.
void FaceTracker::update(Detection &curDetection) {
    time_since_update = 0;
    ++hits;

    if (hits > n_init && m_state==0) {m_state = 1;}
    if (hits > n_confirmed && m_state<=1) {m_state = 2;}
    if (m_state>=2)
    {
        if (hits % (2*n_confirmed)==0) 
        {
            m_state = 3;
        }
        else
        {
            m_state = 2;
        }
    }
    

    // measurement
    cv::Rect2f stateMat(curDetection.m_xmin,curDetection.m_ymin,curDetection.m_width,curDetection.m_height);
    measurement.at<float>(0, 0) = stateMat.x + stateMat.width / 2;
    measurement.at<float>(1, 0) = stateMat.y + stateMat.height / 2;
    measurement.at<float>(2, 0) = stateMat.area();
    measurement.at<float>(3, 0) = stateMat.width / stateMat.height;

    // update
    kf.correct(measurement);


    m_x0=curDetection.m_x0;
    m_y0=curDetection.m_y0;
    m_x1=curDetection.m_x1;
    m_y1=curDetection.m_y1;
    m_x2=curDetection.m_x2;
    m_y2=curDetection.m_y2;
    m_x3=curDetection.m_x3;
    m_y3=curDetection.m_y3;
    m_x4=curDetection.m_x4;
    m_y4=curDetection.m_y4;
}

void FaceTracker::setReid(int reid) {
    m_Reid=reid;
}



// Return the current state vector
cv::Rect2f FaceTracker::rect() const {
    return get_rect_xysr(kf.statePost);
}

// Return the current state vector
void FaceTracker::getRect(){
    m_rect=get_rect_xysr(kf.statePost);
    m_xmin=m_rect.x;
    m_ymin=m_rect.y;
    m_width=m_rect.width;
    m_height=m_rect.height;
    // printf("m_xmin=%f,m_ymin=%f\n",m_xmin,m_ymin);
}
