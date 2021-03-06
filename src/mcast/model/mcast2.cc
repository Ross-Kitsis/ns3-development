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





#define MCAST_ALL_NODE "ff02::114" //Used for hello packets
#define MCAST_PORT 555
#define MCAST_CONTROL_GRP "ff02::115" //Used to send mcast control packets

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThesisRoutingProtocol");

namespace mcast
{
NS_OBJECT_ENSURE_REGISTERED(ThesisRoutingProtocol);

ThesisRoutingProtocol::ThesisRoutingProtocol():
		m_ipv6(0),
		m_initialized(false),
		m_helloInterval(3),
		m_neighbors(Seconds(m_helloInterval * 5)),
		m_dpd(MilliSeconds(100)),
		m_sendHello(true),
		m_mcastPacketSize(1024),
		m_initialDelay(Seconds(10))
{
	m_rng = CreateObject<UniformRandomVariable>();
	//m_neighbors = ThesisNeighbors(Seconds(m_helloInterval * 5));
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
    .AddAttribute ("HelloTimer", "The time between two Hello msgs",
                   TimeValue (Seconds(3)),
                   MakeTimeAccessor (&ThesisRoutingProtocol::m_helloInterval),
                   MakeTimeChecker ())
		.AddAttribute ("HelloBroadcast",
									 "Switch to enable/disable hello msgs",
										BooleanValue (true),
										MakeBooleanAccessor (&ThesisRoutingProtocol::m_sendHello),
										MakeBooleanChecker ())
		.AddAttribute ("Initial Delay",
									 "Initial delay before sending any hello messages",
									  TimeValue (Seconds(10)),
										MakeTimeAccessor (&ThesisRoutingProtocol::m_initialDelay),
										MakeTimeChecker ())



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

  NS_LOG_FUNCTION (this << header << oif);
  Ipv6Address destination = header.GetDestinationAddress ();
  Ptr<Ipv6Route> rtentry = 0;

  if(destination.IsMulticast())
  {
  	NS_LOG_LOGIC("	RoutingOutput: Multicast Destination " << destination);
  }

  //////////////////////////////////////////////////////

	TypeHeader tHeader(UNKNOWN);
	p->PeekHeader(tHeader);

	NS_LOG_LOGIC("Route output type header: " << tHeader);

  //////////////////////////////////////////////////////

  rtentry = Lookup(destination, oif);

  NS_LOG_LOGIC("	ROUTE FOUND " << rtentry);

  //return 0;
	return rtentry;
}

bool
ThesisRoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
			UnicastForwardCallback ucb, MulticastForwardCallback mcb,
			LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << header << header.GetSourceAddress () << header.GetDestinationAddress () << idev);
	bool toReturn = true;


  NS_ASSERT (m_ipv6 != 0);
  // Check if input device supports IP
  NS_ASSERT (m_ipv6->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_ipv6->GetInterfaceForDevice (idev);
  Ipv6Address dst = header.GetDestinationAddress ();
  Ipv6Address src = header.GetSourceAddress();

	if(IsMyOwnAddress(src))
	{
		NS_LOG_LOGIC("Packet from own address" << src);
		return false;
	}

	//std::cout << "mcast route input" << std::endl;



	//Peek header but do not actually remove
	TypeHeader tHeader(UNKNOWN);
	p->PeekHeader(tHeader);

	//p->Print(std::cout);

	//std::cout<< std::endl;

	//std::cout << "Type: "  << (MessageType) tHeader.Get() << std::endl;
	NS_LOG_LOGIC("Message type: " << (MessageType)tHeader.Get());

	if(tHeader.Get() == HELLO || tHeader.Get() == MCAST_CONTROL ||
			dst.IsEqual(Ipv6Address(MCAST_ALL_NODE)) ||
			dst.IsEqual(Ipv6Address(MCAST_CONTROL_GRP)))
	{
		NS_LOG_LOGIC("Received packet with HELLO/CONTROL");
		//Type of packet is hello or control
		//std::cout << "Received MCast hello or control packet" << std::endl;
		lcb(p,header,iif);
		toReturn = true;
	}else
	{
		//Type of packet is something else (Either internet, query or RSU something)
		toReturn = false;
	}
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
	NS_LOG_FUNCTION(this);
}

