/**
 * @file rate-adaptive-sender-app.cc
 *
 * @date May 14, 2012
 *  @author  Abdallah S. Abdallah
 *  @brief this is Adaptive Streaming Server application which will read
 *  the input trace file and generate traffic and send it over UDP based
 *  on parameters such as the video frame size, encoded times, type, given
 *  in the input trace file
 */


#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "seq-ts-header.h"
#include "rate-adaptive-sender-app.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RateAdaptiveSender");
NS_OBJECT_ENSURE_REGISTERED (RateAdaptiveSender);

struct RateAdaptiveSender::TraceEntry RateAdaptiveSender::g_defaultEntries[] = {
  { 0, 534, 'I'},
  { 40, 1542, 'P'},
  { 120, 134, 'B'},
  { 80, 390, 'B'},
  { 240, 765, 'P'},
  { 160, 407, 'B'},
  { 200, 504, 'B'},
  { 360, 903, 'P'},
  { 280, 421, 'B'},
  { 320, 587, 'B'}
};



TypeId
RateAdaptiveSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RateAdaptiveSender")
    .SetParent<Application> ()
    .AddConstructor<RateAdaptiveSender> ()
    .AddAttribute ("RemoteAddress",
                   "The destination Ipv4Address of the outbound packets",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&RateAdaptiveSender::m_peerAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("RemotePort",
                   "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&RateAdaptiveSender::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MaxPacketSize",
                   "The maximum size of a packet.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&RateAdaptiveSender::m_maxPacketSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("nVideoLevels",
                   "The maximum number of video levels to support.",
                   UintegerValue (3),
                   MakeUintegerAccessor (&RateAdaptiveSender::nVideoLevels),
                   MakeUintegerChecker<uint16_t> ())
                   /**
                    * The order below cannot be reveresed because the OutputTraceFilename is needed
                    * to call LoadTrace() by the function SetTraceFile()
                    */
     .AddAttribute ("OutputTraceFilename",
                   "Name of file to output The TimeToSend Traces. By default, uses a hardcoded trace.",
                   StringValue (""),
                   MakeStringAccessor (&RateAdaptiveSender::SetOutputTraceFile),
                   MakeStringChecker ())
    .AddAttribute ("TraceFilename",
                   "Name of file to load a trace from. By default, uses a hardcoded trace.",
                   StringValue (""),
                   MakeStringAccessor (&RateAdaptiveSender::SetTraceFile),
                   MakeStringChecker ())



  ;
  return tid;
}

RateAdaptiveSender::RateAdaptiveSender ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_maxPacketSize = 1400;
  nVideoLevels = 3;
}

RateAdaptiveSender::RateAdaptiveSender (Ipv4Address ip, uint16_t port,
                                char *traceFile)
{

  char buffer[2];
  int i=1;
  std::string composedTraceFileName;
  bool status = true;

  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_peerAddress = ip;
  m_peerPort = port;
  m_currentEntry = 0;
  m_maxPacketSize = 1400;
  nVideoLevels = 3;


  for (i=1 ;  i <= nVideoLevels; i++)
  {
  sprintf(buffer,"%d",i);
  composedTraceFileName = std::string(traceFile) + std::string(buffer);
  status = (composedTraceFileName.c_str() != NULL) && status;
  }

  if (status == true)
   SetTraceFile (traceFile);

}

RateAdaptiveSender::~RateAdaptiveSender ()
{
  NS_LOG_FUNCTION (this);
  // very important to clear the input traces vectors
  for ( unsigned int i(0); i < m_entries.size(); i++ )
  m_entries[i].clear ();
}

void
RateAdaptiveSender::SetRemote (Ipv4Address ip, uint16_t port)
{
  for ( unsigned int i(0); i < m_entries.size(); i++ )
  m_entries[i].clear ();

  m_peerAddress = ip;
  m_peerPort = port;
}

void
RateAdaptiveSender::SetTraceFile (std::string traceFile)
{
  if (traceFile == "")
    {
      LoadDefaultTrace ();
    }
  else
    {
      LoadTrace (traceFile);
    }
}

void
RateAdaptiveSender::SetOutputTraceFile (std::string fileName)
{
	if (fileName == "")
		OutputTraceFilename = OutputTraceFilename + "outTimeToSend1";
	else
		OutputTraceFilename = OutputTraceFilename + fileName;

	std::cout << __PRETTY_FUNCTION__ << "---->" << OutputTraceFilename << std::endl;

}





