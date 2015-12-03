/*
 * mcast2.cc
 *
 *  Created on: Dec 2, 2015
 *      Author: ross
 */

#include "mcast2.h"
#include <iomanip>
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/unused.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv6-route.h"
#include "ns3/node.h"
#include "ns3/names.h"

#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/ipv6-packet-info-tag.h"

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





#define MCAST_ALL_NODE "ff02::114"
#define MCAST_PORT 555

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThesisRoutingProtocol");

namespace mcast
{
NS_OBJECT_ENSURE_REGISTERED(ThesisRoutingProtocol);

ThesisRoutingProtocol::ThesisRoutingProtocol()
	: m_ipv6(0), m_initialized(false), m_helloInterval(3)
{
	m_rng = CreateObject<UniformRandomVariable>();
}

ThesisRoutingProtocol::~ThesisRoutingProtocol()
{
	// TODO Auto-generated destructor stub
}


TypeId
ThesisRoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::mcast::ThesisRoutingProtocol")
    .SetParent<Ipv6RoutingProtocol> ()
    .SetGroupName ("mcast")
    .AddConstructor<ThesisRoutingProtocol> ()
    /*
    .AddAttribute ("UnsolicitedRoutingUpdate", "The time between two Unsolicited Routing Updates.",
                   TimeValue (Seconds(30)),
                   MakeTimeAccessor (&RipNg::m_unsolicitedUpdate),
                   MakeTimeChecker ())
    .AddAttribute ("StartupDelay", "Maximum random delay for protocol startup (send route requests).",
                   TimeValue (Seconds(1)),
                   MakeTimeAccessor (&RipNg::m_startupDelay),
                   MakeTimeChecker ())
    .AddAttribute ("TimeoutDelay", "The delay to invalidate a route.",
                   TimeValue (Seconds(180)),
                   MakeTimeAccessor (&RipNg::m_timeoutDelay),
                   MakeTimeChecker ())
    .AddAttribute ("GarbageCollectionDelay", "The delay to delete an expired route.",
                   TimeValue (Seconds(120)),
                   MakeTimeAccessor (&RipNg::m_garbageCollectionDelay),
                   MakeTimeChecker ())
    .AddAttribute ("MinTriggeredCooldown", "Min cooldown delay after a Triggered Update.",
                   TimeValue (Seconds(1)),
                   MakeTimeAccessor (&RipNg::m_minTriggeredUpdateDelay),
                   MakeTimeChecker ())
    .AddAttribute ("MaxTriggeredCooldown", "Max cooldown delay after a Triggered Update.",
                   TimeValue (Seconds(5)),
                   MakeTimeAccessor (&RipNg::m_maxTriggeredUpdateDelay),
                   MakeTimeChecker ())
    .AddAttribute ("SplitHorizon", "Split Horizon strategy.",
                   EnumValue (RipNg::POISON_REVERSE),
                   MakeEnumAccessor (&RipNg::m_splitHorizonStrategy),
                   MakeEnumChecker (RipNg::NO_SPLIT_HORIZON, "NoSplitHorizon",
                                    RipNg::SPLIT_HORIZON, "SplitHorizon",
                                    RipNg::POISON_REVERSE, "PoisonReverse"))
  */
  ;
  return tid;
}

Ptr<Ipv6Route>
ThesisRoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif,
			Socket::SocketErrno &sockerr)
{
	Ptr<Ipv6Route> r = 0;
	return r;
}

bool
ThesisRoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
			UnicastForwardCallback ucb, MulticastForwardCallback mcb,
			LocalDeliverCallback lcb, ErrorCallback ecb)
{
	bool toReturn = true;
	return toReturn;

}

