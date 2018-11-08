#include "lidar.h"


bool Lidar::running = false;
void Lidar::run(){
	while(!running){
		bool hardError = false;

		if( laser.doProcessSimple(scan, hardError) ){
			;
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
