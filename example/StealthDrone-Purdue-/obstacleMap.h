#pragma once

/** 
 * @breif ObstacleMap contains map information that is surrounded for drone.
 *
 * @usage 
 *	1. Create a instance.
 *	2. call function determineMapInfo.
 *  if obstacleMap is 1 -> there is obstacle.
 *  else -> there is free.
 */

#include <iostream>
#include <cmath>
#include <vector>


#define TO_RADIAN(x) ((x)*180.0f)/3.1415f

class ObstacleMap{
	/**
	 * obstacleMap contains obstacle information. (2, 2) is drone position.
	 */ 
	int obstacleMap[5][5];

	/**
	 * cosValue is every 0.5 degree cos values from 0 to 90 degree.
	 * The reason for not real-time processing is due to the operation speed.
	 */
	float cosValue[180];

public:
	ObstacleMap(){
		for(int i=0; i<5; i++){
			for(int j=0; j<5; j++){
				obstacleMap[i][j] = 0;
			}
		}

		for(int i=0; i<180; i++){
			cosValue[i] = cos(TO_RADIAN(i*0.5));
		}
	};

	~ObstacleMap(){
	};

public:
	/**
	 * @param ranges is float vector from lidar scan.
	 */
	void determineMapInfo(std::vector<float> &ranges);
	int (*getObstacleMap())[5]{ return obstacleMap; }

private:
	void clearMap() {
		for(int i=0; i<5; i++){
			for(int j=0; j<5; j++){
				obstacleMap[i][j] = 0;
			}
		}
	}
};
