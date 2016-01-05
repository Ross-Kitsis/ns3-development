/*
 * tm-dpd.cc
 *
 *  Created on: Jan 1, 2016
 *      Author: ross
 */

#include "tm-dpd.h"
#include "ns3/vector.h"

namespace ns3
{
namespace mcast
{

bool
ThesisMcastDuplicatePacketDetection::IsDuplicate(Ptr<const Packet> p, const ControlHeader &header)
{
	bool toReturn = false;
	//Check cache
	toReturn = m_idCache.IsDuplicate(header.GetId(), header.getApxL(), header.getApxR());
	return toReturn;
}

void
ThesisMcastDuplicatePacketDetection::SetLifetime(Time lifetime)
{
	m_idCache.SetLifetime (lifetime);
}

Time
ThesisMcastDuplicatePacketDetection::GetLifeTime() const
{
  return m_idCache.GetLifeTime ();
}


} /* namespace mcast*/
} /* namespace ns3 */
