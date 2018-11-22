/**
 * @file offboard_velocity.cpp
 * @brief Autonomous drone built from offboard_velocity, fly_mission with YDLidar.
 *
 * @authors Author: Julian Oes <julian@oes.ch>,
 *                  Shakthi Prashanth <shakthi.prashanth.m@intel.com>
 *					Daeun Choe
 *					Jiyeon Oh
 *					Chanhui yun
 * @date 2018-00-00
 */

#include <chrono>
#include <cmath>
#include <dronecode_sdk/action.h>
#include <dronecode_sdk/dronecode_sdk.h>
#include <dronecode_sdk/offboard.h>
#include <dronecode_sdk/telemetry.h>
#include <dronecode_sdk/mission.h>
#include <iostream>
#include <thread>
#include <future>
#include <memory>
#include "lidar.h"
#include "obstacleMap.h"

using namespace dronecode_sdk;
using namespace std::placeholders;
using namespace std::chrono;
using namespace std::this_thread;

//using std::this_thread::sleep_for;
//using std::chrono::milliseconds;
//using std::chrono::seconds;

#define ERROR_CONSOLE_TEXT "\033[31m" // Turn text on console red
#define TELEMETRY_CONSOLE_TEXT "\033[34m" // Turn text on console blue
#define NORMAL_CONSOLE_TEXT "\033[0m" // Restore normal console colour

