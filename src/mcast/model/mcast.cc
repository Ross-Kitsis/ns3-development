/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "mcast.h"
//#include "mcast-neighbor.h"

#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h" //Needed for hellos?
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/ipv6-route.h"

#include "ns3/mobility-model.h"

#include <algorithm>
#include <limits>
#include <iostream>

//#define MCAST_ALL_NODE "ff02::114"
#define MCAST_ALL_NODE "2001::200"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("MobicastProtocol");
namespace mcast
{
NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);

const uint32_t RoutingProtocol::MCAST_PORT = 701;

RoutingProtocol::RoutingProtocol () :
	  				HelloInterval (3), //Hello broadcast interval
	  				EnableHello (true), //Enable hello msgs
	  				EnableBroadcast(true), //Enable broadcasts
	  				m_lastHelloBcastTime (Seconds (0)), //Initialize last broadcast time to 0
	  				m_nb (Seconds (HelloInterval)), //Initialize neighbor broadcasting every hello interval seconds
	  				m_hMult(5),
	  				m_NeighborLifetime(m_hMult * HelloInterval),
	  				m_ipv6 (0),
	  				m_radius(50)
{

}

RoutingProtocol::~RoutingProtocol ()
{
}

TypeId
RoutingProtocol::GetTypeId ()
{


	static TypeId tid = TypeId ("ns3::mcast::RoutingProtocol")
    		.SetParent<Ipv6RoutingProtocol> ()
    		.SetGroupName("mcast")
    		.AddConstructor<RoutingProtocol> ()
        .AddAttribute ("UniformRv",
                       "Access to the underlying UniformRandomVariable",
                       StringValue ("ns3::UniformRandomVariable"),
                       MakePointerAccessor (&RoutingProtocol::m_uniformRandomVariable),
                       MakePointerChecker<UniformRandomVariable> ())
        .AddAttribute ("EnableHello", "Indicates whether a hello messages enable.",
                       BooleanValue (true),
                       MakeBooleanAccessor (&RoutingProtocol::SetHelloEnable,
                       &RoutingProtocol::GetHelloEnable),
                                      MakeBooleanChecker ())
    		;

	return tid;

}

void
RoutingProtocol::Start ()
{
	NS_LOG_FUNCTION (this);
	//if (EnableHello)
	//{
		m_nb.ScheduleTimer ();
	//}
	//m_rreqRateLimitTimer.SetFunction (&RoutingProtocol::RreqRateLimitTimerExpire, this);
	//m_rreqRateLimitTimer.Schedule (Seconds (1));

	//m_rerrRateLimitTimer.SetFunction (&RoutingProtocol::RerrRateLimitTimerExpire, this);
	//m_rerrRateLimitTimer.Schedule (Seconds (1));

}

Ptr<Ipv6Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
	NS_LOG_FUNCTION (this);
	Ipv6Address destination = header.GetDestinationAddress();
  Ptr<Ipv6Route> rtentry = 0;



  if (destination.IsMulticast ())
    {
      // Note:  Multicast routes for outbound packets are stored in the
      // normal unicast table.  An implication of this is that it is not
      // possible to source multicast datagrams on multiple interfaces.
      // This is a well-known property of sockets implementation on
      // many Unix variants.
      // So, we just log it and fall through to LookupStatic ()
      NS_LOG_LOGIC ("		RouteOutput (): Multicast destination" << destination);
    }
  NS_LOG_LOGIC("Net device " << oif->GetAddress());
  rtentry = Lookup (destination, oif);
  if (rtentry)
    {
      sockerr = Socket::ERROR_NOTERROR;
    }
  else
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
    }
  return rtentry;
}

Ptr<Ipv6Route>
RoutingProtocol::Lookup (Ipv6Address dst, Ptr<NetDevice> interface)
{
	NS_LOG_FUNCTION (this << dst << interface);
  Ptr<Ipv6Route> rtentry = 0;
//  uint16_t longestMask = 0;
  if(true)
//  if(dst.IsMulticast() || dst.IsEqual(Ipv6Address(MCAST_ALL_NODE)))
  {
  	NS_LOG_LOGIC (this);
  	NS_LOG_LOGIC (dst);
//  	NS_LOG_LOGIC (interface->GetAddress() << " found as multicast destination");

  	rtentry = Create<Ipv6Route> ();
    rtentry->SetSource (m_ipv6->SourceAddressSelection (m_ipv6->GetInterfaceForDevice (interface), dst));
    rtentry->SetDestination (dst);
    rtentry->SetGateway (Ipv6Address::GetZero ());
    rtentry->SetOutputDevice (interface);

//    NS_LOG_LOGIC ("Returning route with : " << );

    return rtentry;
  }

  return rtentry;
}

