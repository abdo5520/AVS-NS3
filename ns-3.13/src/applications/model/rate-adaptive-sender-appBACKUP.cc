/**
 * @file rate-adaptive-sender-app.cc
 *
 * @date May 14, 2012
 *  @author  Abdallah S. Abdallah
 *  @brief this is Adaptive Streaming (Sender) Server application which will read
 *  the input MPEG-4 trace file,generate traffic and send it over UDP based
 *  on parameters such as the video frame size, encoded times, and frame-type given
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
#include <algorithm>



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RateAdaptiveSender");
NS_OBJECT_ENSURE_REGISTERED (RateAdaptiveSender);

struct RateAdaptiveSender::TraceEntry RateAdaptiveSender::g_defaultEntries[] = {
  { 0, 534, 'I',-1,1},
  { 40, 1542, 'P',-1,2},
  { 120, 134, 'B',-1,3},
  { 80, 390, 'B',-1,4},
  { 240, 765, 'P',-1,5},
  { 160, 407, 'B',-1,6},
  { 200, 504, 'B',-1,7},
  { 360, 903, 'P',-1,8},
  { 280, 421, 'B',-1,9},
  { 320, 587, 'B',-1,10}
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
                   UintegerValue (1400),
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
                   "The name of the input traces file to load traces from. The default uses a hard-coded defined struct",
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
  currentLevel = 0;
  deltaTime = 40;

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
  currentLevel = 0;
  deltaTime = 40;
  m_sendEvent = EventId ();
  m_peerAddress = ip;
  m_peerPort = port;
  m_currentEntry = 0;
  m_maxPacketSize = 1400;
  nVideoLevels = 3;

  // This code just check that the input trace file name is set correctly and can be combined
  // with the video quality level that is represented as char "1", "2", "3", ..., "N"
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

/**
bool myfunction (struct RateAdaptiveSender::TraceEntry  a,struct RateAdaptiveSender::TraceEntry  b) {

	return (a.traceTime < b.traceTime);
}

*/

