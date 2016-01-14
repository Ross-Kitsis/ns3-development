/*
 * mcast2.h
 *
 *  Created on: Dec 2, 2015
 *      Author: ross
 */

#ifndef MCAST2_H_
#define MCAST2_H_

/*NS3 L3 and code components*/
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/nstime.h"

//IPv6 Routing
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-address.h"

//Interfaces & Sockets
#include "ns3/ipv6-interface.h"
#include "ns3/inet6-socket-address.h"

//L3 protocols
#include "ns3/ipv6-l3-protocol.h"

//MCAST packages
#include "ThesisNeighbors.h"
#include "mcast-packet.h"
#include "mcast-utils.h"
#include "ThesisPacketCache.h"
#include "tm-dpd.h"

//Routing tables
#include "ns3/ipv6-routing-table-entry.h"


//Standard files
#include <map>
#include <list>


/**
 * Setup tentative routing table entry (Subject to change as research moves foeward)
 *
 */

namespace ns3
{

namespace mcast
{

class ThesisRoutingTableEntry : public Ipv6RoutingTableEntry
{
public:
	/*
	 * Route status msgs
	 */
	enum Status_e
	{
		ROUTE_VALID,
		ROUTE_INVALID,
	};

	ThesisRoutingTableEntry (void);

	/**
	 * \brief Constructor
	 * \param network network address
	 * \param networkPrefix network prefix
	 * \param nextHop next hop address to route the packet
	 * \param interface interface index
	 * \param prefixToUse prefix that should be used for source address for this destination
	 */
	ThesisRoutingTableEntry (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse);

	/**
	 * \brief Constructor
	 * \param network network address
	 * \param networkPrefix network prefix
	 * \param interface interface index
	 */
	ThesisRoutingTableEntry (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface);

	virtual ~ThesisRoutingTableEntry ();

	/**
	 * \brief Set the route tag
	 * \param routeTag the route tag
	 */
	void SetRouteTag (uint16_t routeTag);

	/**
	 * \brief Get the route tag
	 * \returns the route tag
	 */
	uint16_t GetRouteTag (void) const;

	/**
	 * \brief Set the route metric
	 * \param routeMetric the route metric
	 */
	void SetRouteMetric (uint8_t routeMetric);

	/**
	 * \brief Get the route metric
	 * \returns the route metric
	 */
	uint8_t GetRouteMetric (void) const;

	/**
	 * \brief Set the route status
	 * \param status the route status
	 */
	void SetRouteStatus (Status_e status);

	/**
	 * \brief Get the route status
	 * \returns the route status
	 */
	Status_e GetRouteStatus (void) const;

	/**
	 * \brief Set the route as changed
	 *
	 * The changed routes are scheduled for a Triggered Update.
	 * After a Triggered Update, all the changed flags are cleared
	 * from the routing table.
	 *
	 * \param changed true if route is changed
	 */
	void SetRouteChanged (bool changed);

	/**
	 * \brief Get the route changed status
	 *
	 * \returns true if route is changed
	 */
	bool IsRouteChanged (void) const;
private:
	uint16_t m_tag; //!< route tag
	uint8_t m_metric; //!< route metric
	Status_e m_status; //!< route status
	bool m_changed; //!< route has been updated
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the reference to the output stream
 * \param route the Ipv6 routing table entry
 * \returns the reference to the output stream
 */
std::ostream& operator<< (std::ostream& os, ThesisRoutingTableEntry const& route);

class ThesisRoutingProtocol : public Ipv6RoutingProtocol
{
public:
	ThesisRoutingProtocol();
	virtual ~ThesisRoutingProtocol();

	static TypeId GetTypeId(void);

