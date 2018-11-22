#include "obstacleMap.h"


//void ObstacleMap::determineMapInfo(std::vector<float> ranges){
void ObstacleMap::determineMapInfo(float* ranges){
	clearMap();

	//north-west upper side
	for(int i=0; i<90; i++){
		float xValue = ranges[i]*sinValue[i];
		float yValue = ranges[i]*cosValue[i];
		int row = -1, col = -1;


		if( xValue > 0.0f && xValue < 0.5f){
			col = 2;
			if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			} else if( yValue >= 1.5f && yValue < 2.5f ){
				row = 0;
			}
		} else if( xValue >= 0.5f && xValue < 1.5f ){
			col = 1;
			if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			} else if( yValue >= 1.5f && yValue < 2.5f ){
				row = 0;
			}
		} else if( xValue >= 1.5f && xValue < 2.5f ){
			row = col = 0;
		} else {
			;
		}

		if( row != -1 && col != -1 )
			obstacleMap[row][col] = 1;
	}

	//north-west down side
	for(int i=90; i<180; i++){
		float xValue = ranges[i]*sinValue[i];
		float yValue = ranges[i]*cosValue[i];
		int row = -1, col = -1;

		if( xValue > 0.0f && xValue < 0.5f){
			row = col = -1;
		} else if( xValue >= 0.5f && xValue < 1.5f ){
			col = 1;
			if( yValue >= 0.0f && yValue < 0.5f ){
				row = 2;
			} else if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			}
		} else if( xValue >= 1.5f && xValue < 2.5f ){
			col = 0;
			if( yValue >= 0.0f && yValue < 0.5f ){
				row = 2;
			} else if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			} else if( yValue >= 1.5f && yValue < 2.5f ){
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
		float xValue = ranges[i]*cosValue[i%180];
		float yValue = ranges[i]*sinValue[i%180];
		int row = -1, col = -1;


		if( xValue > 0.0f && xValue < 0.5f){
			col = 2;
			if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			} else if( yValue >= 1.5f && yValue < 2.5f ){
				row = 0;
			}
		} else if( xValue >= 0.5f && xValue < 1.5f ){
			col = 3;
			if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			} else if( yValue >= 1.5f && yValue < 2.5f ){
				row = 0;
			}
		} else if( xValue >= 1.5f && xValue < 2.5f ){
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
		float xValue = ranges[i]*cosValue[i%180];
		float yValue = ranges[i]*sinValue[i%180];
		int row = -1, col = -1;


		if( xValue > 0.0f && xValue < 0.5f){
			row = col = -1;
		} else if( xValue >= 0.5f && xValue < 1.5f ){
			col = 3;
			if( yValue >= 0.0f && yValue < 0.5f ){
				row = 2;
			} else if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			}
		} else if( xValue >= 1.5f && xValue < 2.5f ){
			col = 4;
			if( yValue >= 0.0f && yValue < 0.5f ){
				row = 2;
			} else if( yValue >= 0.5f && yValue < 1.5f ){
				row = 1;
			} else if( yValue >= 1.5f && yValue < 2.5f ){
				row = 0;
			}
		} else {
			;
		}

		if( row != -1 && col != -1 )
			obstacleMap[row][col] = 1;
	}
}
