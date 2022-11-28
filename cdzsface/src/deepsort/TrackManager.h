//
//  tracking.hpp
//  FaceTracking
//
//  Created by Menglong Yang on 2018/11/16.
//  Copyright © 2018 Menglong Yang. All rights reserved.
//

#ifndef tracking_hpp
#define tracking_hpp

#include <stdio.h>


#include "FaceTracker.h"
#include "Detection.hpp"
#include <map>
#include <list>

using namespace cv;

class TrackManager
{
public:

    TrackManager(int _max_history_size = 30);
    ~TrackManager();
    bool track();
    bool track(std::vector<Detection> & detections);
    std::list< FaceTracker *> m_trackerList;
    
private:
    
    bool add(uint64_t tracker_id, Detection &initDetection,int detection_idx);
    
    int max_history_size;
    uint64_t iter;
};


#endif /* tracking_hpp */
