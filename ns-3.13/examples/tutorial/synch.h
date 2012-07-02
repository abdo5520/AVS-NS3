/*
 * synch.h
 *
 *  Created on: Nov 19, 2011
 *      Author: abdalah
 */

#ifndef SYNCH_H_
#define SYNCH_H_
#include <iostream>
#include "ns3/nstime.h"
#include "ns3/simulator.h"

using namespace std;

namespace ns3{

namespace abdoSynch{




class synch {
public:
	Time m_startTime;
	Time m_stopTime;
	EventId m_startEvent;
	EventId m_stopEvent;
	EventId slsCycleEvent;
	double myCounter;
	double myStep;
	Time timeCounter;
	Time timeStep;

	synch();

	void updateStatus();
	void ScheduleSLS();

void fakeSLS();

};

} //namespace abdosynch

} //namespace ns3


#endif /* SYNCH_H_ */
