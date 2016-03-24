/*
 * RsuCache.h
 *
 *  Created on: Mar 13, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_MODEL_RSUCACHE_H_
#define SRC_THESISINTERNETROUTING_MODEL_RSUCACHE_H_

#include "ns3/vector.h"
#include "ns3/nstime.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace thesis
{

class RsuCacheEntry
{
public:

	RsuCacheEntry(Ipv6Address m_Source = Ipv6Address(), Ipv6Address m_Destination = Ipv6Address(),
								Vector m_SendingNodePosition = Vector(), Vector m_SendingNodeVelocity = Vector(),
								Time m_SendTime = Simulator::Now(), Time m_ReceiveTime = Simulator::Now());

	~RsuCacheEntry();

	Ipv6Address GetSource();
	void SetSource(Ipv6Address Source);

	Ipv6Address GetDestination();
	void SetDestination(Ipv6Address Destination);

	Vector GetSendingNodePosition();
	void SetSendingNodePosition(Vector Position);

	Vector GetSendingNodeVelocity();
	void SetSendingNodeVelocity(Vector Velocity);

	Time GetSendTime();
	void SetSendTime(Time SendTime);

	Time GetReceiveTime();
	void SetReceiveTime(Time ReceiveTime);

	bool operator== (RsuCacheEntry const & o) const;

private:
	/*
	 * Source of transmission in VANET
	 */
	Ipv6Address m_Source;
	/*
	 * Destination of transmittion
	 */
	Ipv6Address m_Destination;
	/*
	 * Position of the sending node
	 */
	Vector m_SendingNodePosition;
	/*
	 * Velocity (Speed + direction) of sending node
	 */
	Vector m_SendingNodeVelocity;
	/*
	 * Timestamp of when message was sent
	 */
	Time m_SendTime;
	/*
	 * Timestamp of when message was received
	 */
	Time m_ReceiveTime;

};

class RsuCache
{
public:
	RsuCache();
	virtual ~RsuCache();

	/*
	 * Add entry to the cache
	 */
	void AddEntry(RsuCacheEntry * entry);

	/*
	 * Remove entry from the cache
	 */
	void RemoveEntry(Ipv6Address toRemove);

	/*
	 * Lookup entry in cache
	 * Set entry to the entry
	 * Return true if entry found; false otherwise
	 */
	bool Lookup(Ipv6Address toFind, RsuCacheEntry &entry);

	/**
	 * Iterates through all entries in the cache and averages the velocities of all	entries with teh passed source address
	 */
	Vector GetAverageVelocity(Ipv6Address toFind);

	/**
	 * Check if cache contains a specific entry
	 */
	bool ContainsEntry(Ipv6Address source, Ipv6Address destination, Time sendTime);

private:

	/// Container for the network routes - pair RipNgRoutingTableEntry *, EventId (update event)
	typedef std::list<std::pair <RsuCacheEntry *, EventId> > RSUCache;

  typedef std::list<std::pair <RsuCacheEntry *, EventId> >::iterator RSUCacheIC;


	RSUCache m_cache;

	/*
	 * Iterates through cache, removing any stale entries
	 */
	void CleanCache();

	/*
	 * Maximum time an entry can be in the cache
	 */
	Time m_MaxCacheTime;

};

} /* namespace thesis*/
} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_MODEL_RSUCACHE_H_ */