void
ThesisRoutingProtocol::NotifyInterfaceUp (uint32_t interface)
{

	NS_LOG_FUNCTION(this << interface << "  RUNNING");
	for(uint32_t j=0; j < m_ipv6->GetNAddresses(interface); j++)
	{
		Ipv6InterfaceAddress address = m_ipv6->GetAddress(interface,j);
		Ipv6Prefix networkMask = address.GetPrefix ();
		Ipv6Address networkAddress = address.GetAddress ().CombinePrefix (networkMask);
		if (address != Ipv6Address () && networkMask != Ipv6Prefix ())
		{
			AddNetworkRouteTo (networkAddress, networkMask, interface);
		}
	}

	if (!m_initialized)
	{
		return;
	}

  bool sendSocketFound = false;
  for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
    {
      if (iter->second == interface)
        {
          sendSocketFound = true;
          break;
        }
    }

  	for(uint32_t j = 0; j < m_ipv6->GetNAddresses(interface); j++)
  	{
      Ipv6InterfaceAddress address = m_ipv6->GetAddress (interface, j);
      NS_LOG_LOGIC("		Adding socket to: " << address.GetAddress() <<
      		" On interface " << interface);
  	}
  	if(sendSocketFound)
  	{}


}

void
ThesisRoutingProtocol::NotifyInterfaceDown (uint32_t interface)
{

}

void
ThesisRoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{

}

void
ThesisRoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{

}

void
ThesisRoutingProtocol::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse)
{

}
void
ThesisRoutingProtocol::NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse)
{

}
void
ThesisRoutingProtocol::SetIpv6 (Ptr<Ipv6> ipv6)
{

	NS_LOG_FUNCTION (this << ipv6);
  NS_ASSERT (m_ipv6 == 0 && ipv6 != 0);
  uint32_t i = 0;
  m_ipv6 = ipv6;


  for (i = 0; i < m_ipv6->GetNInterfaces (); i++)
    {
      if (m_ipv6->IsUp (i))
        {
      		NS_LOG_LOGIC("IPv6 interface up, notify interface up");
          NotifyInterfaceUp (i);
        }
      else
        {
      		NS_LOG_LOGIC("IPv6 interface down, notify interface down");
          NotifyInterfaceDown (i);
        }
    }
}

void
ThesisRoutingProtocol::HelloTimerExpire()
{
	NS_LOG_FUNCTION (this);


	DoSendHello ();

	Time delay = m_helloInterval + Seconds (m_rng->GetValue (0, 0.5*m_helloInterval.GetSeconds ()) );

	//Cancel previous timer to reset
	m_helloTimer.Cancel();

	//Set new timer
	m_helloTimer.Schedule(delay);

	//m_nextUnsolicitedUpdate = Simulator::Schedule (delay, &RipNg::SendUnsolicitedRouteUpdate, this);

}

void
ThesisRoutingProtocol::DoSendHello()
{
	NS_LOG_FUNCTION (this);

  for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
  {
    uint32_t interface = iter->second;
    //Assume no interface exclusions

    //Skip MTU and RTE stuff
    Ptr<Packet> p = Create<Packet> ();


  }

}

int64_t
ThesisRoutingProtocol::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);

  m_rng->SetStream (stream);
  return 1;
}