void
ThesisRoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisRoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisRoutingProtocol::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisRoutingProtocol::NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
			uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}
void
ThesisRoutingProtocol::SetIpv6 (Ptr<Ipv6> ipv6)
{

	//std::cout << "Setting IPv6 in routing protocol \n";

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

	Time delay = m_helloInterval + m_rng->GetValue(0,0.5)*m_helloInterval;
	//+ m_rng->GetValue (0, 0.5*m_helloInterval.GetSeconds() );

	//Cancel previous timer to reset
	m_helloTimer.Cancel();

	NS_LOG_LOGIC("Scheduling next hello in: " << delay.GetSeconds());

	//Set new timer
	m_helloTimer.SetFunction(&ThesisRoutingProtocol::HelloTimerExpire, this);
	m_helloTimer.Schedule(delay);

	DoSendHello ();

	//m_nextUnsolicitedUpdate = Simulator::Schedule (delay, &RipNg::SendUnsolicitedRouteUpdate, this);

}

void
ThesisRoutingProtocol::DoSendHello()
{
	NS_LOG_FUNCTION (this);

//	std::cout << "Sending hello packet" << std::endl;

	//Perform address checking here, re-bind to the correct address
	for(uint32_t i = 0; i < m_ipv6 ->GetNInterfaces();i++)
	{
		for(uint32_t j = 0; j < m_ipv6 ->GetNAddresses(i); j++)
		{
			Ipv6Address srcAdd = m_ipv6 ->GetAddress(i,j).GetAddress();
			if(!srcAdd.IsLinkLocal())
			{
				m_globalAddress = srcAdd;
				break;
			}
		}
	}

	//m_recvSocket = Socket::CreateSocket (theNode, tid);
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

  //for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
  //{
  //  uint32_t interface = iter->second;
    //Assume no interface exclusions

    //Skip MTU and RTE stuff
    Ptr<Packet> p = Create<Packet> ();

  	//Get node position
  	Vector pos = GetNodePosition(m_ipv6);

  	//Get node velocityALL_NODE
  	Vector vel = GetNodeVelocity(m_ipv6);

  	//Get Road ID
  	uint64_t roadId = 0;

  	//hopCount
  	uint8_t hopCount = 0;

  	uint8_t type = 1;

  	uint16_t reserved = 0;

  	Ipv6Address origin = m_globalAddress;



  	//Create hello header
  	HelloHeader helloHeader(/*Type*/ type, /*roadId*/ roadId, /*roadId*/ hopCount, /*neighbor life time */ 0, /*radius*/ 0 ,reserved, /**/Ipv6Address(MCAST_ALL_NODE), /**/origin, /*Position*/ pos, /*Velocity*/ vel );

  	Ipv6Address destination = Ipv6Address(MCAST_ALL_NODE);

  	Ptr<Packet> packet = Create<Packet>();
  	packet->AddHeader(helloHeader);

  	TypeHeader theader (HELLO);
  	packet->AddHeader(theader);


  	NS_LOG_LOGIC("Sending packet to: " << destination);
  	//std::cout << "Sending -- " << std::endl;
  	//iter->first->SendTo(packet, 0, Inet6SocketAddress(MCAST_ALL_NODE, MCAST_PORT));
  	m_recvSocket -> SendTo(packet,0,Inet6SocketAddress(MCAST_ALL_NODE, MCAST_PORT));
 // }

}

