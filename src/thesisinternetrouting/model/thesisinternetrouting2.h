/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef THESISINTERNETROUTING_H
#define THESISINTERNETROUTING_H

/*NS3 L3 and code components*/
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/nstime.h"

//IPv6 Routing
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-address.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"

//Interfaces & Sockets
#include "ns3/ipv6-interface.h"
#include "ns3/inet6-socket-address.h"

//L3 protocols
#include "ns3/ipv6-l3-protocol.h"

//Routing tables
#include "ns3/ipv6-routing-table-entry.h"

//Standard files
#include <map>
#include <list>
#include <math.h>

//Mcast files
#include "ns3/Db.h"
#include "ns3/mcast-packet.h"
#include "ns3/mcast-utils.h"


//ThesisFiles
#include "InternetHeader.h"
#include "Thesis-Internet-Routing-Queue.h"
#include "RsuCache.h"

namespace ns3
{
namespace thesis
{

class ThesisInternetRoutingTableEntry2 : public Ipv6RoutingTableEntry
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

	ThesisInternetRoutingTableEntry2 (void);

	/**
	 * \brief Constructor
	 * \param network network address
	 * \param networkPrefix network prefix
	 * \param nextHop next hop address to route the packet
	 * \param interface interface index
	 * \param prefixToUse prefix that should be used for source address for this destination
	 */
	ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse);

	/**
	 * \brief Constructor
	 * \param network network address
	 * \param networkPrefix network prefix
	 * \param interface interface index
	 */
	ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface);

	/**
	 * \brief Constructor
	 * \param network network address
	 * \param networkPrefix network prefix
	 * \param nextHop next hop address to route the packet
	 * \param interface interface index
	 * \param prefixToUse prefix that should be used for source address for this destination
	 * \param RSU address where to stop routing
	 */
	ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse, Ipv6Address RsuAddress);


	virtual ~ThesisInternetRoutingTableEntry2 ();

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
	 * \brief Set the RSU address
	 */
	void SetRsuAddress(Ipv6Address RsuAddress);

	/**
	 * \brief Get RSU address
	 */
	Ipv6Address GetRsuAddress(void) const;

private:
	uint16_t m_tag; //!< route tag
	uint8_t m_metric; //!< route metric
	Status_e m_status; //!< route status
	bool m_changed; //!< route has been updated
	Ipv6Address m_RsuAddress;

};

class ThesisInternetRoutingProtocol2 : public Ipv6RoutingProtocol
{
public:
	ThesisInternetRoutingProtocol2();
	virtual ~ThesisInternetRoutingProtocol2();

	static TypeId GetTypeId(void);

	//Implement from Ipv6RoutingProtocol
	Ptr<Ipv6Route> RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif,
			Socket::SocketErrno &sockerr);
	bool RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
			UnicastForwardCallback ucb, MulticastForwardCallback mcb,
			LocalDeliverCallback lcb, ErrorCallback ecb);
	void NotifyInterfaceUp (uint32_t interface);
	void NotifyInterfaceDown (uint32_t interface);
	void NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address);
	void NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address);
	void NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());
	void NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse = Ipv6Address::GetZero ());
	void SetIpv6 (Ptr<Ipv6> ipv6);
	void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

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
   * \brief Add route to network.
   * \param network network address
   * \param networkPrefix network prefix
   * \param nextHop next hop address to route the packet.
   * \param interface interface index
   * \param prefixToUse prefix that should be used for source address for this destination
   * \param RSU address where routing is being directed towards
   */
  void AddNetworkRouteTo (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse, Ipv6Address RsuAddress);

  /**
	 *\brief Sets the node IP based on the current zone
	 *\brief Initial address MUST be bootstrapped with a dummy IPv6 address
	 *\brief Address will be changed once simulation starts.
	 */
	void SetIpToZone();

	/**
	 * \brief Sets pointer to database
	 */
	void SetRsuDatabase(Ptr<Db> db);

	/**
	 * \brief Sets flag is protocol is running on RSU or on a node
	 */
	void SetIsRSU(bool isRSU);

	/**
	 *
	 * \brief Sets time interval between position checks to see if IP needs to change
	 */
	void SetCheckPositionTime(Time t);

	/**
	 * \brief Get time between 2 successive position checks
	 */
	Time GetCheckPositionTime();

	/**
	 * Removes the default route from the list of routes currently stored
	 * Used when position changes
	 */
	void RemoveDefaultRoute();

protected:
	/**
	 * \brief Dispose this object.
	 */
	virtual void DoDispose ();

	/**
	 * Start protocol operation
	 */
	void DoInitialize ();

	/*
	 * RouteInput for VANET nodes
	 */
	bool RouteInputVanet (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
			UnicastForwardCallback ucb, MulticastForwardCallback mcb,
			LocalDeliverCallback lcb, ErrorCallback ecb);

	/**
	 * RouteInput for RSU
	 */
	bool RouteInputRsu (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
			UnicastForwardCallback ucb, MulticastForwardCallback mcb,
			LocalDeliverCallback lcb, ErrorCallback ecb);

	/**
	 * Delete Route
	 * Schedule deletion of routes when an IP address is changed
	 *
	 */
	void DeleteRoute(ThesisInternetRoutingTableEntry2 *route);