/*
 *
 */
bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{

	NS_LOG_FUNCTION (this);

	bool toReturn = true;

	Ipv6Address dst = header.GetDestinationAddress();
	Ipv6Address src = header.GetSourceAddress();
	uint32_t iif = m_ipv6->GetInterfaceForDevice(idev);


	NS_LOG_LOGIC ("		Route input destination: " << dst << " Source: " << src);

	if(IsMyOwnAddress(src))
	{
		NS_LOG_LOGIC("Packet from own address" << src);
		return false;
	}

	/*
	if(lcb.IsNull() == false)
	{
		NS_LOG_LOGIC("LCB NOT NULL");
	}else
	{
		NS_LOG_LOGIC("LCB NULL");
	}*/


	//uint32_t ri = m_recvSocket->GetBoundNetDevice()->GetIfIndex();

	if(true)
	{
		NS_LOG_LOGIC("	Multicast packet detected, destination: " << dst << " interface  " << iif /* << " recvSocket int " << ri*/);
		//NS_LOG_LOGIC("Multicast delivery to " << iface);


		lcb (p, header, iif);
		return true;
	}


	return toReturn;
}

/*
 * Checks passed address against all registered sockets to determine
 * if sending address if nodes own address
 */
bool
RoutingProtocol::IsMyOwnAddress (Ipv6Address src)
{
  NS_LOG_FUNCTION (this << src);
  for (std::map<Ptr<Socket>, Ipv6InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv6InterfaceAddress iface = j->second;
      if (src == iface.GetAddress())
        {
          return true;
        }
    }
  return false;
}