void
RateAdaptiveSender::SetMaxPacketSize (uint16_t maxPacketSize)
{
  m_maxPacketSize = maxPacketSize;
}


uint16_t RateAdaptiveSender::GetMaxPacketSize (void)
{
  return m_maxPacketSize;
}


void
RateAdaptiveSender::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
RateAdaptiveSender::LoadTrace (std::string filename)
{
  NS_LOG_FUNCTION (this << filename);
  uint32_t time, index, prevTime = 0;
  uint16_t size;
  uint16_t loopCounter=0;
  uint32_t fileLength=0;
  uint16_t filesCounter;
  std::string composedTraceFileName;
  char buffer[2];
  std::string line;
  char frameType;
  TraceEntry entry;
  std::ifstream ifTraceFile;
  std::ofstream outTimeTosendFile;

  /**
   * By Abdallah Abdallah
   * Using ifTraceFile.good() is wrong, in C++ it is not equivalent to check the stream itself, it is just
   * its current status which may not be updated yet. This is why it reads the last line twice
   * This is why it will be modified below
   * solve this by adding a fileLength as first line in the traceFile and read it before starting the while
   * loop , decreasing it every iteration, then compare against zero
   */
  /** Adding multiple Video Levels support by Abdallah Abdallah
   *  June 12, 2012
   */


  std::cout << __PRETTY_FUNCTION__ << __LINE__ << "--->" << OutputTraceFilename.c_str() << "\n";
  outTimeTosendFile.open (OutputTraceFilename.c_str(), std::ofstream::out);

  for (filesCounter = 1;  filesCounter <= nVideoLevels; filesCounter++)
  {
	  sprintf(buffer,"%d",filesCounter);
	  composedTraceFileName = filename + std::string(buffer);
	  std::cout << __PRETTY_FUNCTION__ << __LINE__ << "--->" << composedTraceFileName.c_str() << "\n";

	  ifTraceFile.open (composedTraceFileName.c_str (), std::ifstream::in);
	 // m_entries[filesCounter - 1].clear ();
	  m_entries.push_back(std::vector<struct TraceEntry>());
	  if (!ifTraceFile.good ())
		{
		  LoadDefaultTrace ();
		  ifTraceFile.close ();
		  m_currentEntry = 0;
		}
	  else
	  {
		//  std::cout << __PRETTY_FUNCTION__ << __LINE__ << "--->" << OutputTraceFilename.c_str() << "\n";
		//  outTimeTosendFile.open (OutputTraceFilename.c_str(), std::ofstream::out);
	  std::cout << __PRETTY_FUNCTION__ << __LINE__ << "filesCounter = "<< filesCounter <<std::endl;
	  ifTraceFile >> fileLength;
	  std::cout << __PRETTY_FUNCTION__ << __LINE__  << "fileLength"<< fileLength <<std::endl;
	  while (ifTraceFile.good () && fileLength > 0)
		{


		  std::cout <<  __PRETTY_FUNCTION__ << __LINE__ << "--->" << ifTraceFile.good () << std::endl;
		  /** This code read the entire file and store it in the
		   * vector m_entries after scheduling the correct related
		   * time for events ( scheduling sending time stamps
		   * according to Frame Types )
		   *
		   */
		  /**
		   * check why after passing the filenames from outside
		   * the last line gets read twice into the memory ?
		   */

		  ifTraceFile >> index >> frameType >> time >> size;
		  if (frameType == 'B')
			{
			  entry.timeToSend = 0;
			}
		  else
			{
			  /**
			   * TODO Fix the bug exist here in calucalting the entry.timeToSend
			   * check the outFileStream to see how all the times = 120 !!!
			   * The same problem does not exist with loading the default
			   * trace struct !
			   */
			  entry.timeToSend = time - prevTime;
			  prevTime = time;
			}
		  entry.packetSize = size;
		  entry.frameType = frameType;
		  std::cout <<  __PRETTY_FUNCTION__ << __LINE__ << "--->"  << std::endl;

		  m_entries[filesCounter - 1].push_back(entry);
		  std::cout <<  __PRETTY_FUNCTION__ << __LINE__ << "--->"  << std::endl;
		  if (filesCounter == 1)
		  outTimeTosendFile << entry.frameType <<"		"<< entry.timeToSend << "\n";
		  loopCounter++;
		  fileLength--;
		  std::cout <<  __PRETTY_FUNCTION__ << " loopCounter = " << loopCounter << std::endl;
		} // end of while loop
	  if (filesCounter == 1)
	   outTimeTosendFile.close();
	   ifTraceFile.close ();
	   m_currentEntry = 0;

	  } //end of else branch


  } // end of the different video levels for loop


}

