#ifndef FaceTracker_H
#define FaceTracker_H

#include "opencv2/video/tracking.hpp"
#include "Detection.hpp"
class FaceTracker {
public:
    FaceTracker(){};
    FaceTracker(uint64_t id,Detection &initDetection,int detection_idx);
    void predict();
    void update(Detection &curDetection);
    void setReid(int reid);
    void getRect();


    cv::Rect2f rect() const;
    
    int state() const { return m_state; }
    uint64_t id() const { return m_id; }
    bool lost_too_long(int expiration = 30) { return time_since_update > expiration; };
    int time_since_update = 0;
    uint64_t hits = 0;
    uint64_t m_id = 0;
    int m_Reid = 0;
    int m_state = 0;

    float m_xmin=0.0;
    float m_ymin=0.0;
    float m_width=0.0;
    float m_height=0.0;
    float m_x0=0.0;
    float m_y0=0.0;
    float m_x1=0.0;
    float m_y1=0.0;
    float m_x2=0.0;
    float m_y2=0.0;
    float m_x3=0.0;
    float m_y3=0.0;
    float m_x4=0.0;
    float m_y4=0.0;

    cv::Rect2f m_rect;
    uint64_t n_init = 3;
    uint64_t n_confirmed = 10;
    uint64_t n_loss_confirmed = 10;



    int m_detection_idx=-1;
    float m_detection_iou=0;
    bool m_have_recognize=false;
    bool m_have_show=false;
private:


    std::vector<float> tracked_features={0};
    cv::KalmanFilter kf;
    cv::Mat measurement;
    
};

#endif //KALMAN_H