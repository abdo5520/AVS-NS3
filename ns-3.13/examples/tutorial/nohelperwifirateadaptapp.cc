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
#include "ns3/propagation-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("nohelperwifirateadaptappExample");

UniformVariable x(0.0,10.0);
EventId controlEventID;


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
	std::cout << "localRandom--"  << localRandom << "NowTime--" << Simulator::Now() << std::endl;

//	Time now= Simulator::Now();
	if (localRandom <= 0)
	check = pToServer->AdaptControl(-1);
	else
		check = pToServer->AdaptControl(1);

	std::cout << "Level Changed     " << check << std::endl;
	controlEventID = Simulator::Schedule(MilliSeconds(step), Controlling,pToServer, step);


}




/**

static void
RxDrop (Ptr<const Packet> p)
{
	static int n_droppedPackets=0;
	static Time t1 = Seconds(0.0);
	n_droppedPackets++;
	int n_droppedPacketsPerSecond;
	double average_droppedPackets;
	t1 = t1 + Simulator::Now();
	if (t1 > 1.0)
	{t1 = 0.0;
	average_droppedPackets = n_droppedPackets / Seconds
		}

  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}


*/



int
main (int argc, char *argv[])
{

        cout << "// Default Network Topology Without Extra Parameters           \n";
        cout << "//                                                             \n";    
        cout << "//   Wifi 10.1.3.0  Port#= (port)                              \n";
        cout << "//        (Dest.)  AP                                          \n";    
        cout << "//            *    *                                           \n";    
        cout << "//  |    |    |    |    10.1.1.0                               \n";    
        cout << "//            n1  n0 -------------- n1 (Source)                \n";    
        cout << "//                   point-to-point  |                         \n";    
        cout << "//                                   ================          \n";    
        cout << "//                                     LAN 10.1.2.0            \n"; 
   

  bool verbose = true;
  uint16_t videoLevel = 0;
  double distance = 150.0;
  NS_LOG_INFO ("Installing static mobility; distance " << distance << " .");

//  uint64_t adaptationStep = 1000;

//  uint32_t nCsma = 3;
  uint32_t nWifi = 1;         // Only one WifiStaNode attached to the Access Point
  uint32_t np2pNodes = 2;
  uint16_t nLevels = 3;
  std::string inputFileName("JurassicPark");
  std::string outputFileName("nohelperwifirateAdaptOut1");

// comment unused variables because warnings are treated as errors      
//  uint32_t serverPortNumber= 4000;
  
  CommandLine cmd;

  cmd.AddValue ("inputFileName", "Trace Input File Name", inputFileName);
  cmd.AddValue ("outputFileName", "Trace Output File Name", outputFileName);
  cmd.AddValue ("nLevels", "Number of Video Levels", nLevels);

//  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
//  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
//  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
	//  LogComponentEnable ("RateAdaptiveSender", LOG_LEVEL_INFO);
	//  LogComponentEnable ("RateAdaptiveReceiver", LOG_LEVEL_INFO
  //  LogComponentEnableAll(LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (np2pNodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  // find out later what is the channel delay concept from the model point of view
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
// one of these nodes is the source, and the second one is the Access Point in the WiFi side
  p2pDevices = pointToPoint.Install (p2pNodes);   

// Error Model added into the P2P Connection
// Generating the rate error model
  /**
  Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> (
      "RanVar", RandomVariableValue (UniformVariable (0., 1.)),
      "ErrorRate", DoubleValue (0.001));
//   Attach the rate error model to the receiver ( client side)
  p2pDevices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  p2pDevices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

*/



  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);    // nWifi is the number of WiFi Station nodes entered by the user or set as default

  // set one of the p2p Nodes to become the WiFi Access Point (AP) Node
  NodeContainer wifiApNode;
  // WifiApNode is the p2pNode[1]
  wifiApNode.Add(p2pNodes.Get(1));
// Setup Wifi Channel and MAC layers
/**  Ptr<MobilityModel> mobilitySta = CreateObject<ConstantPositionMobilityModel> ();
  mobilitySta->SetPosition (Vector (50.0, 0.0, 0.0));
  Ptr<MobilityModel> mobilityAp = CreateObject<ConstantPositionMobilityModel> ();
  mobilityAp->SetPosition (Vector (0.0, 0.0, 0.0));
*/
  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());
  Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
  channel->SetPropagationLossModel (log);

  Ptr<YansWifiPhy> phyAp = CreateObject<YansWifiPhy> ();
  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();
  phyAp->SetErrorRateModel (error);
  phyAp->SetChannel (channel);
/**  phyAp->SetMobility (mobilityAp);

  Ptr<YansWifiPhy> phySta = CreateObject<YansWifiPhy> ();
  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();
  phySta->SetErrorRateModel (error);
  phySta->SetChannel (channel);
  phySta->SetMobility (mobilityAp);
*/


  WifiHelper wifi = WifiHelper::Default ();
 // wifi.EnableLogComponents();
