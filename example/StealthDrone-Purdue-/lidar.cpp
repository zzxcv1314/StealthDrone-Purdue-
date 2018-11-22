#include "lidar.h"


bool Lidar::running = false;
void Lidar::run(){
	while(!running){
		bool hardError = false;

		if( countComplete == true )
			continue;


		if( laser.doProcessSimple(scan, hardError) ){
			if( count == 0 ){
				meanOfRanges = scan.ranges;
			} else {
				for(int i=0; i<(unsigned int)scan.ranges.size(); i++){
					if( scan.ranges[i] > 0.01f ){
						meanOfRanges[i] += scan.ranges[i];
					} else {
						meanOfRanges[i] += prevRanges[i];
					}
				}
			}
			prevRanges = scan.ranges;

			count++;
			if( count > 5 ){
				countComplete = true;
				count = 0;
				for( int i=0; i<(unsigned int)meanOfRanges.size(); i++){
					meanOfRanges[i] = meanOfRanges[i]/5.0f;
				}
				meanOfRanges.clear();
			}
			
		}
		usleep(50*1000);
	}

	std::cout << "[Lidar.cpp] End of Running" << std::endl;
}

void Lidar::stop(){
	running = true;
	Stoppable::stop(); // This is from super class and it's necessary.

	std::cout << "[Lidar.cpp] Lidar geting shutdown" << std::endl;

	// These are from Lidar SDK.
	laser.turnOff();
	laser.disconnecting();
}
