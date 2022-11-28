//
//  tracking.cpp
//  FaceTracking
//
//  Created by Menglong Yang on 2018/11/16.
//  Copyright © 2018 Menglong Yang. All rights reserved.
//

#include "FaceTracking.h"

using namespace std;


Tracking::Tracking(int _max_history_size, double _scale, string algorithm)
{
    max_history_size = _max_history_size;
    trackerType = algorithm;
    iter = 0;
    scale = _scale;
}

Tracking::~Tracking()
{
    list<object*>::iterator obj_iter = objectList.begin();
    for (; obj_iter != objectList.end(); obj_iter++) {
        delete *obj_iter;
    }
    iter = 0;
    objectList.clear();
}

bool Tracking::add(int obj_id, InputArray image, const Rect2d& boundingBox)
{
    if (image.empty()) {
        return false;
    }
    object* obj = new object(obj_id, image, boundingBox, scale, trackerType);
    objectList.push_back(obj);
    return true;
}

bool Tracking::track(Mat image)
{
    if (image.empty())
        return false;

    for (auto obj_iter = objectList.begin(); obj_iter != objectList.end();)
	{
        (*obj_iter)->update(image);
        if ((*obj_iter)->lost_too_long(max_history_size))
		{
            delete *obj_iter;
			obj_iter = objectList.erase(obj_iter);
		}
		else
		{
			obj_iter++;
		}
    }
    return true;
}

double overlap_rate(const Rect2d& a, const Rect2d& b)
{
    double x1 = max(a.x, b.x);
    double y1 = max(a.y, b.y);
    double x2 = min(a.x + a.width, b.x + b.width);
    double y2 = min(a.y + a.height, b.y + b.height);
    double dx = max(0.0, x2 - x1);
    double dy = max(0.0, y2 - y1);

    return (dx * dy) / max(a.width * a.height, b.width * b.height);
}

bool Tracking::track(Mat image, std::vector<Rect> & boundingBox)
{
    bool success = track(image);
    if (!success)
        return false;
    
    vector<bool> matched_bbox(boundingBox.size(), false);
    // std::cout<<"boundingBox.size()"<<boundingBox.size()<<std::endl;
    for (auto obj_iter:objectList)
	{
        const Rect2d& bbox = obj_iter->getLocation();
        Rect2d rec_bbox(0, 0, 0, 0);
        double minDis = 1000000000;
        bool matched = false;
        int match_ind = -1;
        for (int i = 0; i < boundingBox.size(); i++)
		{
            if (!matched_bbox[i])
			{
                double deltaX = (boundingBox[i].x + double(boundingBox[i].width) / 2 - bbox.x - bbox.width / 2);
                double deltaY = (boundingBox[i].y + double(boundingBox[i].height) / 2 - bbox.y - bbox.height / 2);
                double dis = deltaX * deltaX + deltaY * deltaY;
                if (dis < minDis && overlap_rate(bbox, boundingBox[i]) > 0.5)
				{
                    minDis = dis;
                    matched = true;
                    match_ind = i;
                }
            }
        }
        if (matched)
		{
            rec_bbox = boundingBox[match_ind];
			obj_iter->update(image, rec_bbox);
            matched_bbox[match_ind] = true;
        }
        else
		{
			obj_iter->setStatus(-1);
        }
    }
    
    for (int i = 0; i < matched_bbox.size(); i++)
	{
        if (!matched_bbox[i])
            add(iter++, image, Rect2d(int(boundingBox[i].x), boundingBox[i].y, boundingBox[i].width, boundingBox[i].height));
    }
    return true;
}

const std::list< object* >& Tracking::getObjects() const
{
    return objectList;
}
