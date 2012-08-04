/*
 * rate-adaptive-receiver.h
 *
 *  Created on: Jun 2, 2012
 *      Author: Abdallah Abdallah
 */

#ifndef RATE_ADAPTIVE_RECEIVER_H_
#define RATE_ADAPTIVE_RECEIVER_H_


#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/stats-module.h"
#include "timestamptag.h"
namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications
 * \defgroup udpecho UdpEcho
 */

/**
 * \ingroup udpecho
 * \brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class RateAdaptiveReceiver : public Application
{
public:
  static TypeId GetTypeId (void);
  RateAdaptiveReceiver ();
  virtual ~RateAdaptiveReceiver ();
  void SetDelayTracker (Ptr<TimeMinMaxAvgTotalCalculator> delay);
  void SetCounter (Ptr<CounterCalculator<> > calc);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  Ptr<TimeMinMaxAvgTotalCalculator> m_delay;
  Ptr<CounterCalculator<> > m_calc;


  // Packet Receiving Handler
  void HandleRead (Ptr<Socket> socket);
  DropTailQueue displayBuffer;
  uint16_t m_port;
  uint32_t m_nReceivedPacket;
  Ptr<Socket> m_socket;
  Address m_local;
};

} // namespace ns3








#endif /* RATE_ADAPTIVE_RECEIVER_H_ */