void
RoutingProtocol::NotifyInterfaceUp (uint32_t interface)
{

	NS_LOG_FUNCTION (this);

  //Create socket for hello msgs
  NS_LOG_LOGIC ("MCAST: adding hello socket to " << MCAST_ALL_NODE << " INIT - HELLO");
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Node> theNode = GetObject<Node> ();
  m_recvSocket = Socket::CreateSocket (theNode, tid);
  Inet6SocketAddress local = Inet6SocketAddress (Ipv6Address(MCAST_ALL_NODE), MCAST_PORT);
  m_recvSocket->Bind (local);
  if(Ipv6Address(MCAST_ALL_NODE).IsMulticast())
  {
  	NS_LOG_LOGIC("Socket has a multicast address");
  	Ptr<UdpSocket> UdpSocketTest = DynamicCast<UdpSocket>(m_recvSocket);
  	if(UdpSocketTest)
  	{
  		UdpSocketTest -> MulticastJoinGroup(0,Ipv6Address(MCAST_ALL_NODE));
  	}else
  	{
  		NS_FATAL_ERROR ("Error: Failed to join multicast group");
  	}
  }
  m_recvSocket->SetRecvCallback (MakeCallback (&RoutingProtocol::Receive, this));
  //m_recvSocket->BindToNetDevice (m_ipv6->GetNetDevice (0));// Not sure if want this
  m_recvSocket->SetRecvPktInfo (true);


	for (uint32_t i = 0 ; i < m_ipv6->GetNInterfaces (); i++)
	{
		for(uint32_t j = 0; j < m_ipv6->GetNAddresses(i); j++)
		{
			Ipv6InterfaceAddress address = m_ipv6->GetAddress(i,j);
			if(address.GetScope() != Ipv6InterfaceAddress::LINKLOCAL && address.GetScope() != Ipv6InterfaceAddress::HOST)
			{
				NS_LOG_LOGIC ("MCAST: adding socket to " << address.GetAddress () << " INIT - ACTIVE");
				TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
				Ptr<Node> theNode = GetObject<Node> ();
				Ptr<Socket> socket = Socket::CreateSocket (theNode, tid);
				Inet6SocketAddress global = Inet6SocketAddress (address.GetAddress (), MCAST_PORT);
				socket->SetRecvCallback (MakeCallback (&RoutingProtocol::Receive,this));
				int ret = socket->Bind (global);
			  if(Ipv6Address(MCAST_ALL_NODE).IsMulticast())
			  {
			  	NS_LOG_LOGIC("Socket has a multicast address");
			  	Ptr<UdpSocket> UdpSocketTest = DynamicCast<UdpSocket>(socket);
			  	if(UdpSocketTest)
			  	{
			  		UdpSocketTest -> MulticastJoinGroup(0,Ipv6Address(MCAST_ALL_NODE));
			  	}else
			  	{
			  		NS_FATAL_ERROR ("Error: Failed to join multicast group");
			  	}
			  }
				NS_ASSERT_MSG (ret == 0, "Bind unsuccessful");
        //socket->BindToNetDevice (m_ipv6->GetNetDevice (i));// Not sure if want this
				socket->SetAllowBroadcast(true);
				m_socketAddresses.insert (std::make_pair (socket, address));
			}
		}
	}

	/*

	NS_LOG_FUNCTION (this << m_ipv6->GetAddress (interface, 0));
	Ptr<Ipv6L3Protocol> l3 = m_ipv6->GetObject<Ipv6L3Protocol> ();



	Ipv6InterfaceAddress iface = l3->GetAddress (interface, 0);
	if (iface.GetAddress() == Ipv6Address ("::1"))
		return;

  NS_LOG_LOGIC ("RIPng: adding sending socket to " << l3->GetAddress(0,0));

	// Create a socket to listen only on this interface
	Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
			UdpSocketFactory::GetTypeId ());
	NS_ASSERT (socket != 0);
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvMcast, this));
	socket->Bind (Inet6SocketAddress (Ipv6Address::GetAllNodesMulticast(), MCAST_PORT));
	socket->BindToNetDevice (l3->GetNetDevice (interface));
	socket->SetAllowBroadcast (true);
	socket->SetAttribute ("IpTtl", UintegerValue (1));
	m_socketAddresses.insert (std::make_pair (socket, iface));

*/

	/* Skip this??

	// create also a subnet broadcast socket
	socket = Socket::CreateSocket (GetObject<Node> (),
			UdpSocketFactory::GetTypeId ());
	NS_ASSERT (socket != 0);
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvAodv, this));
	socket->Bind (InetSocketAddress (iface.GetBroadcast (), AODV_PORT));
	socket->BindToNetDevice (l3->GetNetDevice (i));
	socket->SetAllowBroadcast (true);
	socket->SetAttribute ("IpTtl", UintegerValue (1));
	m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

	 */



//	Ptr<NetDevice> dev = m_ipv6->GetNetDevice (m_ipv6->GetInterfaceForAddress (iface.GetAddress ()));

	//////////////////////////////////// Update later, no routing table yet


	// Add local broadcast record to the routing table
	//
	//	RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*know seqno=*/ true, /*seqno=*/ 0, /*iface=*/ iface,
	//			/*hops=*/ 1, /*next hop=*/ iface.GetBroadcast (), /*lifetime=*/ Simulator::GetMaximumSimulationTime ());
	//	m_routingTable.AddRoute (rt);

	//	if (l3->GetInterface (i)->GetArpCache ())
	//	{
	//		m_nb.AddArpCache (l3->GetInterface (i)->GetArpCache ());
	//	}

	//////////////////////////////////////////////////////////////////////////

	/*

	// Allow neighbor manager use this interface for layer 2 feedback if possible
	Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
	if (wifi == 0)
		return;
	Ptr<WifiMac> mac = wifi->GetMac ();
	if (mac == 0)
		return;

	mac->TraceConnectWithoutContext ("TxErrHeader", m_nb.GetTxErrorCallback ());
	*/
}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t interface)
{
	NS_LOG_FUNCTION (this);
}

/**
 * Notify node the interface is up and running with the given address
 */
void
RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{

	NS_LOG_FUNCTION(this << " interface " << interface << " address " << address);

	/*
	Ptr<Ipv6L3Protocol> l3 = m_ipv6->GetObject<Ipv6L3Protocol> ();
  if (!l3->IsUp (interface))
    return;
  if(!l3->GetNAddresses(interface) == 1)
  {
    Ipv6InterfaceAddress iface = l3->GetAddress (interface, 0);

    //Create socket to listen on this interface with given port
    Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                               UdpSocketFactory::GetTypeId ());
    NS_ASSERT (socket != 0);
              socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvMcast,this));
              socket->Bind (Inet6SocketAddress (iface.GetAddress(), MCAST_PORT));
              socket->BindToNetDevice (l3->GetNetDevice (interface));
              socket->SetAllowBroadcast (true);
              m_socketAddresses.insert (std::make_pair (socket, iface));
  }
  */
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION (this);
}

