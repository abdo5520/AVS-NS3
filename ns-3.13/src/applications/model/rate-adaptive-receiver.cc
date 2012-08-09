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
#include "seq-ts-header.h"


#include "rate-adaptive-receiver.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RateAdaptiveReceiver");
NS_OBJECT_ENSURE_REGISTERED(RateAdaptiveReceiver);

TypeId RateAdaptiveReceiver::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::RateAdaptiveReceiver").SetParent<
			Application>().AddConstructor<RateAdaptiveReceiver>().AddAttribute(
			"Port", "Port on which we listen for incoming packets.",
			UintegerValue(9),
			MakeUintegerAccessor(&RateAdaptiveReceiver::m_port),
			MakeUintegerChecker<uint16_t>());
	return tid;
}

RateAdaptiveReceiver::RateAdaptiveReceiver() {
	NS_LOG_FUNCTION_NOARGS ();
	m_nReceivedPacket = 0;
	m_playBufCurrentSize = 0;
	m_initialPlayDelay = 3;	  // seconds to wait before scheduling the first display (PlayFromBuffer) event
	m_playRate=25;
}

RateAdaptiveReceiver::~RateAdaptiveReceiver() {
	NS_LOG_FUNCTION_NOARGS ();
	m_socket = 0;


}

void RateAdaptiveReceiver::DoDispose(void) {
	NS_LOG_FUNCTION_NOARGS ();
	Application::DoDispose();
}

void RateAdaptiveReceiver::SetDelayTracker(
		Ptr<TimeMinMaxAvgTotalCalculator> delay) {
	m_delay = delay;
	// end Receiver::SetDelayTracker
}
void RateAdaptiveReceiver::SetCounter(Ptr<CounterCalculator<> > calc) {
	m_calc = calc;
	// end Receiver::SetCounter
}

void RateAdaptiveReceiver::StartApplication(void) {
	NS_LOG_FUNCTION_NOARGS ();

	if (m_socket == 0) {
		TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
		m_socket = Socket::CreateSocket(GetNode(), tid);
		InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),
				m_port);
		m_socket->Bind(local);
		if (addressUtils::IsMulticast(m_local)) {
			Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket);
			if (udpSocket) {
				// equivalent to setsockopt (MCAST_JOIN_GROUP)
				udpSocket->MulticastJoinGroup(0, m_local);
			} else {
				NS_FATAL_ERROR("Error: joining multicast on a non-UDP socket");
			}
		}
	}

	m_socket->SetRecvCallback(
			MakeCallback(&RateAdaptiveReceiver::HandleRead, this));

	playEvent = Simulator::Schedule (Seconds (m_initialPlayDelay)+MicroSeconds(100) , &RateAdaptiveReceiver::PlayFromBuffer, this);

}

void RateAdaptiveReceiver::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();

	if (m_socket != 0) {
		m_socket->Close();
		m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
	}
}

void RateAdaptiveReceiver::SetInitialPlayDelay(uint16_t initDelay) {
	m_initialPlayDelay = initDelay;
}

uint16_t RateAdaptiveReceiver::PlayFromBuffer(void)
{

	if (m_playBufCurrentSize - m_playRate < 0){
		// trigger rebuffering log
		std::cout << "REBUFFERING.........REBUFFERING.........REBUFFERING.........REBUFFERING........." << std::endl;
		playEvent = Simulator::Schedule (MilliSeconds (1000), &RateAdaptiveReceiver::PlayFromBuffer, this);

		return(-1);
	}
	else {
		/** FIXME Data corruption risk if the member m_playBufCurrentSize got updated by
		 * another method at the same time since the receiverHandler already updates the same variable
		 * during run time
		 */
		m_playBufCurrentSize = m_playBufCurrentSize - m_playRate;

		playEvent = Simulator::Schedule (MilliSeconds (1000), &RateAdaptiveReceiver::PlayFromBuffer, this);
		return (m_playBufCurrentSize);

	}


}

void RateAdaptiveReceiver::HandleRead(Ptr<Socket> socket) {
	/** Modified by Abdallah Abdallah to send an AckPacket
	 * of 10 Bytes instead of echo the entire frame
	 */

	Ptr<Packet> ackPacket;
	Ptr<Packet> packet;
	SeqTsHeader receivedHeader;
	uint32_t ackSize;
	ackSize = 10;
	uint8_t bufferAck[10];

	Address from;
	while (packet = socket->RecvFrom(from)) {
		if (InetSocketAddress::IsMatchingType(from)) {
			NS_LOG_INFO ("Received " << packet->GetSize () << " bytes from " <<
											InetSocketAddress::ConvertFrom (from).GetIpv4 ());
									m_nReceivedPacket++;
									m_playBufCurrentSize++;
									packet->RemoveHeader(receivedHeader);
									std::cout << "Packet Received #  " << m_nReceivedPacket << "At Time  "
											<< Simulator::Now() << "  was sent at   "<< receivedHeader.GetTs() <<std::endl;
									std::cout << "SeqTsHeader Sequence Number  "<< receivedHeader.GetSeq() << std::endl;
									std::cout << "Packet E2E Delay  "<< Simulator::Now() -  receivedHeader.GetTs() << std::endl;
				// The Ack packets includes the number of packet successfully received and the received packet size
				// + 12 9 the same seqTsHeader Size that we subtracted at the sender side
									sprintf ((char *) bufferAck, "%d,,%d", m_nReceivedPacket,packet->GetSize ()+12);
									ackPacket = Create<Packet>(bufferAck, 10);


									TimestampTag timestamp;
									// Should never not be found since the sender is adding it, but
									// you never know.
									// Since The time stamp found in SeqTsHeader works well, we disable the tags below
							/**		if (packet->FindFirstMatchingByteTag(timestamp)) {
										Time tx = timestamp.GetTimestamp();
										std::cout << "Packet Received #  " << m_nReceivedPacket << "   At Time  "
													<< tx.GetMilliSeconds()  << std::endl;
										std::cout << "Packet E2E Delay  " << Simulator::Now() - tx << std::endl;

										if (m_delay != 0) {
											m_delay->Update(Simulator::Now() - tx);
											std::cout << "Packet E2E Delay  " << Simulator::Now() - tx << std::endl;

										}
									 else {
									//	NS_ASSERT_MSG(false,"PacketTagRetrievefailed");
									 }
								}*/

									/** disable remove all tags below until we figure out how it works
									 * with the tracing call back since we added a tag to the packet on the sender side
									 */

					//          packet->RemoveAllPacketTags ();
					//         packet->RemoveAllByteTags ();
							NS_LOG_LOGIC ("Echoing packet");
							NS_ASSERT(socket->SendTo (ackPacket, 0, from) != -1);

				}

	}
	}

} // Namespace ns3
