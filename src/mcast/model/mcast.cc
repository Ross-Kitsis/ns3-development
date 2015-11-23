/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "mcast.h"
//#include "mcast-neighbor.h"

#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h" //Needed for hellos?
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/ipv6-route.h"

#include "ns3/mobility-model.h"

#include <algorithm>
#include <limits>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("MobicastProtocol");
namespace mcast
{
NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);

const uint32_t RoutingProtocol::MCAST_PORT = 701;

RoutingProtocol::RoutingProtocol () :
	  		HelloInterval (3), //Hello broadcast interval
	  		EnableHello (false), //Enable hello msgs
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
    ;

    return tid;

}

void
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION (this);
  if (EnableHello)
    {
      m_nb.ScheduleTimer ();
    }
  //m_rreqRateLimitTimer.SetFunction (&RoutingProtocol::RreqRateLimitTimerExpire, this);
  //m_rreqRateLimitTimer.Schedule (Seconds (1));

  //m_rerrRateLimitTimer.SetFunction (&RoutingProtocol::RerrRateLimitTimerExpire, this);
  //m_rerrRateLimitTimer.Schedule (Seconds (1));

}

Ptr<Ipv6Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  Ptr<Ipv6Route> route;
  return route;
}

bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb)
{
	bool toReturn = false;
	return toReturn;
}

void
RoutingProtocol::NotifyInterfaceUp (uint32_t interface)
{
	NS_LOG_FUNCTION (this << m_ipv6->GetAddress (interface, 0));
	Ptr<Ipv6L3Protocol> l3 = m_ipv6->GetObject<Ipv6L3Protocol> ();



	Ipv6InterfaceAddress iface = l3->GetAddress (interface, 0);
	if (iface.GetAddress() == Ipv6Address ("::1"))
		return;

	// Create a socket to listen only on this interface
	Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
			UdpSocketFactory::GetTypeId ());
	NS_ASSERT (socket != 0);
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvMcast, this));
	socket->Bind (Inet6SocketAddress (Ipv6Address::GetAny (), MCAST_PORT));
	socket->BindToNetDevice (l3->GetNetDevice (interface));
	socket->SetAllowBroadcast (true);
	socket->SetAttribute ("IpTtl", UintegerValue (1));
	m_socketAddresses.insert (std::make_pair (socket, iface));

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


	Ptr<NetDevice> dev = m_ipv6->GetNetDevice (m_ipv6->GetInterfaceForAddress (iface.GetAddress ()));

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

	// Allow neighbor manager use this interface for layer 2 feedback if possible
	Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
	if (wifi == 0)
		return;
	Ptr<WifiMac> mac = wifi->GetMac ();
	if (mac == 0)
		return;

	mac->TraceConnectWithoutContext ("TxErrHeader", m_nb.GetTxErrorCallback ());
}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t interface)
{
}
void
RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
}

//TO DO ASAP
void
RoutingProtocol::RecvMcast(Ptr<Socket> socket)
{

}

/**
 * Seems to generally be unused in implementations but is virtual defined in IPv6 route
 */
void
RoutingProtocol::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
  NS_LOG_INFO (this << dst << mask << nextHop << interface << prefixToUse);
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
  NS_ASSERT (ipv6 != 0);
  NS_ASSERT (m_ipv6 == 0);

  m_ipv6 = ipv6;

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv6->GetNInterfaces () == 1 && m_ipv6->GetAddress (0, 0).GetAddress() == Ipv6Address ("::1"));
  m_lo = m_ipv6->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);


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
void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
}



void
RoutingProtocol::DoDispose ()
{
}


void
RoutingProtocol::HelloTimerExpire()
{
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
}

void
RoutingProtocol::SendHello ()
{
  NS_LOG_FUNCTION (this);
  for (std::map<Ptr<Socket>, Ipv6InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
  {
  	Ptr<Socket> socket = j->first;
  	Ipv6InterfaceAddress iface = j->second;

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
  	HelloHeader helloHeader(/*Type*/ type, /*roadId*/ roadId, /*roadId*/ hopCount, /*neighbor life time */m_NeighborLifetime, /*radius*/ m_radius,reserved, /**/Ipv6Address().GetOnes(), /**/iface.GetAddress(), /*Position*/ pos, /*Velocity*/ vel );

  	Ptr<Packet> packet = Create<Packet>();
  	packet->AddHeader(helloHeader);
  	TypeHeader theader (HELLO);
  	packet->AddHeader(theader);

  	//Set Jitter time before sending
  	Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
  	Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this , socket, packet, Ipv6Address().GetOnes());

  }
}

void
RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv6Address destination)
{
    socket->SendTo (packet, 0, Inet6SocketAddress (destination, MCAST_PORT));

}

Vector
RoutingProtocol::GetNodePosition (Ptr<Ipv6> ipv6)
{
    Vector pos = ipv6->GetObject<MobilityModel>()->GetPosition();
    NS_LOG_DEBUG (" Node " << ipv6->GetObject<Node>()->GetId() << " position =" << pos);
        return pos;
}

Vector
RoutingProtocol::GetNodeVelocity (Ptr<Ipv6> ipv6)
{
    Vector vel = ipv6->GetObject<MobilityModel>()->GetVelocity();
    NS_LOG_DEBUG (" Node " << ipv6->GetObject<Node>()->GetId() << " velocity =" << vel);
        return vel;
}

void
RoutingProtocol::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  uint32_t startTime;
  if (EnableHello)
    {
      m_htimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
      startTime = m_uniformRandomVariable->GetInteger (0, 100);
      NS_LOG_DEBUG ("Starting at time " << startTime << "ms");
      m_htimer.Schedule (MilliSeconds (startTime));
    }
  Ipv6RoutingProtocol::DoInitialize ();

}

} //mcast
} //ns3