private:

	/**
	 * Removes the default route from the list of routes currently stored
	 * Used when position changes
	 */
	void SetInterfacePointers();

	/**
	 * Assumes list routing protocol is installed on RSU
	 * Sets the pointer to the static route helper containing routes to the hub
	 */
	void SetStaticRoutePointer();

	/**
	 * Returns the first interface number on w wifi interface; else -1
	 */
	int32_t GetWirelessInterface();

	/**
	 * Returns the first interface number on w p2p interface; else -1
	 */
	int32_t GetP2pInterface();

	/**
	 * \brief Pointer to IPv6 protocol on node
	 */
	Ptr<Ipv6> m_ipv6;

	/**
	 * \brief Pointer to database containing RSU locations
	 */
	Ptr<Db> m_Db;

	/**
	 * \brief Boolean value indicating if the node is running the mcast protocol as well as this protocol
	 */
	bool m_hasMcast;

	/**
	 * Pointer to the loopback netdevice
	 */
	Ptr<NetDevice> m_lo;

	/**
	 * Pointer to the wifi netdevice
	 */
	Ptr<NetDevice> m_wi;

	/**
	 * Pointer to the point2point netdevice
	 */
	Ptr<NetDevice> m_pp;

	/**
	 * Pointer to Ipv6StaticRouting (Used by RSU)
	 */
	Ptr<Ipv6StaticRouting> m_sr6;

	/// Container for the network routes - pair RipNgRoutingTableEntry *, EventId (update event)
	typedef std::list<std::pair <ThesisInternetRoutingTableEntry2 *, EventId> > Routes;

	//Iterator for routes
  typedef std::list<std::pair <ThesisInternetRoutingTableEntry2 *, EventId> >::iterator RoutesI;

	//Const Iterator for routes
  typedef std::list<std::pair <ThesisInternetRoutingTableEntry2 *, EventId> >::const_iterator RoutesIC;


	Routes m_routes; //!<  the forwarding table for network.

	/**
	 * Flag notifying if node is an RSU or a vehicle node
	 */
	bool m_IsRSU;

	/**
	 * \brief Time between checking the node position to determine if address needs to be changed
	 */
	Time m_CheckPosition;

	/**
	 * \brief Timer to countdown till VANET node must recheck its zone and adjust IP address if required
	 */
	Timer m_CheckPositionTimer;

	/**
	 * Search current routes for a route to the destination.
	 * Returns a valid route if found; returns a blank route if no route found.
	 *
	 */
	Ptr<Ipv6Route> Lookup(Ipv6Address destination, Ptr<NetDevice>);

	/**
	 * Flag controlling if messages are delay tolerant
	 * Not actively used in current implementation but it is sent in internet header to allow for future expansion
	 * Set to false by default.
	 */
	bool m_IsDtnTolerant;

	/**
	 * Current RSU address; set by the lookup method
	 * Used to determine where the packet is being routed towards
	 */
	Ipv6Address m_RsuDestination;

	/**
	 * Pointer to the RSU cache
	 * Used to manage database of entries coming through the RSU
	 * Not used on VANET nodes
	 */
	RsuCache m_RsuCache;

	/**
	 * Calculate the backoff duration before retransmitting a packet
	 * Return the backoff time in microseconds
	 */
	Time GetBackoffDuration(Vector SenderPosition);

	/**
	 * Pointer to the internet routing queue
	 * Used by VANET nodes to manage retransmissions
	 *
	 */
	ThesisInternetRoutingQueue m_RoutingCache;

	/*
	 * Utilities created in mcast routing protocol
	 * Primarily used to find distances between points
	 */
	mcast::McastUtils utils;

	/**
	 * Check if sender position is effective
	 */
	bool IsEffective(Vector SenderPosition);

	/**
	 * DbEntry pointer to the current RSU
	 * Used to avoid additional processing
	 */
	DbEntry m_currentRsu;

	/*
	 * Strictly effective nodes will not retransmit if they are further from the RSU than the last sending node
	 * Non-strictly effective nodes MAY retransmit but do not have to
	 */
	bool m_isStrictEffective;

	/*
	 * wait time multiplier
	 */
	uint64_t m_rWait;

	/**
	 * Retransmits a packet from the routing queue once timer has expired
	 * Arguments define a unique routing queue entry
	 * Before retransmitting the entry is removed from the queue
	 */
	void SendInternetRetransmit(Ipv6Address source, Ipv6Address destination, Time sendTime);

	/**
	 * Send Ack message after RSU receives msg on wifi
	 */
	void SendAckMessage(Ptr<Packet> ack, UnicastForwardCallback ucb);

};

}
}

#endif /* THESISINTERNETROUTING_H */