void
ThesisRoutingProtocol::DoSendMcastControl()
{
	//std::cout << "Running doSendMCastControl" << std::endl;
	NS_LOG_FUNCTION(this);

	for(uint32_t i = 0; i < m_ipv6 ->GetNInterfaces();i++)
	{
		for(uint32_t j = 0; j < m_ipv6 ->GetNAddresses(i); j++)
		{
			Ipv6Address srcAdd = m_ipv6 ->GetAddress(i,j).GetAddress();
			if(!srcAdd.IsLinkLocal())
			{
				m_globalAddress = srcAdd;
				break;
			}
		}
	}

	//m_recvSocket = Socket::CreateSocket (theNode, tid);
	Inet6SocketAddress local = Inet6SocketAddress (MCAST_CONTROL_GRP, MCAST_PORT);
	if(Ipv6Address(MCAST_CONTROL_GRP).IsMulticast())
	{
		Ptr<UdpSocket> Udp = DynamicCast<UdpSocket>(m_mctrlSocket);
		if(Udp)
		{
			Udp -> MulticastJoinGroup(0,Ipv6Address(MCAST_CONTROL_GRP));
		}else
		{
			NS_FATAL_ERROR ("Error: Failed to join multicast group");
		}
	}
	m_mctrlSocket->Bind (local);
	m_mctrlSocket->SetRecvCallback (MakeCallback (&ThesisRoutingProtocol::Receive, this));





	Ipv6Address Id = m_globalAddress;
	Ipv6Address source = m_globalAddress;

	Vector position = m_ipv6->GetObject<MobilityModel>()->GetPosition();
	Vector velocity = m_ipv6->GetObject<MobilityModel>()->GetVelocity();

  //for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
  //{
  	//uint32_t interface = iter->second;

  	//Get A value
  	double a = m_mutils.getA(velocity, position);

  	//Get B value
  	double b = m_mutils.GetB();

  	//Get left apex
  	Vector left = m_mutils.GetApexL(velocity,position,a);

  	//Get right apex
  	Vector right = m_mutils.GetApexR(velocity,position,a);

  	//Create control header
  	ControlHeader cHeader(Id,source,a,b,left,right);

  	Ipv6Address destination = Ipv6Address(MCAST_CONTROL_GRP);

  	//Create packet and add control header
  	Ptr<Packet> packet = Create<Packet>();
  	packet->AddHeader(cHeader);

  	//Create type header and add to packet
  	TypeHeader theader (MCAST_CONTROL);
  	packet->AddHeader(theader);

  	//Send packet
  	NS_LOG_LOGIC("Sending packet to: " << destination);
  	//iter->first->SendTo(packet, 0, Inet6SocketAddress(destination, MCAST_PORT));
  	m_mctrlSocket -> SendTo(packet,0,Inet6SocketAddress(destination, MCAST_PORT));
  //}
}

Ptr<Ipv6Route>
ThesisRoutingProtocol::Lookup (Ipv6Address dst, Ptr<NetDevice> interface)
{
	NS_LOG_FUNCTION (this << dst << interface);

	Ptr<Ipv6Route> rtentry=0;
	//uint16_t longestMask = 0;

	//Generally assume you would never send on link local multicast

  for (RoutesI it = m_routes.begin (); it != m_routes.end (); it++)
  {

  	ThesisRoutingTableEntry* j = it->first;
  	NS_LOG_LOGIC("  Route to: " << j->GetDest() << " Status " <<  j->GetRouteStatus());
  	if (j->GetRouteStatus () == ThesisRoutingTableEntry::ROUTE_VALID)
  	{
      Ipv6Prefix mask = j->GetDestNetworkPrefix ();
      uint16_t maskLen = mask.GetPrefixLength ();
      Ipv6Address entry = j->GetDestNetwork ();

      if (mask.IsMatch (dst, entry))
      {
      	NS_LOG_LOGIC ("	Found a route via " << j << " mask length " << maskLen);

      	/*If interface given, check route will output on this interface*/
        if (!interface || interface == m_ipv6->GetNetDevice (j->GetInterface ()))
        {
        	Ipv6RoutingTableEntry* route = j;
        	uint32_t interfaceIdx = route->GetInterface ();
        	rtentry = Create<Ipv6Route> ();

        	rtentry->SetDestination (route->GetDest ());
        	rtentry->SetGateway (route->GetGateway ());
        	rtentry->SetOutputDevice (m_ipv6->GetNetDevice (interfaceIdx));

        	NS_LOG_LOGIC ("Route entry created, ready to return");
        }

      }

  	}
  }
//  return 0;
	return rtentry;

}

Vector
ThesisRoutingProtocol::GetNodePosition (Ptr<Ipv6> ipv6)
{
	NS_LOG_FUNCTION (this);
	Vector pos = ipv6->GetObject<MobilityModel>()->GetPosition();
	NS_LOG_DEBUG (" Node " << ipv6->GetObject<Node>()->GetId() << " position =" << pos);
	return pos;
}

