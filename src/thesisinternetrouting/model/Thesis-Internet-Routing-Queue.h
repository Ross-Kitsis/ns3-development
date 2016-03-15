/*
 * Thesis-Internet-Routing-Queue.h
 *
 *  Created on: Feb 17, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_MODEL_THESIS_INTERNET_ROUTING_QUEUE_H_
#define SRC_THESISINTERNETROUTING_MODEL_THESIS_INTERNET_ROUTING_QUEUE_H_

#include "ns3/vector.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/simulator.h"
#include "ns3/timer.h"


namespace ns3
{
namespace thesis
{

class ThesisInternetQueueEntry
{
public:
	typedef Ipv6RoutingProtocol::UnicastForwardCallback UnicastForwardCallback;
	typedef Ipv6RoutingProtocol::ErrorCallback ErrorCallback;

	ThesisInternetQueueEntry (Ptr<const Packet> pa = 0, Ipv6Header const & h = Ipv6Header (),
			UnicastForwardCallback ucb = UnicastForwardCallback (),
			ErrorCallback ecb = ErrorCallback ()) :
				m_packet (pa), m_header (h), m_ucb (ucb), m_ecb (ecb)
	{}

	/**
	 * Compare queue entries
	 * \return true if equal
	 */
	bool operator== (ThesisInternetQueueEntry const & o) const
  						{
		return ((m_packet == o.m_packet) && (m_header.GetDestinationAddress() == o.m_header.GetDestinationAddress ()) );
  						}

	// Fields
	UnicastForwardCallback GetUnicastForwardCallback () const { return m_ucb; }
	void SetUnicastForwardCallback (UnicastForwardCallback ucb) { m_ucb = ucb; }

	ErrorCallback GetErrorCallback () const { return m_ecb; }
	void SetErrorCallback (ErrorCallback ecb) { m_ecb = ecb; }

	Ptr<const Packet> GetPacket () const { return m_packet; }
	void SetPacket (Ptr<const Packet> p) { m_packet = p; }

	Ipv6Header GetIpv6Header () const { return m_header; }
	void SetIpv6Header (Ipv6Header h) { m_header = h; }

	Timer GetTimer() const {return m_RetransmitTimer;}

private:

	/// Data packet
	Ptr<const Packet> m_packet;
	/// IP header
	Ipv6Header m_header;
	/// Unicast forward callback
	UnicastForwardCallback m_ucb;
	/// Error callback
	ErrorCallback m_ecb;
	/// Timer to retransmit
	Timer m_RetransmitTimer;

};


class ThesisInternetRoutingQueue
{
public:


	ThesisInternetRoutingQueue();

	virtual ~ThesisInternetRoutingQueue();

	/**
	 * Adds a packet to the routing queue
	 */
	void AddRoutingEntry(ThesisInternetQueueEntry * entry);

	/**
	 * Remove a queue entry
	 * Generally used if a lookup is true on a packet that was retransmitted
	 * OR used if a timer expires and the packet is retransmitted
	 */
	void RemoveRoutingQueueEntry();

	/**
	 * Lookup cache based on the source, destination and sendtime
	 * Tuple should be unique
	 */
	bool Lookup(Ipv6Address source, Ipv6Address destination, Time sendTime);

private:
	/// Container for the network routes - pair RipNgRoutingTableEntry *, EventId (update event)
	typedef std::list<std::pair <ThesisInternetQueueEntry *, EventId> > RoutingQueue;

  typedef std::list<std::pair <ThesisInternetQueueEntry *, EventId> >::iterator RoutingQueueI;

};

} /* namespace ns3 */
} /* namespace thesis */
#endif /* SRC_THESISINTERNETROUTING_MODEL_THESIS_INTERNET_ROUTING_QUEUE_H_ */