// The Defult value is "ns3::ArfWifiManager" , this is why we change it below to Constant Rate
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager");
/** The Default is (  helper.SetType ("ns3::AdhocWifiMac",
 "QosSupported", BooleanValue (false));) , this is why we use mac.setType later below
 to change it to StaWifiMac
*/
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
// Later we should try the same run using the QosWifiMacHelper
//  QosWifiMacHelper mac = QosWifiMacHelper::Default ();
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices;
  WifiPhyHelper phyHelper;
  //phyHelper.Create()
  staDevices = wifi.Install(phyAp, mac, wifiStaNodes);

// Reusing the  NqosWifiMacHelper mac with different values to set the Access Point Node too 
// after setting up the Station Node
// Note that we reuse the same WifiPhy Phy as it is , without modifications.
// It makes sense since both devices share the same Physical channel
// Notice that we assign the Access-Point to the same SSID as the Station Node(s)
  mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phyAp, mac, wifiApNode);

  // Mobility model is required for any Wifi Node in order for the model to work 
 // correctly , if we want stationary nodes, then we install constant position Mobility Model

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (distance, 0.0, 0.0));
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);
  mobility.Install (wifiApNode);



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
// No more csmaNodes in use
//  stack.Install (csmaNodes);
  stack.Install(p2pNodes);
 // stack.Install(wifiApNode);
// TODO find out how the stack is added to the Access-point Node ?! 
// Although it is copied from the example model but it should not have L4 and above !!
//  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
/**
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);
*/
  address.SetBase ("10.1.3.0", "255.255.255.0"); 
// Added by Abdallah Abdallah
  Ipv4InterfaceContainer wifiStaInterfaces;
  Ipv4InterfaceContainer wifiApInterface;
  wifiApInterface	= address.Assign (apDevices);
  wifiStaInterfaces = address.Assign (staDevices);





  // In this simulation we will run the rate adaptive sender on the p2p node
  // Then we install a regular udpServer application on the Wifi node
  // as a receiver

//In the two lines below I modify the Receiver to make it Echo server
// We will use this to send the rate adaptation feedback later
  uint16_t port = 5555;
// UdpServerHelper server (port);
  RateAdaptiveReceiverHelper videoClient (port);
//  ApplicationContainer apps = server.Install (wifiStaNodes.Get (nWifi - 1));
  ApplicationContainer appsClient = videoClient.Install (wifiStaNodes.Get (nWifi - 1));

  appsClient.Start (Seconds (0.0));
  appsClient.Stop (Seconds (12.001));


  uint32_t MaxPacketSize = 1400-28; // IP Header (20) + UDP Header (8) = 28
  // set the remote address and remote port
  RateAdaptiveSenderHelper videoServer (wifiStaInterfaces.GetAddress(0), port,"");
  // Default maximum Packet Size defined in the model is already 1400 Bytes
  videoServer.SetAttribute ("MaxPacketSize", UintegerValue (MaxPacketSize));
  videoServer.SetAttribute ("nVideoLevels", UintegerValue (nLevels));
  videoServer.SetAttribute("TraceFilename", StringValue(inputFileName));
  videoServer.SetAttribute("OutputTraceFilename", StringValue(outputFileName));

  std::cout << outputFileName << std::endl;
  ApplicationContainer appsServer;
  appsServer = videoServer.Install (p2pNodes.Get (0));

  // Create a pointer to the Video Server object and acquire the address of the server instance
  RateAdaptiveSender *ppServer;
  ppServer= (RateAdaptiveSender *)GetPointer(appsServer.Get(0));

  // set the default Videl Quality Level initialized by the Server
  // For MPEG Low =0 , Midium = 1, High = 2
  // For H.263 16k = 0, 64k = 1 , 256k = 2, VBR = 3;
  ppServer->SetCurrentLevel(videoLevel);

  appsServer.Start (Seconds (1.0));
  appsServer.Stop (Seconds (12.001));





  // Schedule the Control Algorithm Callback function
 // Simulator::Schedule(Seconds(2.0), Controlling,ppServer,adaptationStep);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (12.001));
// The string passed is a Filename prefix to use when creating ascii trace files
  pointToPoint.EnablePcapAll ("nohelperwifirateadaptapp"+inputFileName);
//  phy.EnablePcap ("wifirateadaptappSource"+inputFileName, p2pDevices.Get (0));
//  phy.EnablePcap ("wifirateadaptappGateway"+inputFileName, p2pDevices.Get (1));
  //EnablePcap ("nohelperwifirateadaptappAP"+inputFileName, apDevices.Get (0));


  Simulator::Run ();
  Simulator::Destroy ();
  return 0;


}