void
RateAdaptiveSender::LoadTrace (std::string filename)
{
  NS_LOG_FUNCTION (this << filename);
  char buffer[2];
  char frameType;
  uint32_t time = 0;
  uint32_t index = 0;
  uint32_t prevTimeP = 0;
//  uint32_t prevTimeB = 0;
  uint16_t size;
  uint16_t loopCounter=0;
  uint32_t fileLength=0;
  uint16_t filesCounter=1;

  TraceEntry entry;

  std::string composedTraceFileName;
  std::string line;
  std::ifstream ifTraceFile;
  std::ofstream outTimeTosendFile;

  /**
   * By Abdallah Abdallah
   * Using ifTraceFile.good() is wrong, in C++ it is not equivalent to check the stream itself, it ONLY checks
   * its current status which may not be updated yet. This is why it reads the last line twice
   * This is why it will be modified below
   * solve this by adding a fileLength as first line in the traceFile and read it before starting the while
   * loop , decreasing it every iteration, and compare against zero
   */
  /** Adding multiple Video Levels support by Abdallah S. Abdallah
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
	  // push to initialize the tracing vector before filling the vector with data read from the trace file
	  m_entries.push_back(std::vector<struct TraceEntry>());
	  if (!ifTraceFile.good ())
		{
		  ifTraceFile.close ();
		  m_currentEntry = 0;
		  NS_ASSERT(1 < 0);
		}
	  else
	  {
		//  std::cout << __PRETTY_FUNCTION__ << __LINE__ << "--->" << OutputTraceFilename.c_str() << "\n";
		//  outTimeTosendFile.open (OutputTraceFilename.c_str(), std::ofstream::out);
	  std::cout << __PRETTY_FUNCTION__ << __LINE__ << "filesCounter = "<< filesCounter <<std::endl;
	 // Read the first line in the file which is the number of video frames traces inside the file
	  ifTraceFile >> fileLength;
	  std::cout << __PRETTY_FUNCTION__ << __LINE__  << "fileLength"<< fileLength <<std::endl;
	  }
	  while (fileLength > 0)
		{


		//  std::cout <<  __PRETTY_FUNCTION__ << __LINE__ << "--->" << ifTraceFile.good () << std::endl;
		  /** This code read the entire file and store it in the
		   * vector m_entries after scheduling the correct related
		   * time for events ( scheduling sending time stamps
		   * according to Frame Types )
		   *
		   */
		  /**   ALREADY FIXED
		   * check why after passing the filenames from outside
		   * the last line gets read twice into the memory ?
		   */

		  ifTraceFile >> index >> frameType >> time >> size;
		  entry.traceTime = time;
		  entry.frameType = frameType;
		  entry.frameSize = size;
		  entry.index = index;
		  // entry.timeToSend is to be calculated later according to the rate controller

		  /**
		   * TODO Fix the bug exist here in calculating the entry.timeToSend
		   * check the outFileStream to see how all the times = 120 !!!
		   * The same problem does not exist with loading the default
		   * trace struct !
		 */
		  if (frameType == 'B')
			{
			  entry.timeToSend = 40;
			}

		  else
			{
			  entry.timeToSend = entry.traceTime-prevTimeP;
			  prevTimeP = entry.traceTime;
			}


		  std::cout <<  __PRETTY_FUNCTION__ << __LINE__ << "--->"  << std::endl;

		  m_entries[filesCounter - 1].push_back(entry);

		  std::cout <<  __PRETTY_FUNCTION__ << __LINE__ << "--->"  << std::endl;
		  if (filesCounter == 1){
		  outTimeTosendFile << entry.index << "		" << entry.frameType <<"		" <<  entry.timeToSend << "\n";
		  }
		  loopCounter++;
		  fileLength--;
		  std::cout <<  __PRETTY_FUNCTION__ << " loopCounter = " << loopCounter << std::endl;
		} // end of while loop
	  if (filesCounter == 1){
	   outTimeTosendFile.close();
	  }
	   ifTraceFile.close ();
	   // Notice that m_currentEntry must set to zero before
	   // RateAdaptiveSender::StartApplication (void) get called
	   m_currentEntry = 0;

  } // end of the different video levels For loop
 /** for (filesCounter = 1;  filesCounter <= nVideoLevels; filesCounter++)
  {
	  //sort (myvector.begin()+4, myvector.end(), myfunction);
	  sort (m_entries[filesCounter -1].begin(),m_entries[filesCounter -1].end(),myfunction);

  }
*/
} // end of the function