void
RateAdaptiveSender::LoadDefaultTrace (void)
{
  NS_LOG_FUNCTION (this);
  uint32_t prevTime = 0;
  std::ofstream outTimeTosendFile;
  outTimeTosendFile.open (OutputTraceFilename.c_str(), std::ofstream::out);

  for (uint32_t i = 0; i < (sizeof (g_defaultEntries) / sizeof (struct TraceEntry)); i++)
    {
      struct TraceEntry entry = g_defaultEntries[i];
      if (entry.frameType == 'B')
        {
          entry.timeToSend = 0;
        }
      else
        {
          uint32_t tmp = entry.timeToSend;
          entry.timeToSend -= prevTime;
          prevTime = tmp;
        }
      // use the first vector in the array
      m_entries[0].push_back (entry);
      outTimeTosendFile << entry.frameType <<"		"<< entry.timeToSend << "\n";
    }
  m_currentEntry = 0;
  outTimeTosendFile.close();
}

void
RateAdaptiveSender::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
	  //note how this App. runs on UDP socket
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      m_socket->Bind ();
      m_socket->Connect (InetSocketAddress (m_peerAddress, m_peerPort));
    }
  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &RateAdaptiveSender::Send, this);
}

void
RateAdaptiveSender::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
RateAdaptiveSender::SendPacket (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  Ptr<Packet> p;
  uint32_t packetSize;
  if (size>12)
    {
      packetSize = size - 12; // 12 is the size of the SeqTsHeader
    }
  else
    {
      packetSize = 0;
    }
  p = Create<Packet> (packetSize);
  SeqTsHeader seqTs;
  seqTs.SetSeq (m_sent);
  p->AddHeader (seqTs);
  if ((m_socket->Send (p)) >= 0)
    {
      ++m_sent;
      NS_LOG_INFO ("Sent " << size << " bytes to "
                           << m_peerAddress);
    }
  else
    {
      NS_LOG_INFO ("Error while sending " << size << " bytes to "
                                          << m_peerAddress);
    }
}

void
RateAdaptiveSender::Send (void)
{
  NS_LOG_FUNCTION (this);
  uint16_t vectorCounter = 0;
  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> p;
  // we will use the first vector for testing first time only
  struct TraceEntry *entry = &m_entries[vectorCounter][m_currentEntry];
  std::cout << "Entry Vector Size" << m_entries[vectorCounter].size () << "\n";
  /** By Abdallah S. Abdallah
   * Do-While Looping over all the Frames' entries whose TimeToSend already reached
   * zero which means it is time to send them now.
   * It is serial traversing over the vector
   */
  do
    {
	  /** By Abdallah S. Abdallah
	    * partition each video frame into multiple UDP packets
	    * if the size > maximum packet size assigned to the model
	    * max packet size variable
	    */
	  for (int i = 0; i < entry->packetSize / m_maxPacketSize; i++)
        {
          SendPacket (m_maxPacketSize);
        }

      uint16_t sizetosend = entry->packetSize % m_maxPacketSize;
      SendPacket (sizetosend);
/** By Abdallah S. Abdallah
 * according to the trace file format and how the loading TraceFile function
 * assign B Frames sending times to (zero), then there is no need
 * to sort the TimeToSend of I, and P Frames because they should be
 * already sorted by default
 */
      m_currentEntry++;
      m_currentEntry %= m_entries[vectorCounter].size ();
      entry = &m_entries[vectorCounter][m_currentEntry];
    }
  while (entry->timeToSend == 0);
  m_sendEvent = Simulator::Schedule (MilliSeconds (entry->timeToSend), &RateAdaptiveSender::Send, this);
}

} // Namespace ns3