//TO DO ASAP
void
RoutingProtocol::Receive(Ptr<Socket> socket)
{

	NS_LOG_FUNCTION (this << socket);
	Address sourceAddress;
	Ptr<Packet> packet = socket->Recv();//    RecvFrom (sourceAddress);
	Inet6SocketAddress inetSourceAddr = Inet6SocketAddress::ConvertFrom (sourceAddress);
	Ipv6Address sender = inetSourceAddr.GetIpv6();
	Ipv6Address receiver;

	/*

	if (m_socketAddresses.find (socket) != m_socketAddresses.end ())
	{
		receiver = m_socketAddresses[socket].GetLocal ();
	}
	else if(m_socketSubnetBroadcastAddresses.find (socket) != m_socketSubnetBroadcastAddresses.end ())
	{
		receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal ();
	}
	else
	{
		NS_ASSERT_MSG (false, "Received a packet from an unknown socket");
	}
	*/
	//receiver = m_socketAddresses[socket].GetAddress();
	NS_LOG_DEBUG ("Thesis node " << this << " received a MCAST packet from " << sender << " to " << receiver);


//	UpdateRouteToNeighbor (sender, receiver);
	TypeHeader tHeader (HELLO);
	packet->RemoveHeader (tHeader);


	if (!tHeader.IsValid ())
	{
		//Unknown packet type
		NS_LOG_DEBUG ("MCAST message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
		return; // drop
	}
	switch (tHeader.Get ())
	{
	case HELLO:
	{
		RecvHello (packet);
		break;
	}
	case MCAST_CONTROL:
	{
		NS_LOG_DEBUG("		Receieved mcast control");
		break;
	}
	}
}

/*
 * Receive hello msgs
 */
void
RoutingProtocol::RecvHello (Ptr<Packet> p)
{
	NS_LOG_FUNCTION (this << " Receiving hello message");
	HelloHeader HelloHeader;
	p->RemoveHeader (HelloHeader);
	Ipv6Address dst = HelloHeader.GetDst ();

	//NS_LOG_LOGIC (this << " processing hello from  " << HelloHeader.GetOrigin ());

	uint8_t hop = HelloHeader.GetHopCount () + 1;
	HelloHeader.SetHopCount (hop);

	ProcessHello (HelloHeader);
	return;

}


void
RoutingProtocol::ProcessHello(HelloHeader const & helloHeader)
{
	NS_LOG_FUNCTION (this << " processing hello message ");


	/*
	 *  Whenever a node receives a Hello message from a neighbor, the node
	 * SHOULD make sure that it has an active route to the neighbor, and
	 * create one if necessary.
	 */
//	RoutingTableEntry toNeighbor;
//	if (!m_routingTable.LookupRoute (rrepHeader.GetDst (), toNeighbor))
//	{
//		Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (receiver));
//		RoutingTableEntry newEntry (/*device=*/ dev, /*dst=*/ rrepHeader.GetDst (), /*validSeqNo=*/ true, /*seqno=*/ rrepHeader.GetDstSeqno (),
//				/*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (receiver), 0),
//				/*hop=*/ 1, /*nextHop=*/ rrepHeader.GetDst (), /*lifeTime=*/ rrepHeader.GetLifeTime ());
//		m_routingTable.AddRoute (newEntry);
//	}
//	else
//	{
//		toNeighbor.SetLifeTime (std::max (Time (AllowedHelloLoss * HelloInterval), toNeighbor.GetLifeTime ()));
//		toNeighbor.SetSeqNo (rrepHeader.GetDstSeqno ());
//		toNeighbor.SetValidSeqNo (true);
//		toNeighbor.SetFlag (VALID);
//		toNeighbor.SetOutputDevice (m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (receiver)));
//		toNeighbor.SetInterface (m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (receiver), 0));
//		toNeighbor.SetHop (1);
//		toNeighbor.SetNextHop (rrepHeader.GetDst ());
//		m_routingTable.Update (toNeighbor);
//	}



	//Ipv6Address addr, Time expire, Vector velocity, Vector position
	m_nb.Update (helloHeader.GetOrigin(), Time (m_NeighborLifetime), helloHeader.GetVelocity(), helloHeader.GetPosition());

}

/**
 * Seems to generally be unused in implementations but is virtual defined in IPv6 route
 */
void
RoutingProtocol::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION (this << dst << mask << nextHop << interface << prefixToUse);
	// \todo this can be used to add delegate routes
}