	//Implement from Ipv6RoutingProtocol
	Ptr<Ipv6Route> RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif,
			Socket::SocketErrno &sockerr);
	bool RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
			UnicastForwardCallback ucb, MulticastForwardCallback mcb,
			LocalDeliverCallback lcb, ErrorCallback ecb);
	virtual void NotifyInterfaceUp (uint32_t interface);
	virtual void NotifyInterfaceDown (uint32_t interface);
	virtual void NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address);
	virtual void NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address);
	virtual void NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());
	virtual void NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());
	virtual void SetIpv6 (Ptr<Ipv6> ipv6);
	virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;


  /**
   * \brief send Mcast Control Packet (Must be triggered by transport layer)
   *  Adds the mcast control header to the passed packet before sending it to the control
   *  multicast group.
   *
   */
  void DoSendMcastControl(Ptr<Packet> p);


	///////Skip split horizon implementation in Ripng/////////////////

	/**
	 * Assign a fixed random variable stream number to the random variables
	 * used by this model.  Return the number of streams (possibly zero) that
	 * have been assigned.
	 *
	 * \param stream first stream index to use
	 * \return the number of stream indices assigned by this model
	 */
	int64_t AssignStreams (int64_t stream);

	/**
	 * \brief Get the set of interface excluded from the protocol
	 * \return the set of excluded interfaces
	 */
	std::set<uint32_t> GetInterfaceExclusions () const; //Will be excluded

	/**
	 * \brief Set the set of interface excluded from the protocol
	 * \param exceptions the set of excluded interfaces
	 */
	void SetInterfaceExclusions (std::set<uint32_t> exceptions);

	/**
	 * \brief Get the metric for an interface
	 * \param interface the interface
	 * \returns the interface metric
	 */
	uint8_t GetInterfaceMetric (uint32_t interface) const;

	/**
	 * \brief Set the metric for an interface
	 * \param interface the interface
	 * \param metric the interface metric
	 */
	void SetInterfaceMetric (uint32_t interface, uint8_t metric);

	/**
	 * \brief Add a default route to the router through the nextHop located on interface.
	 *
	 * The default route is usually installed manually, or it is the result of
	 * some "other" routing protocol (e.g., BGP).
	 *
	 * \param nextHop the next hop
	 * \param interface the interface
	 */
	void AddDefaultRouteTo (Ipv6Address nextHop, uint32_t interface);

	/**
	 * \brief Checks if passed address is the same as one of the nodes interface addresses
	 */
	bool IsMyOwnAddress (Ipv6Address src);

protected:
	/**
	 * \brief Dispose this object.
	 */
	virtual void DoDispose ();

	/**
	 * Start protocol operation
	 */
	void DoInitialize ();

