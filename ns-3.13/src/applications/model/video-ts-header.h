/*
 * video-ts-header.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Abdallah Abdallah
 */

#ifndef VIDEO_TS_HEADER_H_
#define VIDEO_TS_HEADER_H_

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {
/**
 * \ingroup udpclientserver
 * \class VideoTsHeader
 * \brief Video Frame header for Video Udp client/server application
 * The header is made of a 32bits sequence number followed by another 32bit
 * a 64bits time stamp.
 */
class VideoTsHeader : public Header
{
public:
  VideoTsHeader ();

  /**
   * \param set the sequence number
   */
  void SetSeq (uint32_t seq);

  /**
     * \param set the partial sequence number
     */
  void
  SetPartSeq (uint32_t part);
  /**
   * \param set both the seq and the partial seq value
   */
  void SetSequences (uint32_t seq, uint32_t partial);
  /**
   * \return the sequence number
   */
  uint32_t GetSeq (void) const;

  uint32_t GetPartSeq (void) const;
  /**
   * \return the time stamp
   */
  Time GetTs (void) const;

  static TypeId GetTypeId (void);
private:
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  uint32_t m_seq;
  uint32_t m_partialSeq;
  uint64_t m_ts;
};

} // namespace ns3



#endif /* VIDEO_TS_HEADER_H_ */
