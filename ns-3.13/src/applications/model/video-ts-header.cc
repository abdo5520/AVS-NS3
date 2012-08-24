/*
 * video-ts-header.cc
 *
 *  Created on: Aug 23, 2012
 *      Author: Abdallah Abdallah
 */


#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "video-ts-header.h"

NS_LOG_COMPONENT_DEFINE ("VideoTsHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (VideoTsHeader);

VideoTsHeader::VideoTsHeader ()
  : m_seq (0),
    m_partialSeq(0),
    m_ts (Simulator::Now ().GetTimeStep ())
{
}

void
VideoTsHeader::SetSeq (uint32_t seq)
{
  m_seq = seq;
}


void
VideoTsHeader::SetPartSeq (uint32_t part)
{
	m_partialSeq = part;
}

void
VideoTsHeader::SetSequences (uint32_t seq, uint32_t part)
{
	m_seq = seq;
	m_partialSeq = part;
}

uint32_t
VideoTsHeader::GetSeq (void) const
{
  return m_seq;
}

uint32_t
VideoTsHeader::GetPartSeq(void) const
{
  return m_partialSeq;
}


Time
VideoTsHeader::GetTs (void) const
{
  return TimeStep (m_ts);
}

TypeId
VideoTsHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VideoTsHeader")
    .SetParent<Header> ()
    .AddConstructor<VideoTsHeader> ()
  ;
  return tid;
}
TypeId
VideoTsHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
VideoTsHeader::Print (std::ostream &os) const
{
  os << "(seq=" << m_seq << "  partial Sequence= " << m_partialSeq << " time=" << TimeStep (m_ts).GetSeconds () << ")";
}
uint32_t
VideoTsHeader::GetSerializedSize (void) const
{
  return 4+4+8;
}

void
VideoTsHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU32 ( m_partialSeq);
  i.WriteHtonU64 (m_ts);
}
uint32_t
VideoTsHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU32 ();
  m_partialSeq = i.ReadNtohU32 ();
  m_ts = i.ReadNtohU64 ();
  return GetSerializedSize ();
}

} // namespace ns3


