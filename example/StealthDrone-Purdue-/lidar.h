#pragma once

/**
  * @brief wrapper class for CYdlidar.
  */

#include"CYdLidar.h"
#include"stoppableThread.h"
#include<iostream>
#include<string>
#include<signal.h>
#include<memory>
#include<unistd.h>
#include<vector>

using namespace std;
using namespace ydlidar;

class Lidar : public Stoppable{
	// @var laser
	// 		from Lidar SDK.
	CYdLidar laser;

	// @var scan
	//		from Lidar SDK.
	LaserScan scan;

	// @var running
	// 		determine Lidar Thread is running or not.
	static bool running;

	// @var port
	// 		information of Lidar port, default port is /dev/ttyUSB0.
	string port;

	// @var baud
	//		information of Lidar baudrate, default baud is 128000 for X4.
	int baud;

	// @var intensities
	//		information of Lidar intensities, default baud is 0 for X4.
	int intensities;

	std::vector<float> meanOfRanges;
	std::vector<float> prevRanges;
	int count;
	bool countComplete;

private:
	// @method	Stop
	// @param 	signo
	// 			Callback funciton for ctrl+C signal.
	static void Stop(int signo){
		printf("[Lidar.h] Received exit signal\n");
		running = true;
	}

public:
	/*---------------------------------------------
	  YDLIDAR C++ Parameters
	  Params: <serial_port> <baudrate> <intensities>
	  Example: /dev/ttyUSB0 128000 0 
	---------------------------------------------*/
	Lidar(string p="/dev/ttyUSB0", int b=128000, int i=0) :
		port(p), baud(b), intensities(i)
	{
		running = false;
		count = 0; 
		countComplete = false;

		//signal(SIGINT, Stop);
		//signal(SIGTERM, Stop);

		laser.setSerialPort(port);
		laser.setSerialBaudrate(baud);
		laser.setIntensities(intensities);

		std::cout << "[Lidar.h] Port: " << port << std::endl;
		std::cout << "[Lidar.h] baud: " << baud << std::endl;
		std::cout << "[Lidar.h] intensities: " << intensities << std::endl;
		laser.initialize();
	}

public:
	// @method	run
	//			When Thread runs, then execute this function.
	virtual void run();

	// @method	stop
	//			If you want stop the thread prommatically, call this function.
	virtual void stop();

	// @method	getRanges
	// @return	range information from Lidar, it contains 720 elements of distance.
	//std::vector<float> getRanges(){ return meanOfRanges; }
	float* getRanges(){ return &meanOfRanges[0]; }

	void setCountComplete(bool b){ countComplete = b; }
	bool getCountComplete(){ return countComplete; }

	bool getRunningStatus(){ return running;}
};