Vector
ThesisRoutingProtocol::GetNodeVelocity (Ptr<Ipv6> ipv6)
{
	NS_LOG_FUNCTION (this);
	Vector vel = ipv6->GetObject<MobilityModel>()->GetVelocity();
	NS_LOG_DEBUG (" Node " << ipv6->GetObject<Node>()->GetId() << " velocity =" << vel);
	return vel;
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
	NS_LOG_FUNCTION("	Packet received " << socket << " IP address " << m_ipv6 );

	//std::cout<<"Rec packet" << std::endl;

	Ptr<Packet> packet = socket->Recv();

	TypeHeader tHeader(HELLO);
	packet->RemoveHeader(tHeader);

	NS_LOG_INFO("Received mcast packet" << *packet);


	if (!tHeader.IsValid ())
	{
		//Unknown packet type
		NS_LOG_DEBUG ("MCAST message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
		return; // drop
	}
	switch (tHeader.Get())
	{
	case HELLO:
	{
	//	RecvHello (packet);
		HelloHeader hHeader;
		packet->RemoveHeader(hHeader);

		NS_LOG_DEBUG("		Receieved hello msg, process hello");
		ProcessHello(hHeader);
		break;
	}
	case MCAST_CONTROL:
	{
		NS_LOG_DEBUG("		Receieved mcast control");


		ControlHeader cHeader;
		packet->RemoveHeader(cHeader);

		ProcessMcastControl(cHeader, packet);
		break;
	}
	case INTERNET:
	{
		//Shouldn't be here
		break;
	}
	case UNKNOWN:
		//Shouldn't be here
		break;

	default:
		//Shouldn't be here
		break;

	}


}

void
ThesisRoutingProtocol::ProcessHello(HelloHeader helloHeader)
{
  NS_LOG_FUNCTION (this << "Processing hello header");

  NS_LOG_FUNCTION (" >>>> Origin" << helloHeader.GetOrigin() << " Position" << helloHeader.GetPosition() << " Velocity"  << helloHeader.GetVelocity());

  m_neighbors.Update(helloHeader.GetOrigin(), helloHeader.GetPosition(), helloHeader.GetVelocity());
}

void
ThesisRoutingProtocol::ProcessMcastControl(ControlHeader cHeader, Ptr<Packet> packet)
{

	//Step 1: Is the node in the ZoR advertised in the ControlHeader?
	Vector position = m_ipv6->GetObject<MobilityModel>()->GetPosition();
	Vector eventPos = cHeader.GetCenter();

	//Check if control packet is duplicate
	//Must have been retransmitted by a closer node, no need to continue processing
	if(m_dpd.IsDuplicate(packet,cHeader))
	{
		//////////// ALSO IMORTANT: CHECK IF POSSIBLE RETRANSMIT scheduled

		//Check if have packet in queue to send (Other node must have had a better neighbor)
		for(std::list<McastRetransmit>::iterator it = m_mr.begin(); it != m_mr.end(); ++it)
		{
			ControlHeader temp;
			//it -> p ->PeekHeader(temp);

			it ->GetPacket()->PeekHeader(temp);

			Ipv6Address Id = temp.GetId();
			Vector ApxL = temp.getApxL();
			Vector ApxR = temp.getApxR();

			if(Id.IsEqual(cHeader.GetId()) && ApxL.x == cHeader.getApxL().x && ApxL.y == cHeader.getApxL().y
					&& ApxR.x == cHeader.getApxR().x && ApxR.y == cHeader.getApxR().y)
			{
				it->timerToSend.Cancel();
				m_mr.erase(it);
				break;
			}

		}
		return;
	}

	if(m_mutils.IsInZor(position,eventPos,cHeader.GetA(),cHeader.GetB()))
	{
		//Node is in the Zor
		//std::cout << "Node is in ZoR" << std::endl;
		if(m_neighbors.IsNeighbor(cHeader.GetSource()))
		{
			//std::cout << "Mcast Sender is a neighbor" << std::endl;
			//Source of packet is a neighbor, have information on the last sender

			//Use center coordinates to find apex current node is closest to
			Vector ClosestApex;

			double dl = m_mutils.GetDistanceBetweenPoints(position.x,position.y,cHeader.getApxL().x,cHeader.getApxL().y);
			double dr = m_mutils.GetDistanceBetweenPoints(position.x,position.y,cHeader.getApxR().x,cHeader.getApxR().y);
			double NodeDistanceToApex;


			if(dl <= dr)
			{
				//Closest apex is left
				ClosestApex = cHeader.getApxL();
				NodeDistanceToApex = dl;

			}else
			{
				//Closest apex is right
				ClosestApex = cHeader.getApxR();
				NodeDistanceToApex = dr;
			}

			//Check if packet is initial transmit or if a retransmission
			if(cHeader.GetId().IsEqual(cHeader.GetSource()))
			{
				//std::cout << "Initial transmit receieved" << std::endl;
				//Event vehicle sent this packet, use center in cHeader
				double SenderDistanceToApex = m_mutils.GetDistanceBetweenPoints(ClosestApex.x, ClosestApex.y,
						cHeader.GetCenter().x, cHeader.GetCenter().y);
				//This if statement should always evaluate true (Sending area focused around initial node)
				if(NodeDistanceToApex < SenderDistanceToApex)
				{
					//Send packet and check effectivity
				/**
				 * Checking efficancy of transmission:
				 *
				 * (Need to have at least one neighbor)
				 * At least one of the neighbors in the nodes neighbor
				 * table must be close to the apex than the current node
				 *
				 */

					if(m_neighbors.GetNeighborTableSize() > 0)
					{
						//Condition 1 met: At least have 1 neighbor
						//Check condition 2; have a neighbor close to apex
						//If condition 2 met then send packet after backoff period

						if(m_neighbors.HaveCloserNeighbor(ClosestApex,NodeDistanceToApex))
						{
							/**
							 * Have a neighbor closer to the apex than current position
							 * Transmission will be effective, send packet after backoff
							*/

							//std::cout << "Efficent retransmission -> resending" << std::endl;

							McastRetransmit * toSend = new McastRetransmit();

					  	//Create packet and add control header
							cHeader.SetSource(m_globalAddress);
					  	packet->AddHeader(cHeader);

					  	//Create type header and add to packet
					  	TypeHeader theader (MCAST_CONTROL);
					  	packet->AddHeader(theader);

					  	//Set packet to send
					  	toSend->SetPacket(packet);

					  	double Vj = m_neighbors.getDistanceClosestNeighborToApex(ClosestApex,NodeDistanceToApex);

					  	double backoff = Vj/NodeDistanceToApex;

					  	int delay = ((int)((10 * backoff) * 10))/10;


					  	toSend->timerToSend.SetFunction(&ThesisRoutingProtocol::DoSendMcastRetransmit, this);
					  	toSend->timerToSend.SetArguments(toSend);
					  	toSend->timerToSend.Schedule(Time(MilliSeconds(delay)));

					  	//Add to retransmit queue
					  	m_mr.push_back(*toSend);
						}

					}

				}

			}else
			{
				//std::cout << "Event vehicle sent packet, processing" << std::endl;
				/*
				 * //Event vehicle sent this packet, use center in cHeader
				double SenderDistanceToApex = m_mutils.GetDistanceBetweenPoints(ClosestApex.x, ClosestApex.y,
						cHeader.GetCenter().x, cHeader.GetCenter().y);
				 */

				//Packet was retransmitted by some other node
				Vector sourcePos = m_neighbors.GetNeighborPosition(cHeader.GetSource());
				double SenderDistanceToApex = m_mutils.GetDistanceBetweenPoints(ClosestApex.x, ClosestApex.y,
						sourcePos.x, sourcePos.y);
				if(NodeDistanceToApex < SenderDistanceToApex)
				{
					//Retransmitted; use source position (Last known)
					double SenderDistanceToApex = m_mutils.GetDistanceBetweenPoints(ClosestApex.x, ClosestApex.y,
							sourcePos.x, sourcePos.y);
					//This if statement should always evaluate true (Sending area focused around initial node)
					if(NodeDistanceToApex < SenderDistanceToApex)
					{
						//Send packet and check effectivity
						/**
						 * Checking efficancy of transmission:
						 *
						 * (Need to have at least one neighbor)
						 * At least one of the neighbors in the nodes neighbor
						 * table must be close to the apex than the current node
						 *
						 */

						if(m_neighbors.GetNeighborTableSize() > 0)
						{
							//Condition 1 met: At least have 1 neighbor
							//Check condition 2; have a neighbor close to apex
							//If condition 2 met then send packet after backoff period

							if(m_neighbors.HaveCloserNeighbor(ClosestApex,NodeDistanceToApex))
							{

								//std::cout << "Efficent transmission -> resending after original sender sent" << std::endl;

								/**
								 * Have a neighbor closer to the apex than current position
								 * Transmission will be effective, send packet after backoff
								 */

								McastRetransmit * toSend = new McastRetransmit();

								//Create packet and add control header
								cHeader.SetSource(m_globalAddress);
								packet->AddHeader(cHeader);

								//Create type header and add to packet
								TypeHeader theader (MCAST_CONTROL);
								packet->AddHeader(theader);

								//Set packet to send
								toSend->SetPacket(packet);

								double Vj = m_neighbors.getDistanceClosestNeighborToApex(ClosestApex,NodeDistanceToApex);

								double backoff = Vj/NodeDistanceToApex;

								int delay = ((int)((10 * backoff) * 10))/10;


								toSend->timerToSend.SetFunction(&ThesisRoutingProtocol::DoSendMcastRetransmit, this);
								toSend->timerToSend.SetArguments(toSend);
								toSend->timerToSend.Schedule(Time(MilliSeconds(delay)));

								//Add to retransmit queue
								m_mr.push_back(*toSend);
							}
						}
					}
				}
			}

		}else
		{
			/**
			 * Source of packet is not a neighbor
			 * Start forwarding process anyway but set a large delay to allow
			 * better candidates to send first
			 */
			McastRetransmit * toSend = new McastRetransmit();

			//Create packet and add control header
			cHeader.SetSource(m_globalAddress);
			packet->AddHeader(cHeader);


			//Create type header and add to packet
			TypeHeader theader (MCAST_CONTROL);
			packet->AddHeader(theader);

			//Set packet to send
			toSend -> SetPacket(packet);

			//Set control header to avoid removing from packet when doing checks
			toSend -> SetControlHeader(cHeader);

			double backoff = 1;

			int delay = ((int)((10 * backoff) * 10))/10;

			//std::cout << "Mcast received but source not in "
			//		<< " not in ntable. Retransmitting" << std::endl;

			//DoSendMcastRetransmit(toSend);

			toSend->timerToSend.SetFunction(&ThesisRoutingProtocol::DoSendMcastRetransmit, this);
			toSend->timerToSend.SetArguments(toSend);
			toSend->timerToSend.Schedule(Time(MilliSeconds(delay)));

			//std::cout << "Time set, pushing in queue" << std::endl;

			//Add to retransmit queue
			m_mr.push_back(*toSend);
		}
	}else
	{
		//std::cout << "Mcast received but node not in ZoR, drop packet" << std::endl;
		//Not in ZoR; drop packet and do nothing at the moment
		return;
	}

}

void
ThesisRoutingProtocol::PurgeMcastRetransmitEntry(McastRetransmit *mr)
{
	NS_LOG_FUNCTION(this);
	//std::cout << "Attempting to purge retransmit entry" << std::endl;

	/*
	TypeHeader tHeader(HELLO);
	mr->GetPacket()->RemoveHeader(tHeader);

	std::cout << "Hello Header Removed" << std::endl;

	ControlHeader cHeader;
	mr->GetPacket() -> RemoveHeader(cHeader);

	std::cout << "Control header removed" << std::endl;
	 */

	ControlHeader cHeader = mr -> GetControlHeader();

	for(std::list<McastRetransmit>::iterator it = m_mr.begin(); it != m_mr.end(); ++it)
	{


		/*
		TypeHeader tHeader(HELLO);
		it->GetPacket()->RemoveHeader(tHeader);

		ControlHeader temp;
		it ->GetPacket()->RemoveHeader(temp);
		*/

		ControlHeader temp = it -> GetControlHeader();

		Ipv6Address Id = temp.GetId();
		Vector ApxL = temp.getApxL();
		Vector ApxR = temp.getApxR();

		if(Id.IsEqual(cHeader.GetId()) && ApxL.x == cHeader.getApxL().x && ApxL.y == cHeader.getApxL().y
				&& ApxR.x == cHeader.getApxR().x && ApxR.y == cHeader.getApxR().y)
		{
//			it->timerToSend.Cancel();
//			it ->GetPacket()->AddHeader(temp);
//			it ->GetPacket()->AddHeader(tHeader);
			m_mr.erase(it);
			//std::cout << "REMOVED" << std::endl;
			break;
		}
	}

//	mr->GetPacket()->AddHeader(cHeader);
//	mr->GetPacket()->AddHeader(tHeader);

	//std::cout << "Packet purged, going back to resend >>>>>>>>>" << std::endl;

	return;
}

void
ThesisRoutingProtocol::DoSendMcastRetransmit(McastRetransmit *mr)
{

	Ptr<Packet> packet = mr->GetPacket();

	//uint8_t *buffer = new uint8_t();
	//packet -> CopyData(buffer, 1024);

	Ptr<Packet> toSend = Create<Packet>(1024);

	toSend -> AddHeader(mr->GetControlHeader());

	TypeHeader tHeader(MCAST_CONTROL);
	toSend -> AddHeader(tHeader);

	NS_LOG_FUNCTION(this);
	//std::cout << " <<<<<<<<<<<<<<<<<<<<<<<<< Retransmitting mcast packet >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
	//std::cout << " <<<<<<<<<<<<<<<<<<<<<<<<<$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;

	//Timer expired; retransmit the packet - assume packet is correctly formed at this point
	//for (SocketListI iter = m_sendSocketList.begin (); iter != m_sendSocketList.end (); iter++ )
	//  {
	// 	uint32_t interface = iter->second;

	//Send packet
	Ipv6Address destination = Ipv6Address(MCAST_CONTROL_GRP);
	NS_LOG_LOGIC("Sending packet to: " << destination);
	m_mctrlSocket->SendTo(toSend, 0, Inet6SocketAddress(destination, MCAST_PORT));

	  //}
	//Purge mcast entry from list of entries to retransmit
	PurgeMcastRetransmitEntry(mr);

}


bool
ThesisRoutingProtocol::IsMyOwnAddress (Ipv6Address src)
{
  NS_LOG_FUNCTION (this << src);
  if(src.IsEqual(m_globalAddress))
  {
  	return true;
  }
  return false;
}

void
ThesisRoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
  NS_LOG_FUNCTION (this << stream);

  std::ostream* os = stream->GetStream ();

  *os << "Node: " << m_ipv6->GetObject<Node> ()->GetId ()
      << " Time: " << Simulator::Now ().GetSeconds () << "s "
      << "Ipv6 ThesisRouting table" << std::endl;


  if(!m_routes.empty())
  {
  	*os << "Destination                    Next Hop                                    If" << std::endl;
  	for (RoutesCI it = m_routes.begin(); it != m_routes.end(); it++)
  	{
  	 	ThesisRoutingTableEntry* route = it->first;
  	 	std::ostringstream dest, gw/*, mask*/;

  	 	dest << route -> GetDestNetwork() << "/" << int(route->GetDestNetworkPrefix().GetPrefixLength());
  	 	*os << std::setiosflags (std::ios::left) << std::setw (31) << dest.str ();
  	  gw << route->GetGateway ();
  	  *os << std::setiosflags (std::ios::left) << std::setw (27) << gw.str ();



  	 	*os << std::endl;
  	}
  }

}

