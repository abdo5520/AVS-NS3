/*
 * backgroundcontrol.h
 *
 *  Created on: Jul 31, 2012
 *      Author: abdo
 */

#ifndef BACKGROUNDCONTROL_H_
#define BACKGROUNDCONTROL_H_
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

namespace ns3 {



class BackgoundControl {

	EventId controlEventID;
	Ptr<RateAdaptiveSender> pToServer;
	Ptr<RateAdaptiveSender> pToClient;
public:
	void BackgroundControl();
	~BackgroundControl();
	void BackgroundControl(Ptr<RateAdaptiveSender> pS, Ptr<RateAdaptiveReceiver> pC);
	void Controlling();



};


} // namespace ns3



#endif /* BACKGROUNDCONTROL_H_ */
