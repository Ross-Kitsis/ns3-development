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
	typedef Ipv6RoutingProtocol::LocalDeliverCallback LocalDeliveryCallback;

	ThesisInternetQueueEntry (Ptr<Packet> pa = 0, Ipv6Header const & h = Ipv6Header (),
			UnicastForwardCallback ucb = UnicastForwardCallback (),
			ErrorCallback ecb = ErrorCallback (), Time SendTime = Time(),
			LocalDeliveryCallback lcb = LocalDeliveryCallback()) :
				m_packet (pa), m_header (h), m_ucb (ucb), m_ecb (ecb), m_SendTime(SendTime), m_lcb(lcb)
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


	Ptr<Packet> GetPacket () const { return m_packet; }
	void SetPacket (Ptr<Packet> p) { m_packet = p; }

	Ipv6Header GetIpv6Header () const { return m_header; }
	void SetIpv6Header (Ipv6Header h) { m_header = h; }

	Time GetPacketSendTime() const { return m_SendTime;}
	void SetPacketSendTime(Time SendTime) { m_SendTime = SendTime;}

	Timer GetTimer() const {return m_RetransmitTimer;}

	/// Timer to retransmit
	Timer m_RetransmitTimer;

	LocalDeliveryCallback GetLocalDeliveryCallback() const {return m_lcb;}
	void SetLocalDeliveryCallback (LocalDeliveryCallback lcb) {m_lcb = lcb;}


private:

	/// Data packet
	Ptr<Packet> m_packet;

	/// IP header
	Ipv6Header m_header;

	/// Unicast forward callback
	UnicastForwardCallback m_ucb;

	/// Error callback
	ErrorCallback m_ecb;

	/// Timestamp of when packet was sent
	Time m_SendTime;

	// Local Delivery callback
	LocalDeliveryCallback m_lcb;
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
	void RemoveRoutingQueueEntry(Ipv6Address source, Ipv6Address destination, Time sendTime);

	/**
	 * Lookup cache based on the source, destination and sendtime
	 * Tuple should be unique
	 */
	bool Lookup(Ipv6Address source, Ipv6Address destination, Time sendTime);

	/**
	 * Lookup cache based on the source, destination and sendtime
	 * Tuple should be unique
	 * Return routing entry
	 */
	ThesisInternetQueueEntry* GetRoutingEntry(Ipv6Address source, Ipv6Address destination, Time sendTime);

private:
	/// Container for the network routes - pair RipNgRoutingTableEntry *, EventId (update event)
	typedef std::list<std::pair <ThesisInternetQueueEntry *, EventId> > RoutingQueue;

  typedef std::list<std::pair <ThesisInternetQueueEntry *, EventId> >::iterator RoutingQueueI;

  RoutingQueue m_queue;

};

} /* namespace ns3 */
} /* namespace thesis */
#endif /* SRC_THESISINTERNETROUTING_MODEL_THESIS_INTERNET_ROUTING_QUEUE_H_ */
