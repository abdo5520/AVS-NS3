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

NS_LOG_COMPONENT_DEFINE ("bwestimationExample");

int
main (int argc, char *argv[])
{

        cout << "// Default Network Topology Without Extra Parameters           \n";
        cout << "//                                                             \n";    
        cout << "//   Wifi 10.1.3.0                                             \n";    
        cout << "//        (Dest.)  AP                                          \n";    
        cout << "//            *    *                                           \n";    
        cout << "//  |    |    |    |    10.1.1.0                               \n";    
        cout << "//            n1  n0 -------------- n1 (Source)                \n";    
        cout << "//                   point-to-point  |                         \n";    
        cout << "//                                   ================          \n";    
        cout << "//                                     LAN 10.1.2.0            \n"; 
   

  bool verbose = true;
  
//  uint32_t nCsma = 3;
  uint32_t nWifi = 1;           // Only one WifiStaNode attached to the Access Point
  uint32_t np2pNodes = 2;   
        
  uint32_t serverPortNumber= 4000;
  


  CommandLine cmd;
//  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
//  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
//  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
      LogComponentEnableAll(LOG_LEVEL_INFO);
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (np2pNodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  // find out later what is the channel delay concept from the model point of view
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));  

  NetDeviceContainer p2pDevices;
// one of these nodes is the source, and the second one is the Access Point in the WiFi side
  p2pDevices = pointToPoint.Install (p2pNodes);   

// No csma nodes in this simulation , maybe later
/**
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);    // nCsma is the number of CSMA nodes entered by the user or set as default

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));    // 6560 nanoSec = 6.56 microSec

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
*/
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);    // nWifi is the number of WiFi nodes entered by the user or set as default
  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi = WifiHelper::Default ();
//  wifi.EnableLogComponents();

// The Default value is "ns3::ArfWifiManager" , this is why we change it below
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
// The Default is (  helper.SetType ("ns3::AdhocWifiMac",
//  "QosSupported", BooleanValue (false));) , this is why we use mac.setType later below

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
// Later we should try the same run using the QosWifiMacHelper
//  QosWifiMacHelper mac = QosWifiMacHelper::Default ();


  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

// Reusing the  NqosWifiMacHelper mac with different values to set the Access Point Node too 
// after setting up the Station Node
// Note that we reuse the same WifiPhy Phy as it is , without modifications.
// It makes sense since both devices share the same Physical channel
// Notice that we assign the Access-Point to the same SSID as the Station Node(s)
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  // Mobility model is required for any Wifi Node in order for the model to work 
 // correctly , if we want stationary nodes, then we install constant position Mobility Model

  MobilityHelper mobility;
/**
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
*/
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);
// fixed position mobility model for the Access Point
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
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
// Added by Abdallah 
  Ipv4InterfaceContainer wifiStaInterfaces;     
  wifiStaInterfaces = address.Assign (staDevices);
  address.Assign (apDevices);

  UdpEchoServerHelper echoServer (serverPortNumber);

// Install the server on the source node
//   ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  ApplicationContainer serverApps = echoServer.Install (p2pNodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (20.0));

 // UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
// Connect to the remote address of the server
  UdpEchoClientHelper echoClient (p2pInterfaces.GetAddress(1)  ,serverPortNumber );

  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps =  echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (20.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (25.0));
// The string passed is a Filename prefix to use when creating ascii trace files
  pointToPoint.EnablePcapAll ("bwestimation");
  phy.EnablePcap ("bwestimation", apDevices.Get (0));
  phy.EnablePcap ("bwestimation", staDevices.Get(0));

//  csma.EnablePcap ("third", csmaDevices.Get (0), true);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;


}
