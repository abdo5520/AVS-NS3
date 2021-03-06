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
#include "ns3/stats-module.h"
//#include "backgroundcontrol.h"

// 			Default Network Topology
//		-----------------------------
//     10.1.1.0(Client)
//     				n0 -------------- n1
//                   					Point-to-Point (Source/Server)
//                                      LAN 10.1.1.1
//


using namespace ns3;
//using namespace std;

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



/**
  * \brief Implements the Adaptive Streaming Logic by making the decision then passing the target video level to the
  * the AdaptiveVideoServer instance by calling the member function (AdaptControl)
  * \param PToServer regular C++ pointer to the RateAdaptiveSender instance
  * \param step the periodic time step in (millisecond) to reschedule the Controlling Logic
  * \return void return value
 */
void Controlling(RateAdaptiveSender *pToServer, uint64_t step)
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
	controlEventID = Simulator::Schedule(MilliSeconds(step), Controlling,pToServer, step);


}



static void
RxDrop (Ptr<const Packet> p)
{
	static int n_droppedPackets=0;
	static Time t1 = Seconds(0.0);
	n_droppedPackets++;
/**	int n_droppedPacketsPerSecond;
	double average_droppedPackets;
	t1 = t1 + Simulator::Now();
	if (t1 > 1.0)
	{t1 = 0.0;
	average_droppedPackets = n_droppedPackets / Seconds
		}
*/

  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}

void TxCallback (Ptr<CounterCalculator<uint32_t> > datac,
                 std::string path, Ptr<const Packet> packet) {
  NS_LOG_INFO ("Sent frame counted in " <<
               datac->GetKey ());
  datac->Update ();
  // end TxCallback
}


int
main (int argc, char *argv[])
{

        cout << "// Default Network Topology Without Extra Parameters                \n";
        cout << "//    10.1.1.0 (Client)                           					 \n";
        cout << "//          n0 -------------- n1 (Source/Server)      	             \n";
        cout << "//                   			point-to-point                       \n";
        cout << "//                          LAN 10.1.1.1      		    			 \n";
   

        string format ("omnet");
        string experiment ("wifi-distance-test");
        string strategy ("wifi-default");
        string input;
        string runID;


  bool verbose = true;
  // Random Generator Related Code
  SeedManager::SetSeed (3);  // Changes seed from default of 1 to 3
  SeedManager::SetRun (7);  // Changes run number from default of 1 to 7
  // Now, create random variables


  uint16_t videoLevel = 2;
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


  if (verbose)
    {
//	  LogComponentEnable ("RateAdaptiveSender", LOG_LEVEL_INFO);
//	  LogComponentEnable ("RateAdaptiveReceiver", LOG_LEVEL_INFO);
 //    LogComponentEnableAll(LOG_LEVEL_INFO);
    }



  NodeContainer p2pNodes;
  p2pNodes.Create (np2pNodes);


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  // Generating the rate error model
//  Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> (
//      "RanVar", RandomVariableValue (UniformVariable (0., 1.)),
//      "ErrorRate", DoubleValue (0.001));
//   Attach the rate error model to the receiver ( client side)
//  p2pDevices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  p2pDevices.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

//  p2pDevices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
//  p2pDevices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));




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
// Setup the stack , connect the nodes and devices and install the applications
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
    appsClient.Stop (Seconds (12.001));
    // This needs to be changed once we move to using the TCP protocol
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
    // set the default Videl Quality Level initialized by the Server
    // For MPEG Low =0 , Midium = 1, High = 2
    // For H.263 16k = 0, 64k = 1 , 256k = 2, VBR = 3;
    ppServer->SetCurrentLevel(videoLevel);
    appsServer.Start (Seconds (1.0));
    appsServer.Stop (Seconds (12.001));


    // Schedule the Control Algorithm Callback function
//    Simulator::Schedule(Seconds(2.0), Controlling,ppServer,1000);
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Simulator::Stop (Seconds (12.001));
  // The string passed is a Filename prefix to use when creating ascii trace files
    pointToPoint.EnablePcapAll ("p2prateadaptapp");
  //  csma.EnablePcap ("third", csmaDevices.Get (0), true);

    // Statistic and Data collection
    DataCollector data;
    data.DescribeRun (experiment, strategy, input, runID);
    data.AddMetadata ("author", "Abdallah S. Abdallah");
    Ptr<CounterCalculator<uint32_t> > totalTx =
      CreateObject<CounterCalculator<uint32_t> >();
    totalTx->SetKey ("Videoserver-Total-Transmitted-MacFrames");
    totalTx->SetContext ("node[1]");
    Config::Connect ("/NodeList/1/DeviceList/*/$ns3::PointToPointNetDevice/Mac/MacTx",
                     MakeBoundCallback (&TxCallback, totalTx));
    data.AddDataCalculator (totalTx);


    // Total received MAC frames on the receiver side
    Ptr<PacketCounterCalculator> totalRx = CreateObject<PacketCounterCalculator>();
    totalRx->SetKey ("VideoClient-Total-Received-MacFrames");
    totalRx->SetContext ("P2PNodes[0]");
    Config::Connect ("/NodeList/0/DeviceList/*/$ns3::PointToPointNetDevice/Mac/MacRx",
    		MakeCallback (&PacketCounterCalculator::PacketUpdate, totalRx));
    data.AddDataCalculator (totalRx);

// Total sent transport Application Layer's packet on the Server side
    Ptr<PacketCounterCalculator> appTx =
      CreateObject<PacketCounterCalculator>();
    appTx->SetKey ("sender-tx-packets");
    appTx->SetContext ("P2PNodes[1]");
//    Config::Connect ("/NodeList/1/ApplicationList/*/$RateAdaptiveSender/Tx",
 //                    MakeCallback (&PacketCounterCalculator::PacketUpdate,appTx));
//    data.AddDataCalculator (appTx);


// Total received transport Application Layer's packet on the Server side


    Simulator::Run ();
    Simulator::Destroy ();
    return 0;

}

