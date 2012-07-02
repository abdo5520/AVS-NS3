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

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void HandleRead (Ptr<Socket> socket);

  uint16_t m_port;
  Ptr<Socket> m_socket;
  Address m_local;
};

} // namespace ns3








#endif /* RATE_ADAPTIVE_RECEIVER_H_ */
