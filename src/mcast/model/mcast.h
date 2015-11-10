/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MCAST_H
#define MCAST_H


/*NS3 L3 and code components*/
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"

//#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6-routing-protocol.h"

//#include "ns3/ipv4-interface.h"
#include "ns3/ipv6-interface.h"

//#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"


#include <map>

namespace ns3
{
	namespace mcast
	{
		class RoutingProtocol : public Ipv6RoutingProtocol
		{
		public:

			static TypeId GetTypeID(void);
			static const uint32_t MCAST_PORT;

			RoutingProtocol();
			virtual ~RoutingProtocol();
			virtual void DoDispose();

			//Inherit from IPv6 Routing (virtual methods in header)
		  Ptr<Ipv6Route> RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
		  bool RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		                     UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		                     LocalDeliverCallback lcb, ErrorCallback ecb);
		  virtual void NotifyInterfaceUp (uint32_t interface);
		  virtual void NotifyInterfaceDown (uint32_t interface);
		  virtual void NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address);
		  virtual void NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address);
		  virtual void SetIpv6 (Ptr<Ipv6> ipv6);
		  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

		  //Set protocols parameters
		  //void SetHelloEnable(bool f){EnableHello = f;}

		protected:
		  virtual void DoInitialize(void);
		private:

		  Time HelloInterval; //Interval between which nodes (may) send hello msgs
		  bool EnableBroadcast; //Indicates whether nodes should send broadcast msgs to neighbors

		  //Ip Protocol
		  Ptr<Ipv6> m_ipv6;
		  //Unicast socket per IP interface, map socket to interface address
		  std::map<Ptr<Socket>,Ipv6InterfaceAddress> m_socketAddresses;
		  //Subnet directed broadcast for each interface (Use multiple interfaces?)
		  std::map< Ptr<Socket>, Ipv6InterfaceAddress > m_socketSubnetBroadcastAddresses;
		  /// Loopback device used to defer transmissions until packets fully formed
		  Ptr<NetDevice> m_lo;

		};
	}

}

#endif /* MCAST_H */

