/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "mcast.h"
//#include "mcast-neighbor.h"

#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h" //Needed for hellos?
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/ipv6-route.h"
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
	  		HelloInterval (Seconds (1)), //Hello broadcast interval
	  		EnableHello (false), //Enable hello msgs
	  		EnableBroadcast(true), //Enable broadcasts
	  		m_lastHelloBcastTime (Seconds (0)), //Initialize last broadcast time to 0
	  		m_nb (HelloInterval) //Initialize nieghbor broadcasting every hello interval seconds

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
  Time diff = HelloInterval - offset;
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
  }


//  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
//    {
//      Ptr<Socket> socket = j->first;
//      Ipv4InterfaceAddress iface = j->second;
//      RrepHeader helloHeader (/*prefix size=*/ 0, /*hops=*/ 0, /*dst=*/ iface.GetLocal (), /*dst seqno=*/ m_seqNo,
//                                               /*origin=*/ iface.GetLocal (),/*lifetime=*/ Time (AllowedHelloLoss * HelloInterval));
//      Ptr<Packet> packet = Create<Packet> ();
//      packet->AddHeader (helloHeader);
//      TypeHeader tHeader (AODVTYPE_RREP);
//      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
//      Ipv4Address destination;
//      if (iface.GetMask () == Ipv4Mask::GetOnes ())
//        {
//          destination = Ipv4Address ("255.255.255.255");
//        }
//      else
//        {
//          destination = iface.GetBroadcast ();
//        }
//      Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
//      Simulator::Schedule (jitter, &RoutingProtocol::SendTo, this , socket, packet, destination);
//    }
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

