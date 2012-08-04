/*
 * timestamptag.h
 *
 *  Created on: Aug 3, 2012
 *      Author: abdo
 */

#ifndef TIMESTAMPTAG_H_
#define TIMESTAMPTAG_H_

#include "ns3/network-module.h"
namespace ns3 {
//------------------------------------------------------
class TimestampTag : public Tag {
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);

  // these are our accessors to our tag structure
  void SetTimestamp (Time time);
  Time GetTimestamp (void) const;

  void Print (std::ostream &os) const;

private:
  Time m_timestamp;

  // end class TimestampTag
};



}; //end of ns3 name space




#endif /* TIMESTAMPTAG_H_ */