/**
 * Seems to generally be unused in implementations but is virtual defined in IPv6 route
 */
void
RoutingProtocol::NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION (this << dst << mask << nextHop << interface);
	// \todo this can be used to delete delegate routes
}


void
RoutingProtocol::SetIpv6 (Ptr<Ipv6> ipv6)
{
	NS_LOG_FUNCTION (this);
	NS_ASSERT (ipv6 != 0);
	NS_ASSERT (m_ipv6 == 0);

	m_ipv6 = ipv6;

	// Create lo route. It is asserted that the only one interface up for now is loopback
	NS_ASSERT (m_ipv6->GetNInterfaces () == 1 && m_ipv6->GetAddress (0, 0).GetAddress() == Ipv6Address ("::1"));
	m_lo = m_ipv6->GetNetDevice (0);
	//NS_ASSERT (m_lo != 0);


	///////////////// TO BE UPDATED LATER /////////////////////////////

	// Remember lo route
	//  RoutingTableEntry rt (/*device=*/ m_lo, /*dst=*/ Ipv4Address::GetLoopback (), /*know seqno=*/ true, /*seqno=*/ 0,
	//                                    /*iface=*/ Ipv4InterfaceAddress (Ipv4Address::GetLoopback (), Ipv4Mask ("255.0.0.0")),
	//                                    /*hops=*/ 1, /*next hop=*/ Ipv4Address::GetLoopback (),
	//                                    /*lifetime=*/ Simulator::GetMaximumSimulationTime ());
	//  m_routingTable.AddRoute (rt);

	//////////////////////////////////////////////////////////////////

	Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

int64_t
RoutingProtocol::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
	NS_LOG_FUNCTION (this);
}



void
RoutingProtocol::DoDispose ()
{
	NS_LOG_FUNCTION (this);

}


void
RoutingProtocol::HelloTimerExpire()
{

	/*
	NS_LOG_FUNCTION (this);
	Time offset = Time (Seconds (0));
	if (m_lastHelloBcastTime > Time (Seconds (0)))
	{
		offset = Simulator::Now () - m_lastHelloBcastTime;
		NS_LOG_DEBUG ("Hello deferred due to last bcast at:" << m_lastHelloBcastTime);
	}
	else
	{
		SendHello ();
	}
	m_htimer.Cancel ();
	Time diff = Seconds(HelloInterval) - offset;
	m_htimer.Schedule (std::max (Time (Seconds (0)), diff));
	m_lastHelloBcastTime = Time (Seconds (0));
	*/


  std::cout << "Sending hello and scheduling next hello " << this << std::endl;
	NS_LOG_FUNCTION (this << HelloInterval);

	m_htimer.Cancel();

	m_htimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
	m_htimer.Schedule(Time(Seconds(HelloInterval + m_uniformRandomVariable->GetInteger(0,2))));
	SendHello();

}

void
RoutingProtocol::SendHello ()
{
	NS_LOG_FUNCTION (this);

	Ptr<Socket> socket;
	Ipv6InterfaceAddress iface;

	for (std::map<Ptr<Socket>, Ipv6InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		socket = j->first;
		iface = j->second;
		break;
	}


	//Get node position
	Vector pos = GetNodePosition(m_ipv6);

	//Get node velocity
	Vector vel = GetNodeVelocity(m_ipv6);

	//Get Road ID
	uint64_t roadId = 0;

	//hopCount
	uint8_t hopCount = 0;

	uint8_t type = 1;

	uint16_t reserved = 0;

	//Create hello header
	HelloHeader helloHeader(/*Type*/ type, /*roadId*/ roadId, /*roadId*/ hopCount, /*neighbor life time */m_NeighborLifetime, /*radius*/ m_radius,reserved, /**/Ipv6Address().GetAllNodesMulticast(), /**/iface.GetAddress(), /*Position*/ pos, /*Velocity*/ vel );

	Ipv6Address destination = Ipv6Address(MCAST_ALL_NODE);

	NS_LOG_LOGIC("Sending packet to: " << destination << " from address " << iface.GetAddress());

	Ptr<Packet> packet = Create<Packet>();
	packet->AddHeader(helloHeader);
	TypeHeader theader (HELLO);
	packet->AddHeader(theader);

	//Set Jitter time before sending
	Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
	//Time delay = Time(Seconds(m_uniformRandomVariable->GetInteger(0,2)));
	Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this , socket, packet, destination);

}

