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
inline void action_error_exit(ActionResult result, const std::string &message)
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

/**
 * Does Offboard control using NED co-ordinates.
 *
 * returns true if everything went well in Offboard control, exits with a log otherwise.
 */
bool offb_ctrl_ned(std::shared_ptr<dronecode_sdk::Offboard> offboard)
{
    const std::string offb_mode = "NED";
    // Send it once before starting offboard, otherwise it will be rejected.
    offboard->set_velocity_ned({0.0f, 0.0f, 0.0f, 0.0f});

    Offboard::Result offboard_result = offboard->start();
    offboard_error_exit(offboard_result, "Offboard start failed");
    offboard_log(offb_mode, "Offboard started");

    offboard_log(offb_mode, "Turn to face East");
    offboard->set_velocity_ned({0.0f, 0.0f, 0.0f, 90.0f});
    sleep_for(seconds(1)); // Let yaw settle.

    {
        const float step_size = 0.01f;
        const float one_cycle = 2.0f * (float)M_PI;
        const unsigned steps = 2 * unsigned(one_cycle / step_size);

        offboard_log(offb_mode, "Go North and back South");
        for (unsigned i = 0; i < steps; ++i) {
            float vx = 5.0f * sinf(i * step_size);
            offboard->set_velocity_ned({vx, 0.0f, 0.0f, 90.0f});
            sleep_for(milliseconds(10));
        }
    }

    offboard_log(offb_mode, "Turn to face West");
    offboard->set_velocity_ned({0.0f, 0.0f, 0.0f, 270.0f});
    sleep_for(seconds(2));

    offboard_log(offb_mode, "Go up 2 m/s, turn to face South");
    offboard->set_velocity_ned({0.0f, 0.0f, -2.0f, 180.0f});
    sleep_for(seconds(4));

    offboard_log(offb_mode, "Go down 1 m/s, turn to face North");
    offboard->set_velocity_ned({0.0f, 0.0f, 1.0f, 0.0f});
    sleep_for(seconds(4));

    // Now, stop offboard mode.
    offboard_result = offboard->stop();
    offboard_error_exit(offboard_result, "Offboard stop failed: ");
    offboard_log(offb_mode, "Offboard stopped");

    return true;
}

/**
 * Does Offboard control using body co-ordinates.
 *
 * returns true if everything went well in Offboard control, exits with a log otherwise.
 */
bool offb_ctrl_body(std::shared_ptr<dronecode_sdk::Offboard> offboard)
{
    const std::string offb_mode = "BODY";

    // Send it once before starting offboard, otherwise it will be rejected.
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});

    Offboard::Result offboard_result = offboard->start();
    offboard_error_exit(offboard_result, "Offboard start failed: ");
    offboard_log(offb_mode, "Offboard started");

    offboard_log(offb_mode, "Turn clock-wise and climb");
    offboard->set_velocity_body({0.0f, 0.0f, -1.0f, 60.0f});
    sleep_for(seconds(5));

    offboard_log(offb_mode, "Turn back anti-clockwise");
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, -60.0f});
    sleep_for(seconds(5));

    offboard_log(offb_mode, "Wait for a bit");
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
    sleep_for(seconds(2));

    offboard_log(offb_mode, "Fly a circle");
    offboard->set_velocity_body({5.0f, 0.0f, 0.0f, 30.0f});
    sleep_for(seconds(15));

    offboard_log(offb_mode, "Wait for a bit");
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
    sleep_for(seconds(5));

    offboard_log(offb_mode, "Fly a circle sideways");
    offboard->set_velocity_body({0.0f, -5.0f, 0.0f, 30.0f});
    sleep_for(seconds(15));

    offboard_log(offb_mode, "Wait for a bit");
    offboard->set_velocity_body({0.0f, 0.0f, 0.0f, 0.0f});
    sleep_for(seconds(8));

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


	//--------------------------------------------------------------------------
	

	std::cout << "Arming..." << std::endl;
    const ActionResult arm_result = action->arm();
    action_error_exit(arm_result, "Arming failed");
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
    action_error_exit(takeoff_result, "Takeoff failed");
    std::cout << "In Air..." << std::endl;
    sleep_for(seconds(10));

	{
		std::cout << "Starting mission." << std::endl;
		auto prom = std::make_shared<std::promise<Mission::Result>>();
		auto future_result = prom->get_future();
		mission->start_mission_async([prom](Mission::Result result) {
				prom->set_value(result);
				std::cout << "Started mission." << std::endl;
				});

		const Mission::Result result = future_result.get();
		handle_mission_err_exit(result, "Mission start failed: ");
	}

	while(!want_to_pause){
		sleep_for(seconds(1));
	}

	{
		auto prom = std::make_shared<std::promise<Mission::Result>>();
		auto future_result = prom->get_future();

		std::cout << "Pausing mission..." << std::endl;
		mission->pause_mission_async([prom](Mission::Result result){ 
				prom->set_value(result);
				});

		const Mission::Result result = future_result.get();
		if(result != Mission::Result::SUCCESS) {
			std::cout << "Failed to pause mission (" << Mission::result_str(result) << ")" << std::endl;
		} else {
			std::cout << "Mission paused." << std::endl;
		}
	}

	sleep_for(seconds(5));

    bool ret = offb_ctrl_ned(offboard);
    if (ret == false) {
        return EXIT_FAILURE;
    }

	{
		auto prom = std::make_shared<std::promise<Mission::Result>>();
		auto future_result = prom->get_future();

		std::cout << "Resuming mission..." << std::endl;
		mission->start_mission_async([prom](Mission::Result result){
				prom->set_value(result);
				});

		const Mission::Result result = future_result.get();
		if( result != Mission::Result::SUCCESS) {
			std::cout << "Failed to resume mission (" << Mission::result_str(result) << ")" << std::endl;
		} else {
			std::cout << "Resumed mission." << std::endl;
		}
	}


	while (!mission->mission_finished()){
		sleep_for(seconds(1));
	}

	{
		std::cout << "Commanding RTL..." << std::endl;
		const ActionResult result = action->return_to_launch();
		if( result != ActionResult::SUCCESS) {
			std::cout << "Failed to command RTL (" << action_result_str(result) << ")" << std::endl;
		} else {
			std::cout << "Commanded RTL." << std::endl;
		}
	}

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

    return EXIT_SUCCESS;
}
