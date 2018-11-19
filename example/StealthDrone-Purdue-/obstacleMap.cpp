#include "obstacleMap.h"


void ObstacleMap::determineMapInfo(std::vector<float> &ranges){
	clearMap();

	//north-west upper side
	for(int i=0; i<90; i++){
		float xIntercept = ranges[i]*cosValue[i];
		float yIntercept = sqrt(ranges[i]*ranges[i] - xIntercept*xIntercept);
		int row = -1, col = -1;


		if( xIntercept > 0.0f && xIntercept < 0.5f){
			col = 2;
			if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			} else if( yIntercept >= 1.5f && yIntercept < 2.5f ){
				row = 0;
			}
		} else if( xIntercept >= 0.5f && xIntercept < 1.5 ){
			col = 1;
			if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			} else if( yIntercept >= 1.5f && yIntercept < 2.5f ){
				row = 0;
			}
		} else if( xIntercept >= 1.5f && xIntercept < 2.5 ){
			row = col = 0;
		} else {
			;
		}

		if( row != -1 && col != -1 )
			obstacleMap[row][col] = 1;
	}

	//north-west down side
	for(int i=90; i<180; i++){
		float xIntercept = ranges[i]*cosValue[i];
		float yIntercept = sqrt(ranges[i]*ranges[i] - xIntercept*xIntercept);
		int row = -1, col = -1;


		if( xIntercept > 0.0f && xIntercept < 0.5f){
			row = col = -1;
		} else if( xIntercept >= 0.5f && xIntercept < 1.5 ){
			col = 1;
			if( yIntercept >= 0.0f && yIntercept < 0.5f ){
				row = 2;
			} else if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			}
		} else if( xIntercept >= 1.5f && xIntercept < 2.5 ){
			col = 0;
			if( yIntercept >= 0.0f && yIntercept < 0.5f ){
				row = 2;
			} else if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			} else if( yIntercept >= 1.5f && yIntercept < 2.5f ){
				row = 0;
			}
		} else {
			;
		}

		if( row != -1 && col != -1 )
			obstacleMap[row][col] = 1;
	}

	//north-esat upper side
	for(int i=630; i<720; i++){
		float xIntercept = ranges[i]*cosValue[i%180];
		float yIntercept = sqrt(ranges[i]*ranges[i] - xIntercept*xIntercept);
		int row = -1, col = -1;


		if( xIntercept > 0.0f && xIntercept < 0.5f){
			col = 2;
			if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			} else if( yIntercept >= 1.5f && yIntercept < 2.5f ){
				row = 0;
			}
		} else if( xIntercept >= 0.5f && xIntercept < 1.5f ){
			col = 3;
			if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			} else if( yIntercept >= 1.5f && yIntercept < 2.5f ){
				row = 0;
			}
		} else if( xIntercept >= 1.5f && xIntercept < 2.5f ){
			row = 0;
			col = 4;
		} else {
			;
		}

		if( row != -1 && col != -1 )
			obstacleMap[row][col] = 1;
	}

	//north-east down side
	for(int i=540; i<630; i++){
		float xIntercept = ranges[i]*cosValue[i%180];
		float yIntercept = sqrt(ranges[i]*ranges[i] - xIntercept*xIntercept);
		int row = -1, col = -1;


		if( xIntercept > 0.0f && xIntercept < 0.5f){
			row = col = -1;
		} else if( xIntercept >= 0.5f && xIntercept < 1.5f ){
			col = 3;
			if( yIntercept >= 0.0f && yIntercept < 0.5f ){
				row = 2;
			} else if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			}
		} else if( xIntercept >= 1.5f && xIntercept < 2.5f ){
			col = 4;
			if( yIntercept >= 0.0f && yIntercept < 0.5f ){
				row = 2;
			} else if( yIntercept >= 0.5f && yIntercept < 1.5f ){
				row = 1;
			} else if( yIntercept >= 1.5f && yIntercept < 2.5f ){
				row = 0;
			}
		} else {
			;
		}

		if( row != -1 && col != -1 )
			obstacleMap[row][col] = 1;
	}
}
