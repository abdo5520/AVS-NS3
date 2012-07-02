/*
 * @file rate-adaptive-sender-app.h
 *
 * @date May 14, 2012
 *  @author Abdallah Abdallah
 */

#ifndef RATE_ADAPTIVE_SENDER_APP_H_
#define RATE_ADAPTIVE_SENDER_APP_H_

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include <vector>
#include <iostream>
#include <stdlib.h>

#define maxVideolLevels 32


namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup udpclientserver
 * \class UdpTraceClient
 * \brief A trace based streamer
 *
 * sends udp packets based on a trace file of an MPEG4 stream
 * trace files could be downloaded form :
 * http://www.tkn.tu-berlin.de/research/trace/ltvt.html (the 2 first lines of
 * the file should be removed)
 * A valid trace file is a file with 4 columns:
 * -1- the first one represents the frame index
 * -2- the second one indicates the type of the frame: I, P or B
 * -3- the third one indicates the time on which the frame was generated by the encoder
 * -4- the fourth one indicates the frame size in byte
 * if no valid MPEG4 trace file is provided to the application the trace from
 * g_defaultEntries array will be loaded.
 */
class RateAdaptiveSender : public Application
{
public:
  static TypeId
  GetTypeId (void);
  /**
   * \brief creates a rate-adaptive streaming Sender application
   */
  RateAdaptiveSender ();

  /**
   * \brief creates a rate-adaptive streaming Sender application
   * \param ip the destination ip address to which the stream will be sent
   * \param port the destination udp port to which the stream will be sent
   * \param traceFile a path to an MPEG4 trace file formatted as follows:
   *  FrameNo Frametype   Time[ms]    Length [byte]
   *  FrameNo Frametype   Time[ms]    Length [byte]
   *  ...
   *
   *
   */

  RateAdaptiveSender (Ipv4Address ip, uint16_t port, char *traceFile);
  ~RateAdaptiveSender ();

  /**
   * \brief set the destination IP address and port
   * \param ip the destination ip address to which the stream will be sent
   * \param port the destination udp port to which the stream will be sent
   */
  void SetRemote (Ipv4Address ip, uint16_t port);

  /**
   * \brief set the trace file to be used by the application
   * \param filename a path to an MPEG4 trace file formatted as follows:
   *  Frame No Frametype   Time[ms]    Length [byte]
   *  Frame No Frametype   Time[ms]    Length [byte]
   *  ...
   */
  void SetTraceFile (std::string filename);


  void SetOutputTraceFile (std::string filename);

  /**
   * \return the maximum packet size
   */
  uint16_t GetMaxPacketSize (void);

  /**
   * \param maxPacketSize The maximum packet size
   */
  void SetMaxPacketSize (uint16_t maxPacketSize);

protected:
  virtual void DoDispose (void);

private:
  void LoadTrace (std::string filename);
  void LoadDefaultTrace (void);
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void ScheduleTransmit (Time dt);
  void Send (void);
  void SendPacket (uint32_t size);


  struct TraceEntry
  {
    uint32_t timeToSend;
    uint16_t packetSize;
    char frameType;
  };
  uint32_t m_sent;
  std::string OutputTraceFilename;
  Ptr<Socket> m_socket;
  Ipv4Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;
  uint16_t nVideoLevels; 				// number of video levels supported = number of input traces
  std::vector < std::vector<struct TraceEntry> > m_entries;   // the data structure to hold all the video traces levels at a time
  uint32_t m_currentEntry;
  static struct TraceEntry g_defaultEntries[];
  uint16_t m_maxPacketSize;
};

} // namespace ns3



#endif /* RATE_ADAPTIVE_SENDER_APP_H_ */