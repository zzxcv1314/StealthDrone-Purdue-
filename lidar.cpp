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
	Stoppable::stop();

	std::cout << "[Lidar.cpp] Lidar geting shutdown" << std::endl;
	laser.turnOff();
	laser.disconnecting();
}