void
RateAdaptiveSender::LoadDefaultTrace (void)
{
	//TODO fix entry.packetSize to become entry.frameSize after I already changed the struct definition
  NS_LOG_FUNCTION (this);
  uint32_t prevTime = 0;
  std::ofstream outTimeTosendFile;
  outTimeTosendFile.open (OutputTraceFilename.c_str(), std::ofstream::out);

  for (uint32_t i = 0; i < (sizeof (g_defaultEntries) / sizeof (struct TraceEntry)); i++)
    {
	  //TODO find out how this assignment works
      struct TraceEntry entry = g_defaultEntries[i];
      if (entry.frameType == 'B')
        {
    	  // This is not the absolute time, but it is the relative time difference with the current clock value
          entry.timeToSend = 0;
        }
      else
        {
          entry.timeToSend =entry.traceTime - prevTime;
          prevTime = entry.traceTime;
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
  // This can be used later to handle the received frame in order to handle the incoming ACks
  // Plus handling the Rate-Control Feedback messages
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
  // TODO find how the video frame size loses 12 bytes. This should be addition
  // not subtraction !
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
  // TODO How the 64 bit time-stamp gets set inside the header ?
  // Does get automatically in case sending succeeded ?
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
  // default Video level is the lowest until change is triggered based on feedback
  uint16_t levelCounter = currentLevel;
  // Use NS_ASSERT , dont use the original assert function
  NS_ASSERT(m_sendEvent.IsExpired());

  Ptr<Packet> p;
  // we will use the first vector for testing first time only
  struct TraceEntry *entry = &m_entries[levelCounter][m_currentEntry];
  struct TraceEntry *entryNext = &m_entries[levelCounter][m_currentEntry+1];
  std::cout << "Entry Vector Size" << m_entries[levelCounter].size () << "\n";
  std::cout << "m_currentEntry" << m_currentEntry << "\n";
  std::cout << "vectorCounter" << levelCounter << "\n" ;
  std::cout << " m_maxPacketSize" <<  m_maxPacketSize <<"\n";
  /**
   * By Abdallah S. Abdallah
   * Do-While Looping over all the Frames' entries whose TimeToSend already reached
   * zero which means it is time to send them now.
   * It is serial traversing over the vector
   */
/
  if ( (entry->frameType == "B") && (entryNext->frameType =="B") ) {
	  /* By Abdallah S. Abdallah
	    * partition each video frame into multiple UDP packets
	    * if the size > maximum packet size assigned to the model
	    * max packet size variable
	    */
	  for (int i = 0; i < entry->frameSize / m_maxPacketSize; i++)
        {
		  // No smoothing is implemented. The server sends the maximum possible packet size
		  // when the video frame size does not fit into one networking packet
          SendPacket (m_maxPacketSize);
        }

      uint16_t sizetosend = entry->frameSize % m_maxPacketSize;
      SendPacket (sizetosend);

  	/** By Abdallah S. Abdallah
  	 * according to the trace file format and how the loading TraceFile function
  	 * assign B Frames sending times to (zero), then there is no need
  	 * to sort the TimeToSend of I, and P Frames because they should be
  	 * already sorted by default
  	 */
        m_currentEntry++;
        m_currentEntry %= m_entries[levelCounter].size ();
  // assign the pointer to the next video frame within the current level
        entry = &m_entries[levelCounter][m_currentEntry];
        m_sendEvent = Simulator::Schedule (MilliSeconds (entry->timeToSend), &RateAdaptiveSender::Send, this);
        return;
  }

   if ( (entry->frameType == "B") && (entryNext->frameType == ("I" || "P"))){
	   uint32_t timeShift = entryNext->traceTime - entry->traceTime;
	   for (int i = 0; i < entry->frameSize / m_maxPacketSize; i++)
	          {
	  		  // No smoothing is implemented. The server sends the maximum possible packet size
	  		  // when the video frame size does not fit into one networking packet
	            SendPacket (m_maxPacketSize);
	          }
	   uint16_t sizetosend = entry->frameSize % m_maxPacketSize;
	        SendPacket (sizetosend);
	        m_currentEntry++;
	        m_currentEntry %= m_entries[levelCounter].size ();
	  // assign the pointer to the next video frame within the current level
	        entry = &m_entries[levelCounter][m_currentEntry];
	        m_sendEvent = Simulator::Schedule (MilliSeconds (timeShift), &RateAdaptiveSender::Send, this);
	        return;

  }
   if ( (entry->frameType == ("I" || "P")) && (entryNext->frameType == "B")){

   }


  do
    {
	  /* By Abdallah S. Abdallah
	    * partition each video frame into multiple UDP packets
	    * if the size > maximum packet size assigned to the model
	    * max packet size variable
	    */
	  for (int i = 0; i < entry->frameSize / m_maxPacketSize; i++)
        {
		  // No smoothing is implemented. The server sends the maximum possible packet size
		  // when the video frame size does not fit into one networking packet
          SendPacket (m_maxPacketSize);
        }

      uint16_t sizetosend = entry->frameSize % m_maxPacketSize;
      SendPacket (sizetosend);

	/** By Abdallah S. Abdallah
	 * according to the trace file format and how the loading TraceFile function
	 * assign B Frames sending times to (zero), then there is no need
	 * to sort the TimeToSend of I, and P Frames because they should be
	 * already sorted by default
	 */
      m_currentEntry++;
      m_currentEntry %= m_entries[levelCounter].size ();
// assign the pointer to the next video frame within the current level
      entry = &m_entries[levelCounter][m_currentEntry];
    }
  while (entry->timeToSend == 0);


  m_sendEvent = Simulator::Schedule (MilliSeconds (entry->timeToSend), &RateAdaptiveSender::Send, this);
}

} // Namespace ns3


