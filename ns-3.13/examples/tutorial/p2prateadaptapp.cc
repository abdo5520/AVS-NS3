/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
//#include "backgroundcontrol.h"

// 			Default Network Topology
//		-----------------------------
//     10.1.1.0(Client)
//     				n0 -------------- n1
//                   					Point-to-Point (Source/Server)
//                                      LAN 10.1.1.1
//


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("p2prateadaptappExample");

UniformVariable x(0.0,10.0);
EventId controlEventID;

/**
void BackgoundControl::BackgroundControl(){


}
void BackgoundControl::BackgroundControl(Ptr<RateAdaptiveSender> pS, Ptr<RateAdaptiveReceiver> pC)
{

	this->pToClient = pC;
	this->pToServer = pS;
	controlEventID = Simulator::Schedule(MilliSeconds(0.0), &BackgoundControl::Controlling,this );


}


void BackgoundControl::Controlling()
{

	bool check;
	uint32_t localRandom = x.GetInteger(0,2);
	NS_ASSERT(controlEventID.IsExpired());


	check = pToServer->AdaptControl(localRandom);
	std::cout << "Level Changed     " << check << std::endl;
	controlEventID = Simulator::Schedule(MilliSeconds(999),  &BackgoundControl::Controlling,this);


}
*/



void Controlling(RateAdaptiveSender *pToServer)
{
	NS_ASSERT(controlEventID.IsExpired());
	bool check=true;

	uint32_t localRandom = x.GetValue(-1,1);
	std::cout << "localRandom--"  << localRandom << "NowTime--" << Simulator::Now()<< std::endl;

//	Time now= Simulator::Now();
	if (localRandom <= 0)
	check = pToServer->AdaptControl(-1);
	else
		check = pToServer->AdaptControl(1);
	std::cout << "Level Changed     " << check << std::endl;
	controlEventID = Simulator::Schedule(MilliSeconds(1000), Controlling,pToServer);


}

int
main (int argc, char *argv[])
{

        cout << "// Default Network Topology Without Extra Parameters                \n";
        cout << "//    10.1.1.0 (Client)                           					 \n";
        cout << "//          n0 -------------- n1 (Source/Server)      	             \n";
        cout << "//                   			point-to-point                       \n";
        cout << "//                          LAN 10.1.1.1      		    			 \n";
   

  bool verbose = true;
  // Random Generator Related Code
  SeedManager::SetSeed (3);  // Changes seed from default of 1 to 3
  SeedManager::SetRun (7);  // Changes run number from default of 1 to 7
  // Now, create random variables


  
  uint32_t np2pNodes = 2;
  uint16_t nLevels = 3;
  std::string inputFileName("JurassicPark");
  std::string outputFileName("p2pJurassicParkOutTimeToSend1");

// comment unused variables because warnings are treated as errors      
//  uint32_t serverPortNumber= 4000;
  
  CommandLine cmd;

  cmd.AddValue ("inputFileName", "Trace Input File Name", inputFileName);
  cmd.AddValue ("outputFileName", "Trace Output File Name", outputFileName);
  cmd.AddValue ("nLevels", "Number of Video Levels", nLevels);


  cmd.Parse (argc,argv);
  LogComponentEnable ("RateAdaptiveSender", LOG_LEVEL_INFO);
  LogComponentEnable ("RateAdaptiveReceiver", LOG_LEVEL_INFO);

  if (verbose)
    {
     LogComponentEnableAll(LOG_LEVEL_INFO);
    }



  NodeContainer p2pNodes;
  p2pNodes.Create (np2pNodes);


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  /**
   * This class aggregates instances of these objects, by default, to each node:
   *  - ns3::ArpL3Protocol
   *  - ns3::Ipv4L3Protocol
   *  - ns3::Icmpv4L4Protocol
   *  - ns3::UdpL4Protocol
   *  - a TCP based on the TCP factory provided
   *  - a PacketSocketFactory
   *  - Ipv4 routing (a list routing object and a static routing object)
  */

  InternetStackHelper stack;
  stack.Install (p2pNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  std::cout << "p2pDevices.Get" << p2pDevices.GetN() << std::endl;
  p2pInterfaces = address.Assign (p2pDevices);

  //In the two lines below I modify the Receiver to make it Echo server
  // We will use this to send the rate adaptation feedback later
    uint16_t port = 5555;

    RateAdaptiveReceiverHelper videoClient (port);
    ApplicationContainer appsClient = videoClient.Install (p2pNodes.Get (0));
    appsClient.Start (Seconds (0.0));
    appsClient.Stop (Seconds (11.001));

    uint32_t MaxPacketSize = 1400-28; // IP Header (20) + UDP Header (8) = 28



    // set the remote address and remote port
    Ptr<RateAdaptiveSender> pServer;
    RateAdaptiveSender *ppServer;

    RateAdaptiveSenderHelper videoServer (p2pInterfaces.GetAddress(0), port,"");
    // Default maximum Packet Size defined in the model is already 1400 Bytes
    videoServer.SetAttribute ("MaxPacketSize", UintegerValue (MaxPacketSize));
    videoServer.SetAttribute ("nVideoLevels", UintegerValue (nLevels));
    videoServer.SetAttribute("TraceFilename", StringValue(inputFileName));
    videoServer.SetAttribute("OutputTraceFilename", StringValue(outputFileName));
    std::cout << outputFileName << std::endl;
    ApplicationContainer appsServer = videoServer.Install (p2pNodes.Get (np2pNodes-1));
    ppServer= (RateAdaptiveSender *)GetPointer(appsServer.Get(0));
    appsServer.Start (Seconds (1.0));
    appsServer.Stop (Seconds (11.001));


    // Schedule the Control Algorithm Callback function
    Simulator::Schedule(Seconds(2.0), Controlling,ppServer);





    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Simulator::Stop (Seconds (11.001));
  // The string passed is a Filename prefix to use when creating ascii trace files
    pointToPoint.EnablePcapAll ("p2prateadaptapp");



  //  csma.EnablePcap ("third", csmaDevices.Get (0), true);

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;

}