/*
 * TO DO
 */
void
ThesisRoutingTableEntry::SetRouteChanged(bool changed)
{
	NS_LOG_FUNCTION(this);
}

/*
 * TO DO
 */
void
ThesisRoutingTableEntry::SetRouteStatus (Status_e status)
{
	NS_LOG_FUNCTION(this);
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
	NS_LOG_FUNCTION(this);
  if (m_metric != routeMetric)
    {
      m_metric = routeMetric;
      m_changed = true;
    }
}

ThesisRoutingTableEntry::Status_e
ThesisRoutingTableEntry::GetRouteStatus (void) const
{
	return m_status;
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
	NS_LOG_FUNCTION(this);

	if(m_helloTimer.IsRunning())
	{
		m_helloTimer.Cancel();
	}
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
  			//TypeId tid = TypeId::LookupByName("ns3::Ipv6RawSocketFactory");

  			Ptr<Node> theNode = GetObject<Node> ();
  			Ptr<Socket> socket = Socket::CreateSocket (theNode, tid);
  			Inet6SocketAddress local = Inet6SocketAddress (address.GetAddress (), MCAST_PORT);
  			int ret = socket->Bind (local);
  			NS_ASSERT_MSG (ret == 0, "Bind unsuccessful to " << address);
  			socket->BindToNetDevice (m_ipv6->GetNetDevice (i));
  			socket->ShutdownRecv ();
  			socket->SetIpv6RecvHopLimit (true);
  			m_sendSocketList[socket] = i;

  			//Set global address pointer for easy reference later
  			m_globalAddress = address.GetAddress();


  			//Add multicast route (Hello)
  		  AddNetworkRouteTo (Ipv6Address(MCAST_ALL_NODE), Ipv6Prefix("ff"), i);

  		  //Add multicast route (Control)
  		  AddNetworkRouteTo (Ipv6Address(MCAST_CONTROL_GRP), Ipv6Prefix("ff"), i);

  		}

  	}

  }

  //Set Recieve Socket (HELLO)
  if (!m_recvSocket)
  {
  	NS_LOG_LOGIC ("MCAST multicast interface: adding receiving socket");


  	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		//TypeId tid = TypeId::LookupByName("ns3::Ipv6RawSocketFactory");


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

  //Set Recieve Socket (MCAST_CONTROL)
  if (!m_mctrlSocket)
  {
  	NS_LOG_LOGIC ("MCAST multicast interface: adding receiving socket for control");



  	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
//		TypeId tid = TypeId::LookupByName("ns3::Ipv6RawSocketFactory");



  	Ptr<Node> theNode = GetObject<Node> ();
  	m_mctrlSocket = Socket::CreateSocket (theNode, tid);
  	Inet6SocketAddress local = Inet6SocketAddress (MCAST_CONTROL_GRP, MCAST_PORT);
  	if(Ipv6Address(MCAST_CONTROL_GRP).IsMulticast())
  	{
	  	Ptr<UdpSocket> Udp = DynamicCast<UdpSocket>(m_mctrlSocket);
	  	if(Udp)
	  	{
	  		Udp -> MulticastJoinGroup(0,Ipv6Address(MCAST_CONTROL_GRP));
	  	}else
	  	{
	  		NS_FATAL_ERROR ("Error: Failed to join multicast group");
	  	}
  	}
  	m_mctrlSocket->Bind (local);
  	m_mctrlSocket->SetRecvCallback (MakeCallback (&ThesisRoutingProtocol::Receive, this));
  	m_mctrlSocket->SetIpv6RecvHopLimit (true);
  	m_mctrlSocket->SetRecvPktInfo (true);
  }


  Time delay = m_initialDelay + Seconds (m_rng->GetValue (0, 0.5*m_helloInterval.GetSeconds ()) );
  //m_nextUnsolicitedUpdate = Simulator::Schedule (delay, &RipNg::SendUnsolicitedRouteUpdate, this);

  if(m_sendHello)
  {
  	m_helloTimer.SetFunction(&ThesisRoutingProtocol::HelloTimerExpire, this);
  	m_helloTimer.Schedule(delay);
  }
	Ipv6RoutingProtocol::DoInitialize ();
}

void
ThesisRoutingProtocol::ScheduleSendControl()
{
	NS_LOG_FUNCTION(this);

	double v = m_rng -> GetValue();
	double eventProbability = 0.01;

	//m_globalAddress = m_ipv6 -> GetAddress(1,1).GetAddress();

	if(v < eventProbability)
	{
		//std::cout << "value: " << v << "Sending mcast packet" << std::endl;
		m_scheduleControlEvent = Simulator::Schedule(Seconds(5),&ThesisRoutingProtocol::ScheduleSendControl,this);
		m_transmitControlEvent = Simulator::Schedule(Seconds(0.),&ThesisRoutingProtocol::DoSendMcastControl, this);
	}else
	{
		//std::cout << "waiting to send" << std::endl;
		m_scheduleControlEvent = Simulator::Schedule(Seconds(5),&ThesisRoutingProtocol::ScheduleSendControl,this);
	}

}


}
}
