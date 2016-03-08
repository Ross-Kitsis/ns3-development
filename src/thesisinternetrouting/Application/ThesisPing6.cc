/*
 * ThesisPing6.cc
 *
 *  Created on: Feb 22, 2016
 *      Author: Ross Kitsis
 */

#include "ThesisPing6.h"

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
#include "ns3/icmpv6-header.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-extension-header.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("ThesisPing6");

NS_OBJECT_ENSURE_REGISTERED (ThesisPing6);


ThesisPing6::ThesisPing6()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sent = 0;
  m_socket = 0;
  m_seq = 0;
  m_sendEvent = EventId ();
}

ThesisPing6::~ThesisPing6()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

TypeId
ThesisPing6::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ThesisPing6")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<ThesisPing6>()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&ThesisPing6::m_count),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&ThesisPing6::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteIpv6",
                   "The Ipv6Address of the outbound packets",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&ThesisPing6::m_peerAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("LocalIpv6",
                   "Local Ipv6Address of the sender",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&ThesisPing6::m_localAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated",
                   UintegerValue (100),
                   MakeUintegerAccessor (&ThesisPing6::m_size),
                   MakeUintegerChecker<uint32_t>())
  ;
  return tid;
}

void
ThesisPing6::DoDispose()
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
ThesisPing6::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (!m_socket)
    {
      //TypeId tid = TypeId::LookupByName ("ns3::Ipv6RawSocketFactory");

  		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  		m_socket = Socket::CreateSocket (GetNode (), tid);

      NS_ASSERT (m_socket);

      m_socket->Bind (Inet6SocketAddress (m_localAddress, 0));
      //m_socket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));
      m_socket->SetRecvCallback (MakeCallback (&ThesisPing6::HandleRead, this));

    }

  ScheduleTransmit (Seconds (0.));
}

void
ThesisPing6::SetLocal (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_localAddress = ipv6;
}

void
ThesisPing6::SetRemote (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_peerAddress = ipv6;
}

void
ThesisPing6::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> >());
    }

  Simulator::Cancel (m_sendEvent);
}

void
ThesisPing6::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void
ThesisPing6::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &ThesisPing6::Send, this);
}

void
ThesisPing6::SetRouters (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}

void
ThesisPing6::Send ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> p = 0;
  uint8_t* data = new uint8_t[m_size];
  Ipv6Address src;
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();

  /*
  if (m_ifIndex > 0)
    {
      for (uint32_t i = 0; i < GetNode ()->GetObject<Ipv6> ()->GetNAddresses (m_ifIndex); i++)
        {
          Ipv6InterfaceAddress srcIa;
          srcIa = GetNode ()->GetObject<Ipv6> ()->GetAddress (m_ifIndex, i);

          if (srcIa.IsInSameSubnet (m_peerAddress))
            {
              src = srcIa.GetAddress ();
              break;
            }
        }
     }
  else
    {
      src = m_localAddress;
    }
	*/

  /*
  if(m_ifIndex > 0)
  {
  	Ptr<Ipv6> ipv6 = GetNode() ->GetObject<Ipv6>();
  	for(uint32_t i = 0; i < ipv6 -> GetNAddresses(m_ifIndex); i++)
  	{
  		Ipv6Address srcAdd = ipv6 -> GetAddress(m_ifIndex,i).GetAddress();
  		if(!srcAdd.IsLinkLocal())
  		{
  			src = srcAdd;
  		}
  	}
  }*/


  for(uint32_t i = 0; i < ipv6 ->GetNInterfaces();i++)
  {
  	for(uint32_t j = 0; j < ipv6 ->GetNAddresses(i); j++)
  	{
  		Ipv6Address srcAdd = ipv6 ->GetAddress(i,j).GetAddress();
  		if(!srcAdd.IsLinkLocal())
  		{
  			src = srcAdd;
  			break;
  		}
  	}
  }

  std::cout << "PING >>>>>>>>>>>>>>>>> Source address used to send: " << src << std::endl;
  std::cout << "PING >>>>>>>>>>>>>>>>> Destination address sending to: " << m_peerAddress << std::endl;

  NS_ASSERT_MSG (m_size >= 4, "ICMPv6 echo request payload size must be >= 4");
  data[0] = 0xDE;
  data[1] = 0xAD;
  data[2] = 0xBE;
  data[3] = 0xEF;

  p = Create<Packet> (data, 4);
  p->AddAtEnd (Create<Packet> (m_size - 4));

  /*

  Icmpv6Echo req (1);

  req.SetId (0xBEEF);
  req.SetSeq (m_seq);
  m_seq++;

// we do not calculate pseudo header checksum here, because we are not sure about
// source IPv6 address. Checksum is calculated in Ipv6RawSocketImpl.

  p->AddHeader (req);
  */

  //m_socket->BindToNetDevice()

  int bindResult = m_socket->Bind (Inet6SocketAddress (src, 0));

  std::cout << " >>>>>>>>>>>>>>>>>>> BIND RESULT : " << bindResult << std::endl;

  /* use Loose Routing (routing type 0)
  if (m_routers.size ())
    {
      Ipv6ExtensionLooseRoutingHeader routingHeader;
      routingHeader.SetNextHeader (Ipv6Header::IPV6_ICMPV6);
      routingHeader.SetTypeRouting (0);
      routingHeader.SetSegmentsLeft (m_routers.size ());
      routingHeader.SetRoutersAddress (m_routers);
      p->AddHeader (routingHeader);
      m_socket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_EXT_ROUTING));
    }
    */

  m_socket->SendTo (p, 0, Inet6SocketAddress (m_peerAddress, 0));
  ++m_sent;

  NS_LOG_INFO ("Sent " << p->GetSize () << " bytes to " << m_peerAddress);

  if (m_sent < m_count)
    {
      ScheduleTransmit (m_interval);
    }

  delete[] data;
}

void
ThesisPing6::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  /*

  Ptr<Packet> packet=0;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (Inet6SocketAddress::IsMatchingType (from))
        {
          Ipv6Header hdr;
          Icmpv6Echo reply (0);
          Icmpv6DestinationUnreachable destUnreach;
          Icmpv6TimeExceeded timeExceeded;
          Inet6SocketAddress address = Inet6SocketAddress::ConvertFrom (from);

          packet->RemoveHeader (hdr);

          uint8_t type;
          packet->CopyData (&type, sizeof(type));

          switch (type)
            {
            case Icmpv6Header::ICMPV6_ECHO_REPLY:
              packet->RemoveHeader (reply);

              NS_LOG_INFO ("Received Echo Reply size  = " << std::dec << packet->GetSize () <<
                           " bytes from " << address.GetIpv6 () <<
                           " id =  " << (uint16_t)reply.GetId () <<
                           " seq = " << (uint16_t)reply.GetSeq () <<
                           " Hop Count = " << (uint16_t) (64 - hdr.GetHopLimit ()));
              break;
            case Icmpv6Header::ICMPV6_ERROR_DESTINATION_UNREACHABLE:
              packet->RemoveHeader (destUnreach);

              NS_LOG_INFO ("Received Destination Unreachable from " << address.GetIpv6 ());
              break;
            case Icmpv6Header::ICMPV6_ERROR_TIME_EXCEEDED:
              packet->RemoveHeader (timeExceeded);

              NS_LOG_INFO ("Received Time Exceeded from " << address.GetIpv6 ());
              break;
            default:
              break;
            }
        }
    }
    */
}


} /* namespace ns3 */
