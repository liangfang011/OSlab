//
//  FSCAN.h
//  IOSched
//
//  Created by Liang Fang on 4/24/14.
//  Copyright (c) 2014 Liang Fang. All rights reserved.
//

#ifndef IOSched_FSCAN_h
#define IOSched_FSCAN_h

#include "Scheduler.h"

class FSCAN : public Scheduler {
    enum directionType {up, down};      // up: 1,2,3...  down: 19,18,17...
    int direction;
    int beginProcessTime;
    
public:
    FSCAN() {
        direction = up;                 // initial scan direction is up
        beginProcessTime = numeric_limits<int>::min();
    }
    
    /* schedule funtion **/
    void schedule() {
        for (int i = 0; i < numOfRequest; i++) {                    // update all IO requests' arrival time
            timeInfo.arrival_time[i] = IO_queue[i].arrivalTime;     // The order is not scheduling order !!!!!!
        }
        
        while (!IO_queue.empty()) {
            checkIfScanToEnds();
            
            // choose an IO request to disk
            int minDist = numeric_limits<int>::max();
            int minIndex = -1;
            for (int i = 0; i < IO_queue.size(); i++) {
                if (IO_queue[i].arrivalTime <= beginProcessTime) {     // if this IO request belongs to a processing queue
                    
                    // find closest IO request in the direction of scan
                    if ((direction == up && IO_queue[i].track >= headPos) || (direction == down && IO_queue[i].track <= headPos)) {                           if (minDist > abs(IO_queue[i].track - headPos)) {
                            minDist = abs(IO_queue[i].track - headPos);
                            minIndex = i;
                        }
                    }
                    
                }
            }
            
            if (minIndex != -1) {                                       // one IO request is chosen, names io
                
                IORequest io = IO_queue[minIndex];
                IO_queue.erase(IO_queue.begin() + minIndex);            // erase chosen IO request from IO_queue
                
                timeInfo.diskStart_time[io.index] = timeInfo.TIME;
                
                int headMoveTime = abs(io.track - headPos);     // time from head begin to move to IO completion
                timeInfo.total_movement += headMoveTime;        // update total head move number
                timeInfo.TIME += headMoveTime;                  // set current time to head moving finish time
                timeInfo.complete_time[io.index] = timeInfo.TIME;      // update complete time
                headPos = io.track;
                
                // check if we need change scan direction
                checkIfScanToEnds();
                
            } else {                                            // no IO request arrive yet with track in the direction
                if (!IO_queue.empty()) {
                    if (timeInfo.TIME < IO_queue.front().arrivalTime) {
                        timeInfo.TIME = IO_queue.front().arrivalTime;
                    }
                    beginProcessTime = timeInfo.TIME;
                    direction = up;
                }
            
            }
            
        }
    }
    
    void checkIfScanToEnds() {
        // check if we need change scan direction
        int biggestTrack = numeric_limits<int>::min();
        int smallestTrack = numeric_limits<int>::max();
        for (int i = 0; i < IO_queue.size(); i++) {
            if (IO_queue[i].arrivalTime <= beginProcessTime) {
                if (biggestTrack < IO_queue[i].track) {         // find IO requests which are on two sides
                    biggestTrack = IO_queue[i].track;
                }
                if (smallestTrack > IO_queue[i].track) {
                    smallestTrack = IO_queue[i].track;
                }
            }
        }
        
        // check if scan to the last request on this direction, then change direction
        if (headPos > biggestTrack && direction == up) {
            direction = down;
        }
        if (headPos < smallestTrack && direction == down) {
            direction = up;
        }
    }
};

#endif
