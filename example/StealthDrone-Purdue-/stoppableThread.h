#pragma once

/*
*	(Pure virtual class)
*	This class for Stoppable thread.
*	inherited this class and implement run function.
*	When you want to stop thread, call the stop function of this class.
*
*	ref) https://thispointer.com/c11-how-to-stop-or-terminate-a-thread/
*/



#include<iostream>
#include<future>
#include<thread>
#include<chrono>
#include<assert.h>

class Stoppable
{
	std::promise<void> exitSignal;
	std::future<void> futureObj;

public:
	Stoppable() :
		futureObj(exitSignal.get_future())
	{
	}

	Stoppable(Stoppable && obj) :
		exitSignal(std::move(obj.exitSignal)), futureObj(std::move(obj.futureObj))
	{
		//std::cout << "Move Constructor is called" << std::endl;
	}

	Stoppable & operator=(Stoppable && obj)
	{
		//std::cout << "Move Assignment is called" << std::endl;
		exitSignal = std::move(obj.exitSignal);
		futureObj = std::move(obj.futureObj);
		return *this;
	}

public:
	virtual void run() = 0;

	void operator()(){
		run();
	}

	bool stopRequested(){
		if( futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout )
			return false;
		return true;
	}

	virtual void stop(){
		std::cout << "[StoppableThread.h] Stopping Thread" << std::endl;
		exitSignal.set_value();
	}
};