private:
	/// Container for the network routes - pair RipNgRoutingTableEntry *, EventId (update event)
	typedef std::list<std::pair <ThesisRoutingTableEntry *, EventId> > Routes;

	/// Const Iterator for container for the network routes
	typedef std::list<std::pair <ThesisRoutingTableEntry *, EventId> >::const_iterator RoutesCI;

	/// Iterator for container for the network routes
	typedef std::list<std::pair <ThesisRoutingTableEntry *, EventId> >::iterator RoutesI;

	/**
	 * \brief Receive Thesis packets.
	 *
	 * \param socket the socket the packet was received to.
	 */
	void Receive (Ptr<Socket> socket);

	/**
	 * \brief Lookup in the forwarding table for destination.
	 * \param dest destination address
	 * \param interface output interface if any (put 0 otherwise)
	 * \return Ipv6Route to route the packet to reach dest address
	 */
	Ptr<Ipv6Route> Lookup (Ipv6Address dest, Ptr<NetDevice> = 0);

	/**
	 * Receive and process unicast packet
	 * \param socket socket where packet is arrived
	 */
	void RecvUnicast (Ptr<Socket> socket);
	/**
	 * Receive and process multicast packet
	 * \param socket socket where packet is arrived
	 */
	void RecvMulticast (Ptr<Socket> socket);
	/**
	 * \brief Add route to network.
	 * \param network network address
	 * \param networkPrefix network prefix
	 * \param nextHop next hop address to route the packet.
	 * \param interface interface index
	 * \param prefixToUse prefix that should be used for source address for this destination
	 */
	void AddNetworkRouteTo (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse);

	/**
	 * \brief Add route to network.
	 * \param network network address
	 * \param networkPrefix network prefix
	 * \param interface interface index
	 */
	void AddNetworkRouteTo (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface);

	/**
	 * \brief Send Unsolicited Routing Updates on all interfaces.
	 * Convert this to hello
	 */
	void SendUnsolicitedRouteUpdate (void);
	/**
	 * \brief Invalidate a route.
	 * \param route the route to be removed
	 */
	void InvalidateRoute (ThesisRoutingTableEntry *route);

	/**
	 * \brief Delete a route.
	 * \param route the route to be removed
	 */
	void DeleteRoute (ThesisRoutingTableEntry *route);

	/**
	 * \brief Send hello message to all nodes in range in the mcast
	 * multicast group.
	 *
	 */
	void DoSendHello(void);

	/**
	 *\brief Hello timer expired, due to send a hello packet
	 */
	void HelloTimerExpire(void);

  /**
   * Get node position as a vector
   */
  Vector GetNodePosition (Ptr<Ipv6> ipv6);

  /**
   * Get node position as a vector
   */
  Vector GetNodeVelocity (Ptr<Ipv6> ipv6);

  /**
   * \brief Process hello headers and update neighbor table
   */
  void ProcessHello(HelloHeader helloHeader);

  /**
   * \brief Process control packet header
   */
  void ProcessMcastControl(ControlHeader cHeader, Ptr<Packet> packet);



  /**
   * \brief retransmits an mcast packet, assumes the retransmit backoff has expired
   * Expects a fully formed packet with all appropriate headers as argument
   */
  void DoSendMcastRetransmit(Ptr<Packet> packet);

	//Attributes

	Routes m_routes; //!<  the forwarding table for network.
	Ptr<Ipv6> m_ipv6; //!< IPv6 protocol reference
	Time m_startupDelay; //!< Random delay before protocol startup.
	Time m_unsolicitedUpdate; //!< time between two Unsolicited Routing Updates (Convert to hello msgs timer)


	// note: we can not trust the result of socket->GetBoundNetDevice ()->GetIfIndex ();
	// it is dependent on the interface initialization (i.e., if the loopback is already up).
	/// Socket list type
	typedef std::map< Ptr<Socket>, uint32_t> SocketList;
	/// Socket list type iterator
	typedef std::map<Ptr<Socket>, uint32_t>::iterator SocketListI;
	/// Socket list type const iterator
	typedef std::map<Ptr<Socket>, uint32_t>::const_iterator SocketListCI;

	//Need to change this??
	SocketList m_sendSocketList; //!< list of sockets for sending (socket, interface index)
	Ptr<Socket> m_recvSocket; //!< receive socket
	Ptr<Socket> m_mctrlSocket; //!< Receieve socket for

	Ptr<UniformRandomVariable> m_rng; //!< Rng stream.
	bool m_initialized; //!< flag to allow socket's late-creation.

	//Times for hello
	Time m_helloInterval; //Time between 2 hello messages
	Time m_neighborInvalid; //Max time between neighbor relationship invalidated
	Timer m_helloTimer; //Timer for hello messages; calls send hello when expires
	Ipv6Address m_globalAddress; //Pointer to GLOBAL ipv6 address (Assumes single interface)


	//Neighbors m_nb; //List of neighbors from whom hello messages have been received
	ThesisNeighbors m_neighbors;

	//Utilities
	McastUtils m_mutils;

	//Duplicate packet detection for mcast packets
	ThesisMcastDuplicatePacketDetection m_dpd;

	/**
	 * struct type_name {
	 * member_type1 member_name1;
	 * member_type2 member_name2;
	 * member_type3 member_name3;
	 * } object_names;
	 */

	/**
	 * \brief Struc to hold a packet and a timer; once timer expires the packet is sent
	 */
	struct McastRetransmit
	{
		Ptr<Packet> p;
		Timer timerToSend;
	};

	std::list<McastRetransmit> m_mr;

};

}//namespace mcast
}//namespace ns3
#endif /* SRC_MCAST_MODEL_MCAST2_H_ */
