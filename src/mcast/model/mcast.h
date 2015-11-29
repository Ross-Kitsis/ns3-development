/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MCAST_H
#define MCAST_H


/*NS3 L3 and code components*/
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/nstime.h"

//#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6.h"

//#include "ns3/ipv4-interface.h"
#include "ns3/ipv6-interface.h"

//#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"

#include "mcast-neighbor.h"
#include "mcast-packet.h"


#include <map>

namespace ns3
{
	namespace mcast
	{
		class RoutingProtocol : public Ipv6RoutingProtocol
		{
		public:

			static TypeId GetTypeId(void);
			static const uint32_t MCAST_PORT;

			RoutingProtocol();
			virtual ~RoutingProtocol();
			virtual void DoDispose();

		  // From Ipv6RoutingProtocol
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



		  //Set protocols parameters
		  void SetHelloEnable(bool f){EnableHello = f;}
		  bool GetHelloEnable () const { return EnableHello; }


		  //Assign streams to each node
		  int64_t AssignStreams (int64_t stream);


		protected:
		  virtual void DoInitialize(void);
		private:

		  uint16_t HelloInterval; //Interval between which nodes (may) send hello msgs
		  bool EnableHello; //Indicates if hello packets should be sent
		  bool EnableBroadcast; //Indicates whether nodes should send broadcast msgs to neighbors


		  //Unicast socket per IP interface, map socket to interface address
		  std::map<Ptr<Socket>,Ipv6InterfaceAddress> m_socketAddresses;
		  //Subnet directed broadcast for each interface (Use multiple interfaces?)
		  std::map< Ptr<Socket>, Ipv6InterfaceAddress > m_socketSubnetBroadcastAddresses;
		  /// Loopback device used to defer transmissions until packets fully formed
		  Ptr<NetDevice> m_lo;


		private:
		  //Start protocol operation
		  void Start();

		  ///Receive packets
		  //Receive hello packets from neighbors
		  void RecHello(Ptr<Socket> socket);

		  ///Send packets
		  //Send hello packet to all neighbors in range
		  void SendHello();

		  //Send packet via socket
		  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv6Address destination);

		  //Receive packets
		  void RecvMcast(Ptr<Socket> socket);

		  //Handle receiving hello packets
		  void RecvHello (Ptr<Packet> p, Ipv6Address receiver, Ipv6Address sender);

		  //Process hello messages
		  void ProcessHello(HelloHeader const & helloHeader, Ipv6Address receiver);

		  //Hello Time
		  Timer m_htimer;

		  //Schedule next hello message
		  void HelloTimerExpire();

		  //Get node position as a vector
		  Vector GetNodePosition (Ptr<Ipv6> ipv6);

		  //Get node position as a vector
		  Vector GetNodeVelocity (Ptr<Ipv6> ipv6);

		  //Uniform random variable provider
		  Ptr<UniformRandomVariable> m_uniformRandomVariable;

		  //Last broadcast time
		  Time m_lastHelloBcastTime;

		  /// Handle neighbors
		  Neighbors m_nb;

		  ///Multiple of hello interval to keep neighbor relationship alive
		  uint8_t m_hMult;

		  //Holdtime for neighbor
//		  Time m_NeighborLifetime;
		  uint16_t m_NeighborLifetime;

		  ///Internal pointer to IPv6
		  Ptr<Ipv6> m_ipv6;

		  ///mcast Radius
		  uint16_t m_radius;


		};
	}

}

#endif /* MCAST_H */

