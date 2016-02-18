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
			ErrorCallback ecb = ErrorCallback (), Time exp = Simulator::Now ()) :
				m_packet (pa), m_header (h), m_ucb (ucb), m_ecb (ecb),
				m_expire (exp + Simulator::Now ())
	{}

	/**
	 * Compare queue entries
	 * \return true if equal
	 */
	bool operator== (ThesisInternetQueueEntry const & o) const
  		{
		return ((m_packet == o.m_packet) && (m_header.GetDestinationAddress() == o.m_header.GetDestinationAddress ()) &&
				    (m_expire == o.m_expire));
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
	void SetExpireTime (Time exp) { m_expire = exp + Simulator::Now (); }
	Time GetExpireTime () const { return m_expire - Simulator::Now (); }

private:

	/// Data packet
	Ptr<const Packet> m_packet;
	/// IP header
	Ipv6Header m_header;
	/// Unicast forward callback
	UnicastForwardCallback m_ucb;
	/// Error callback
	ErrorCallback m_ecb;
	/// Expire time for queue entry
	Time m_expire;
};


class ThesisInternetRoutingQueue
{
public:


	ThesisInternetRoutingQueue();
	virtual ~ThesisInternetRoutingQueue();



};

} /* namespace ns3 */
} /* namespace thesis */
#endif /* SRC_THESISINTERNETROUTING_MODEL_THESIS_INTERNET_ROUTING_QUEUE_H_ */
