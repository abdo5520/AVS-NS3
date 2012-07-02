/*
 * synch.cc
 *
 *  Created on: Nov 19, 2011
 *      Author: abdalah
 */

#include "synch.h"


namespace ns3{

namespace abdoSynch{


#define maxClock 10






	 synch::synch()
	{
		 myCounter = 0.0;
		 myStep = 1.0;
		timeCounter= Time::FromDouble(myCounter,Time::MS);
		timeStep = Time::FromDouble(myStep,Time::MS);
		m_stopTime = Time::FromDouble(10.0,Time::MS);

	}

	void synch::updateStatus()
	{

	}
	void synch::ScheduleSLS()
	{
		  cout<< "timeCounter = " <<timeCounter<<endl;
		  slsCycleEvent = Simulator::Schedule (timeCounter-Now(), &synch::fakeSLS,this);

	}


void synch::fakeSLS()
	{
	updateStatus();
	cout<< "One MilliSecond = One Frame" << "Time =" << Simulator::Now() <<endl;
	myCounter = myCounter + myStep;
	timeCounter = Time::FromDouble(myCounter,Time::MS);
	if (timeCounter.Compare(m_stopTime) )
		{
		ScheduleSLS();
		}
	}


} //namespace abdySynch
}// namespace ns3
