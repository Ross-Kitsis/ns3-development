/*
 * ThesisPacketCache.h
 *
 *  Created on: Jan 4, 2016
 *      Author: Ross Kitsis
 */

#ifndef SRC_MCAST_MODEL_THESISPACKETCACHE_H_
#define SRC_MCAST_MODEL_THESISPACKETCACHE_H_

#include <vector>
#include "ns3/vector.h"
#include "ns3/ipv6-address.h"
#include "ns3/simulator.h"

#include "mcast-packet.h"

namespace ns3
{

namespace mcast
{

class ThesisMcastCache
{
public:
	//Constructor
	ThesisMcastCache(Time lifetime) : m_lifetime (lifetime){}

  /// Check that entry (addr, id) exists in cache. Add entry, if it doesn't exist.
  bool IsDuplicate (Ipv6Address addr, Vector left, Vector right);

  /// Remove all expired entries
  void Purge ();

  /// Return number of entries in cache
  uint32_t GetSize ();

  /// Set lifetime for future added entries.
  void SetLifetime (Time lifetime) { m_lifetime = lifetime; }

  /// Return lifetime for existing entries in cache
  Time GetLifeTime () const { return m_lifetime; }


private:

  struct UniqueId
  {
  	//Unique sender address
  	Ipv6Address m_sender;

  	//Left apex
  	Vector m_left;

  	//Right apex
  	Vector m_right;

  	// When record will expire
  	Time m_expire;
  };
  struct IsExpired
  {
  	bool operator() (const struct UniqueId & u) const
  	{
  		return (u.m_expire < Simulator::Now ());
  	}
  };

  /// Already seen IDs
  std::vector<UniqueId> m_idCache;

	/// Default lifetime for ID records
	Time m_lifetime;

};

} // namespace mcast
} /* namespace ns3 */

#endif /* SRC_MCAST_MODEL_THESISPACKETCACHE_H_ */
