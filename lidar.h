#pragma once

#include"CYdLidar.h"
#include"stoppableThread.h"
#include<iostream>
#include<string>
#include<signal.h>
#include<memory>
#include<unistd.h>

using namespace std;
using namespace ydlidar;

class Lidar : public Stoppable{
	CYdLidar laser;
	LaserScan scan;
	static bool running;

	string port;
	int baud;
	int intensities;

private:
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

		signal(SIGINT, Stop);
		signal(SIGTERM, Stop);

		laser.setSerialPort(port);
		laser.setSerialBaudrate(baud);
		laser.setIntensities(intensities);

		std::cout << "[Lidar.h] Port: " << port << std::endl;
		std::cout << "[Lidar.h] baud: " << baud << std::endl;
		std::cout << "[Lidar.h] intensities: " << intensities << std::endl;
		laser.initialize();
	}

public:
	virtual void run();
	virtual void stop();

	std::vector<float> getRanges(){ return scan.ranges; }
};