void
RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv6Address destination)
{
	NS_LOG_FUNCTION (this << destination);
	socket->SendTo (packet, 0, Inet6SocketAddress (destination, MCAST_PORT));
	//socket->Send(packet);
	//socket->Send(packet);
}

Vector
RoutingProtocol::GetNodePosition (Ptr<Ipv6> ipv6)
{
	NS_LOG_FUNCTION (this);
	Vector pos = ipv6->GetObject<MobilityModel>()->GetPosition();
	NS_LOG_DEBUG (" Node " << ipv6->GetObject<Node>()->GetId() << " position =" << pos);
	return pos;
}

Vector
RoutingProtocol::GetNodeVelocity (Ptr<Ipv6> ipv6)
{
	NS_LOG_FUNCTION (this);
	Vector vel = ipv6->GetObject<MobilityModel>()->GetVelocity();
	NS_LOG_DEBUG (" Node " << ipv6->GetObject<Node>()->GetId() << " velocity =" << vel);
	return vel;
}

void
RoutingProtocol::DoInitialize ()
{
	NS_LOG_FUNCTION (this);
	/////////////////////////////////Bind sockets

	NS_LOG_LOGIC("Adding socket for multicast");
/*
	for (uint32_t i = 0 ; i < m_ipv6->GetNInterfaces (); i++)
	{
		for(uint32_t j = 0; j < m_ipv6->GetNAddresses(i); j++)
		{
			Ipv6InterfaceAddress address = m_ipv6->GetAddress(i,j);
			if(address.GetScope() != Ipv6InterfaceAddress::LINKLOCAL && address.GetScope() != Ipv6InterfaceAddress::HOST)
			{
				NS_LOG_LOGIC ("MCAST: adding socket to " << address.GetAddress () << " INIT - ACTIVE");
				TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
				Ptr<Node> theNode = GetObject<Node> ();
				Ptr<Socket> socket = Socket::CreateSocket (theNode, tid);
				Inet6SocketAddress global = Inet6SocketAddress (address.GetAddress (), MCAST_PORT);
				socket->SetRecvCallback (MakeCallback (&RoutingProtocol::Recieve,this));
				int ret = socket->Bind (global);
				NS_ASSERT_MSG (ret == 0, "Bind unsuccessful");
//        socket->BindToNetDevice (l3->GetNetDevice (interface)); Not sure if want this
				socket->SetAllowBroadcast(true);
				m_socketAddresses.insert (std::make_pair (socket, address));
			}
		}
	}

  //Create socket for hello msgs
  NS_LOG_LOGIC ("MCAST: adding hello socket to " << MCAST_ALL_NODE << " INIT - HELLO");
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Node> theNode = GetObject<Node> ();
  m_recvSocket = Socket::CreateSocket (theNode, tid);
  Inet6SocketAddress local = Inet6SocketAddress (MCAST_ALL_NODE, MCAST_PORT);
  m_recvSocket->Bind (local);
  m_recvSocket->SetRecvCallback (MakeCallback (&RoutingProtocol::Recieve, this));
  //m_recvSocket->SetIpv6RecvHopLimit (true);
  m_recvSocket->SetRecvPktInfo (true);
*/
  //Create sockets for sending data


	uint32_t startTime;
	if (EnableHello)
	{
		m_htimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
		startTime = m_uniformRandomVariable->GetInteger (0, 100);
		NS_LOG_DEBUG ("Starting at time " << startTime << "ms");
		m_htimer.Schedule (MilliSeconds (startTime));
	}

  std::cout << "Initializing mcast for" << this << std::endl;

	Ipv6RoutingProtocol::DoInitialize ();

}

} //mcast
} //ns3

