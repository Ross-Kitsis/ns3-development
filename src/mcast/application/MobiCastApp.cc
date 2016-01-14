/*
 * MoniCastApp.cc
 *
 *  Created on: Jan 8, 2016
 *      Author: ross
 */

#include "MobiCastApp.h"

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"

#define MCAST_PORT 555
#define MCAST_CONTROL_GRP "ff02::115" //Used to send mcast control packets

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("MobiCastApp");

NS_OBJECT_ENSURE_REGISTERED(MobiCastApp);

TypeId
MobiCastApp::GetTypeId()
{
	static TypeId tid = TypeId("ns3::MobiCastApp")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<MobiCastApp>()
        ;
	return tid;
}

MobiCastApp::MobiCastApp()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sent = 0;
  m_socket = 0;
  m_seq = 0;
  m_sendEvent = EventId ();

  // Set IPv6 references
  Ptr<Node> node;

  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
  NS_ASSERT_MSG (ipv6, "Ipv6 not installed on node");
  Ptr<Ipv6RoutingProtocol> proto = ipv6->GetRoutingProtocol ();
  NS_ASSERT_MSG (proto, "Ipv6 routing not installed on node");
  Ptr<mcast::ThesisRoutingProtocol> mcast = DynamicCast<mcast::ThesisRoutingProtocol> (proto);
  NS_ASSERT_MSG (mcast, "ThesisRoutingProtocol not installed on node");


  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
  m_rng = uv;
  SeedManager::SetSeed(time(0));
}

MobiCastApp::~MobiCastApp()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

void
MobiCastApp::DoDispose()
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
MobiCastApp::StartApplication()
{
	NS_LOG_FUNCTION_NOARGS();

	// Check if have a socket to receive mcast packets
	// Socket needs to be for mcast address??
  if (!m_socket)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);

      NS_ASSERT (m_socket);

      m_socket->Bind (Inet6SocketAddress (MCAST_CONTROL_GRP, MCAST_PORT));
//      m_socket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));

    	if(Ipv6Address(MCAST_CONTROL_GRP).IsMulticast())
    	{
  	  	Ptr<UdpSocket> Udp = DynamicCast<UdpSocket>(m_socket);
  	  	if(Udp)
  	  	{
  	  		Udp -> MulticastJoinGroup(0,Ipv6Address(MCAST_CONTROL_GRP));
  	  	}else
  	  	{
  	  		NS_FATAL_ERROR ("Error: Failed to join multicast group");
  	  	}
    	}

      m_socket->SetRecvCallback (MakeCallback (&MobiCastApp::HandleRead, this));
    }

  ScheduleTransmit();

}

void
MobiCastApp::ScheduleTransmit()
{
  NS_LOG_FUNCTION (this);
  double v = m_rng -> GetValue();

  if(v < m_eventProbability)
  {
  	m_transmitEvent = Simulator::Schedule(Time(Seconds(1)) + Time(Seconds(m_interval)),&MobiCastApp::ScheduleTransmit,this);
  	m_sendEvent = Simulator::Schedule(Time(Seconds(0.1)),&MobiCastApp::Send, this);
  }else
  {
  	m_transmitEvent = Simulator::Schedule(Time(Seconds(1)) + Time(Seconds(m_interval)),&MobiCastApp::ScheduleTransmit,this);
  }
}


void
MobiCastApp::Send ()
{
	NS_LOG_FUNCTION_NOARGS ();
	NS_ASSERT (m_sendEvent.IsExpired ());

	///////////////////// Add more to packet to make it more realistic
	// Make packet 1024 bytes??

	Ptr<Packet> p = Create<Packet> ();

	m_routing.DoSendMcastControl(p);
}

void
MobiCastApp::HandleRead (Ptr<Socket> socket)
{
	//TO IMPLEMENT
}

void
MobiCastApp::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket)
  {
  	m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> >());
  }

  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_transmitEvent);
}

} /* namespace ns3 */
