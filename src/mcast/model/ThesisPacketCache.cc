/*
 * ThesisPacketCache.cc
 *
 *  Created on: Jan 4, 2016
 *      Author: ross
 */

#include "ThesisPacketCache.h"
#include <algorithm>
#include "ns3/vector.h"

namespace ns3
{

namespace mcast
{

/**
 * Determines if a passed packet is a duplicate
 */
bool
ThesisMcastCache::IsDuplicate(Ipv6Address addr, Vector left, Vector right)
{
	//Remove expired entries
	Purge();
	//Check all addresses in the chache for a match
  for (std::vector<UniqueId>::const_iterator i = m_idCache.begin ();
       i != m_idCache.end (); ++i)
    if (i->m_sender == addr && i->m_left.x == left.x && i->m_left.y == left.y
    												&& i->m_right.x == right.x && i->m_right.y == right.y)
      return true;
  struct UniqueId uniqueId =
  { addr, left,right, m_lifetime + Simulator::Now () };
  m_idCache.push_back (uniqueId);
  return false;

}

/**
 * Remove expired entries
 */
void
ThesisMcastCache::Purge()
{
	m_idCache.erase(remove_if (m_idCache.begin (), m_idCache.end (),
                              IsExpired ()), m_idCache.end ());
}

/**
 * Return the size of the cache
 */
uint32_t
ThesisMcastCache::GetSize()
{
	//Purge stale records
	Purge();
	return m_idCache.size();
}

}
} /* namespace ns3 */