void
ThesisRoutingProtocol::AddNetworkRouteTo (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
{
  NS_LOG_FUNCTION (this << network << networkPrefix << interface);

  ThesisRoutingTableEntry* route = new ThesisRoutingTableEntry (network, networkPrefix, interface);
  route->SetRouteMetric (1);
  route->SetRouteStatus (ThesisRoutingTableEntry::ROUTE_VALID);
  route->SetRouteChanged (true);

  m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisRoutingProtocol::Receive (Ptr<Socket> socket)
{

}

void
ThesisRoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{

}

/*
 * TO DO
 */
void
ThesisRoutingTableEntry::SetRouteChanged(bool changed)
{

}

/*
 * TO DO
 */
void
ThesisRoutingTableEntry::SetRouteStatus (Status_e status)
{
  if (m_status != status)
    {
      m_status = status;
      m_changed = true;
    }
}

/*
 * TO DO
 */
void ThesisRoutingTableEntry::SetRouteMetric (uint8_t routeMetric)
{
  if (m_metric != routeMetric)
    {
      m_metric = routeMetric;
      m_changed = true;
    }
}

/*
 * RipNgRoutingTableEntry
 */

ThesisRoutingTableEntry::ThesisRoutingTableEntry ()
  : m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisRoutingTableEntry::ThesisRoutingTableEntry (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
  : Ipv6RoutingTableEntry ( ThesisRoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, nextHop, interface, prefixToUse) ),
    m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisRoutingTableEntry::ThesisRoutingTableEntry (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
  : Ipv6RoutingTableEntry ( Ipv6RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, interface) ),
    m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisRoutingTableEntry::~ThesisRoutingTableEntry ()
{
}




void
ThesisRoutingProtocol::DoDispose ()
{

}

void
ThesisRoutingProtocol::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  //bool addedGlobal = false;

  m_initialized = true;

  for (uint32_t i = 0 ; i < m_ipv6->GetNInterfaces (); i++)
  {
  	/* Assume no interface exclusions*/
  	for (uint32_t j = 0; j < m_ipv6->GetNAddresses (i); j++)
  	{
  		Ipv6InterfaceAddress address = m_ipv6->GetAddress (i, j);
  		if (address.GetScope() == Ipv6InterfaceAddress::GLOBAL)
  		{
  			NS_LOG_LOGIC ("	MCAST: adding socket to " << address.GetAddress ());
  			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  			Ptr<Node> theNode = GetObject<Node> ();
  			Ptr<Socket> socket = Socket::CreateSocket (theNode, tid);
  			Inet6SocketAddress local = Inet6SocketAddress (address.GetAddress (), MCAST_PORT);
  			int ret = socket->Bind (local);
  			NS_ASSERT_MSG (ret == 0, "Bind unsuccessful to " << address);
  			socket->BindToNetDevice (m_ipv6->GetNetDevice (i));
  			socket->ShutdownRecv ();
  			socket->SetIpv6RecvHopLimit (true);
  			m_sendSocketList[socket] = i;


  			//Add multicast route
  		  AddNetworkRouteTo (Ipv6Address(MCAST_ALL_NODE), Ipv6Prefix("ff"), i);

  		}

  	}

  }

  //Set Recieve Socket (HELLO)
  if (!m_recvSocket)
  {
  	NS_LOG_LOGIC ("MCAST multicast interface: adding receiving socket");
  	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  	Ptr<Node> theNode = GetObject<Node> ();
  	m_recvSocket = Socket::CreateSocket (theNode, tid);
  	Inet6SocketAddress local = Inet6SocketAddress (MCAST_ALL_NODE, MCAST_PORT);
  	if(Ipv6Address(MCAST_ALL_NODE).IsMulticast())
  	{
	  	Ptr<UdpSocket> Udp = DynamicCast<UdpSocket>(m_recvSocket);
	  	if(Udp)
	  	{
	  		Udp -> MulticastJoinGroup(0,Ipv6Address(MCAST_ALL_NODE));
	  	}else
	  	{
	  		NS_FATAL_ERROR ("Error: Failed to join multicast group");
	  	}
  	}
  	m_recvSocket->Bind (local);
  	m_recvSocket->SetRecvCallback (MakeCallback (&ThesisRoutingProtocol::Receive, this));
  	m_recvSocket->SetIpv6RecvHopLimit (true);
  	m_recvSocket->SetRecvPktInfo (true);
  }


  Time delay = m_helloInterval + Seconds (m_rng->GetValue (0, 0.5*m_helloInterval.GetSeconds ()) );
  //m_nextUnsolicitedUpdate = Simulator::Schedule (delay, &RipNg::SendUnsolicitedRouteUpdate, this);
  m_helloTimer.SetFunction(&ThesisRoutingProtocol::HelloTimerExpire, this);
  m_helloTimer.Schedule(delay);

	Ipv6RoutingProtocol::DoInitialize ();
}

}
}
