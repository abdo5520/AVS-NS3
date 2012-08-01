/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "rate-adaptive-receiver.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RateAdaptiveReceiver");
NS_OBJECT_ENSURE_REGISTERED (RateAdaptiveReceiver);

TypeId
RateAdaptiveReceiver::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RateAdaptiveReceiver")
    .SetParent<Application> ()
    .AddConstructor<RateAdaptiveReceiver> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&RateAdaptiveReceiver::m_port),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

RateAdaptiveReceiver::RateAdaptiveReceiver ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

RateAdaptiveReceiver::~RateAdaptiveReceiver()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

void
RateAdaptiveReceiver::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void 
RateAdaptiveReceiver::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&RateAdaptiveReceiver::HandleRead, this));
}

void 
RateAdaptiveReceiver::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void 
RateAdaptiveReceiver::HandleRead (Ptr<Socket> socket)
{
	/** Modified by Abdallah Abdallah to send an AckPacket
	 * of 10 Bytes instead of echo the entire frame
	 */


  Ptr<Packet> ackPacket;
 Ptr<Packet> packet;
  uint32_t ackSize;
  ackSize = 10;
  ackPacket = Create<Packet> (ackSize);
  Address from;
  while (packet = socket->RecvFrom (from))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("Received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 ());

          packet->RemoveAllPacketTags ();
          packet->RemoveAllByteTags ();

          NS_LOG_LOGIC ("Echoing packet");
          NS_ASSERT(socket->SendTo (ackPacket, 0, from) != -1);

        }
    } //end of while loop
}//end of function

} // Namespace ns3
