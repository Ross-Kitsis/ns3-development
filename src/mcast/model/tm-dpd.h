/*
 * tm-dpd.h
 *
 *  Created on: Jan 1, 2016
 *      Author: ross
 */

#ifndef SRC_MCAST_MODEL_TM_DPD_H_
#define SRC_MCAST_MODEL_TM_DPD_H_

#include "ns3/header.h"
#include "ns3/packet.h"
#include "ns3/vector.h"

namespace ns3
{
namespace mcast
{

class ThesisMcastDuplicatePacketDetection
{
 /**
  * \brief Helper class to track previously received broadcasts and multicasts
  *
  * Base detection for mcast on the source address, and apex coordinates in the mcast header
  *
  */
public:
	//Constructor
	ThesisMcastDuplicatePacketDetection(Time lifetime);
	// Check if packet is duplicate; if not save packet information
	bool IsDuplicate (Ptr<const Packet> p, const Header &header);
	// Set duplicate record lifetime
	void SetLifetime (Time lifetime);
	//Get duplicate lifetime
	Time GetLifeTime() const;

private:
	//Implement a cache to hold entries

};


} /* namespace mcast*/
} /* namespace ns3 */

#endif /* SRC_MCAST_MODEL_TM_DPD_H_ */