// Handles Action's result
inline void handle_action_err_exit(ActionResult result, const std::string &message)
{
    if (result != ActionResult::SUCCESS) {
        std::cerr << ERROR_CONSOLE_TEXT << message << action_result_str(result)
                  << NORMAL_CONSOLE_TEXT << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Handles Offboard's result
inline void offboard_error_exit(Offboard::Result result, const std::string &message)
{
    if (result != Offboard::Result::SUCCESS) {
        std::cerr << ERROR_CONSOLE_TEXT << message << Offboard::result_str(result)
                  << NORMAL_CONSOLE_TEXT << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Handles Mission's result
inline void handle_mission_err_exit(Mission::Result result, const std::string &message)
{
    if (result != Mission::Result::SUCCESS) {
        std::cerr << ERROR_CONSOLE_TEXT << message << Mission::result_str(result)
                  << NORMAL_CONSOLE_TEXT << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Handles connection result
inline void connection_error_exit(ConnectionResult result, const std::string &message)
{
    if (result != ConnectionResult::SUCCESS) {
        std::cerr << ERROR_CONSOLE_TEXT << message << connection_result_str(result)
                  << NORMAL_CONSOLE_TEXT << std::endl;
        exit(EXIT_FAILURE);
    }
}

// MAKE_MISSION_ITEM
std::shared_ptr<MissionItem> make_mission_item(double latitude_deg, double longitude_deg,
												float relative_altitude_m, float speed_m_s,
												bool is_fly_through, float gimbal_pitch_deg,
												float gimbal_yaw_deg, MissionItem::CameraAction camera_action)
{
	std::shared_ptr<MissionItem> new_item(new MissionItem());
	new_item->set_position(latitude_deg, longitude_deg);
	new_item->set_relative_altitude(relative_altitude_m);
	new_item->set_speed(speed_m_s);
	new_item->set_fly_through(is_fly_through);
	new_item->set_gimbal_pitch_and_yaw(gimbal_pitch_deg, gimbal_yaw_deg);
	new_item->set_camera_action(camera_action);

	return new_item;
}

// Logs during Offboard control
inline void offboard_log(const std::string &offb_mode, const std::string msg)
{
    std::cout << "[" << offb_mode << "] " << msg << std::endl;
}

bool offb_ctrl_ned(std::shared_ptr<dronecode_sdk::Offboard> offboard, dronecode_sdk::Offboard::VelocityNEDYaw v)
{
	const std::string offb_mode = "NED";

    offboard->set_velocity_ned({0.0f, 0.0f, 0.0f, 0.0f});

    Offboard::Result offboard_result = offboard->start();
    offboard_error_exit(offboard_result, "Offboard start failed");
    offboard_log(offb_mode, "Offboard started");

	offboard->set_velocity_ned(v);
	sleep_for(seconds(2));

	offboard_result = offboard->stop();
	offboard_error_exit(offboard_result, "Offboard stop failed: ");
	offboard_log(offb_mode, "Offboard stopped");

	return true;
}

void usage(std::string bin_name)
{
    std::cout << NORMAL_CONSOLE_TEXT << "Usage : " << bin_name << " <connection_url>" << std::endl
              << "Connection URL format should be :" << std::endl
              << " For TCP : tcp://[server_host][:server_port]" << std::endl
              << " For UDP : udp://[bind_host][:bind_port]" << std::endl
              << " For Serial : serial:///path/to/serial/dev[:baudrate]" << std::endl
              << "For example, to connect to the simulator use URL: udp://:14540" << std::endl;
}

template <typename T>
void startMission(T &mission) {
	std::cout << "Starting/Resuming mission." << std::endl;
	auto prom = std::make_shared<std::promise<Mission::Result>>();
	auto future_result = prom->get_future();
	mission->start_mission_async([prom](Mission::Result result) {
			prom->set_value(result);
			//std::cout << "Started mission." << std::endl;
			});

	const Mission::Result result = future_result.get();
	handle_mission_err_exit(result, "Mission start failed: ");
	std::cout << "Mission started/resumed" << std::endl;
}

template <typename T>
void pauseMission(T &mission){
	auto prom = std::make_shared<std::promise<Mission::Result>>();
	auto future_result = prom->get_future();

	std::cout << "Pausing mission..." << std::endl;
	mission->pause_mission_async([prom](Mission::Result result){ 
			prom->set_value(result);
			});

	const Mission::Result result = future_result.get();
	handle_mission_err_exit(result, "Mission pause failed: ");
	std::cout << "Mission Paused" << std::endl;
}

template <typename T>
void commandRTL(T &action){
	std::cout << "Commanding RTL..." << std::endl;
	const ActionResult result = action->return_to_launch();
	handle_action_err_exit(result, "Failed to command RTL");
	std::cout << "Commanded RTL." << std::endl;
}

bool running = false;
void Stop(int signo){
	printf("[main.cpp] Received exit signal\n");
	running = true;
}



int main(int argc, char **argv)
{
	/*

	// Without constructor params, Lidar would have /dev/ttyUSB0 port, 128000 baudrate, no intensities check
	Lidar lidar;

	// Make thread to run Lidar.
	std::thread lidarThread([&](){
		lidar.run();
		});

	// We need some time for launching Lidar.
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	// When you get data from Lidar, call getRanges from lidar instance.
	std::vector<float> data = lidar.getRanges();
	for(int i=0; i<(unsigned int)data.size(); i++){
		std::cout << "[main.cpp] " << i << ": " << data[i] << std::endl;
	}
	
	lidar.stop();
	lidarThread.join();

	std::cout << "[main.cpp] Thread joined" << std::endl;


	return 0;

	*/

    DronecodeSDK dc;
	{
    	std::string connection_url;
    	ConnectionResult connection_result;

		auto prom = std::make_shared<std::promise<void>>();
		auto future_result = prom->get_future();

		std::cout << "Waiting to discover system..." << std::endl;
		dc.register_on_discover([prom](uint64_t uuid) {
				std::cout << "Discovered system with UUID:" << uuid << std::endl;
				prom->set_value();
				});


    	if (argc == 2) {
        	connection_url = argv[1];
        	connection_result = dc.add_any_connection(connection_url);
	    } else {
    	    usage(argv[0]);
        	return 1;
    	}

    	if (connection_result != ConnectionResult::SUCCESS) {
        	std::cout << ERROR_CONSOLE_TEXT
            	      << "Connection failed: " << connection_result_str(connection_result)
                	  << NORMAL_CONSOLE_TEXT << std::endl;
        	return 1;
    	}

		future_result.get();
	}

	dc.register_on_timeout([](uint64_t uuid){
			std::cout << "System with UUID timed out: " << uuid << std::endl;
			std::cout << "Exiting." << std::endl;
			exit(0);
			});

    // Wait for the system to connect via heartbeat
    while (!dc.is_connected()) {
        std::cout << "Wait for system to connect via heartbeat" << std::endl;
        sleep_for(seconds(1));
    }

	signal(SIGINT, Stop);
	signal(SIGTERM, Stop);

	Lidar lidar;
	ObstacleMap OM;

	std::thread lidarThread([&](){
		lidar.run();
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    // System got discovered.
    System &system = dc.system();
    auto action = std::make_shared<Action>(system);
    auto offboard = std::make_shared<Offboard>(system);
	auto mission = std::make_shared<Mission>(system);
    auto telemetry = std::make_shared<Telemetry>(system);

    while (!telemetry->health_all_ok()) {
        std::cout << "Waiting for system to be ready" << std::endl;
        sleep_for(seconds(1));
    }
    std::cout << "System is ready" << std::endl;
	std::cout << "Creating and uploading mission" << std::endl;


	//-----------------------Creating mission-----------------------------------
	std::vector<std::shared_ptr<MissionItem>> mission_items;

	mission_items.push_back( make_mission_item(47.398170327054473,
												8.5456490218639658,
												10.0f,
												5.0f,
												false,
												20.0f,
												60.0f,
												MissionItem::CameraAction::NONE) );

	mission_items.push_back( make_mission_item(47.398241338125118,
												8.5455360114574332,
												10.0f,
												2.0f,
												false,
												0.0f,
												-60.0f,
												MissionItem::CameraAction::NONE) );

	std::cout << "Uploading mission..." << std::endl;
	auto prom = std::make_shared<std::promise<Mission::Result>>();
	auto future_result = prom->get_future();
	mission->upload_mission_async(mission_items,
									[prom](Mission::Result result){
									prom->set_value(result);
									});

	const Mission::Result result = future_result.get();
	if( result != Mission::Result::SUCCESS ){
		std::cout << "Mission upload failed (" << Mission::result_str(result) << "), exiting." << std::endl;
		return 1;
	}
	std::cout << "Mission uploaded." << std::endl;
	//----------------------------------------------------------------------------
	

	std::cout << "Arming..." << std::endl;
    const ActionResult arm_result = action->arm();
    handle_action_err_exit(arm_result, "Arming failed");
    std::cout << "Armed" << std::endl;

	std::atomic<bool> want_to_pause{false};
	// Before starting the mission, we want to be sure to subscribe to the mission progress.
	mission->subscribe_progress([&want_to_pause](int current, int total){
			std::cout << "Mission status update: " << current << "/" << total << std::endl;
			if( current >= 1 ){
				// We can only set a flag here. If we do more request inside the callback,
				// We risk blocking the system.
				want_to_pause = true;
			}
		});

    const ActionResult takeoff_result = action->takeoff();
    handle_action_err_exit(takeoff_result, "Takeoff failed");
    std::cout << "In Air..." << std::endl;
    sleep_for(seconds(10));

	
	//startMission(mission);

	/*
	 * Checking for pause by busy waiting.  
	 * [TODO]
	 * When we use Lidar data, modified these three line. 
	 */
	while( !lidar.getRunningStatus() && !running ){
		//std::cout << "[INFO] " << telemetry->position() << std::endl;
		if( lidar.getCountComplete() == true ){
			OM.determineMapInfo(lidar.getRanges());
			int (*map)[5] = OM.getObstacleMap();

			/*for(int i=0; i<5; i++){
				for(int j=0; j<5; j++){
					printf("%d ", map[i][j]);
				}
				printf("\n");
			}
			printf("\n");*/

			/*
			 * Emergency Stop
			 */
			if( map[2][1] == 1 && map[1][1] == 1 && map[3][1] == 1 &&
					map[1][2] == 1 && map[3][2] == 1 ){
				running = true;
			}
			
			/*
			 * [TODO]
			 * Get the most nearest way from the destination.
			 * There are 5 options. (1,1), (2,1), (3,1), (1,2), (3,2)
			 */
			if(map[2][0] == 1){
				pauseMission(mission);
				if(map[1][1] == 0 && map[0][1] == 0){
					//offb_ctrl_ned(offboard, {1.0f, 1.0f, 0.0f, 45.0f});
					std::cout << "[LIDAR INFO]all clear move north-west" << std::endl;
				} else if( map[1][3] == 0 && map[0][3] == 0){
					//offb_ctrl_ned(offboard, {-1.0f, 1.0f, 0.0f, 135.0f});
					std::cout << "[LIDAR INFO]all clear move north-east" << std::endl;
				} else if( map[1][1] == 0 ){
					std::cout << "[LIDAR INFO]move north-west" << std::endl;
				} else if( map[1][3] == 0 ){
					std::cout << "[LIDAR INFO]move north-east" << std::endl;
				} else if( map[2][1] == 0 ){
					std::cout << "[LIDAR INFO]move west" << std::endl;
				} else if( map[2][3] == 0 ){
					std::cout << "[LIDAR INFO]move east" << std::endl;
				}
			}

			if(map[2][0] == 0 && map[2][1] == 0){
				startMission(mission);
			}

			lidar.setCountComplete(false);
		}
	}

	if( running ){
		lidar.stop();
		lidarThread.join();
		exit(0);
	}

	/*while(!want_to_pause){
		sleep_for(seconds(1));
	}*/

	//pauseMission(mission);
	

	//sleep_for(seconds(5));

    //  using local NED co-ordinates
    //bool ret = offb_ctrl_ned(offboard);
    //if (ret == false) {
       // return EXIT_FAILURE;
    //}

	//startMission(mission);

	while (!mission->mission_finished()){
		sleep_for(seconds(1));
	}

	commandRTL(action);

	sleep_for(seconds(2));

	while( telemetry->armed()){
		sleep_for(seconds(1));
	}
	std::cout << "Disarmed, exiting" << std::endl;


	//std::cout << "[INFO] Position: " << telemetry->position() << std::endl;

    //  using local NED co-ordinates
    //bool ret = offb_ctrl_ned(offboard);
    //if (ret == false) {
    //    return EXIT_FAILURE;
    //}

    //  using body co-ordinates
    //ret = offb_ctrl_body(offboard);
    //if (ret == false) {
    //    return EXIT_FAILURE;
    //}


    //const ActionResult land_result = action->land();
    //action_error_exit(land_result, "Landing failed");

    // We are relying on auto-disarming but let's keep watching the telemetry for a bit longer.
    //sleep_for(seconds(10));
    //std::cout << "Landed" << std::endl;

	lidar.stop();
	lidarThread.join();

    return EXIT_SUCCESS;
}
