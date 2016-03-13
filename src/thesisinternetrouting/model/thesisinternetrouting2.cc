/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "thesisinternetrouting2.h"
#include "ns3/ipv6-route.h"

#include <iomanip>
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"

#define VANET_TO_RSU "ff02::116"
#define RSU_TO_VANET "ff02::117"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThesisInternetRoutingProtocol2");

namespace thesis
{

class DeferredRouteOutputTag : public Tag
{
public:
  DeferredRouteOutputTag (int32_t o = -1) : Tag (), m_oif (o) {}

  static TypeId GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::thesis::DeferredRouteOutputTag")
      .SetParent<Tag> ()
      .SetGroupName("thesis")
      .AddConstructor<DeferredRouteOutputTag> ()
    ;
    return tid;
  }

  TypeId  GetInstanceTypeId () const
    {
      return GetTypeId ();
    }

    int32_t GetInterface() const
    {
      return m_oif;
    }

    void SetInterface(int32_t oif)
    {
      m_oif = oif;
    }

    uint32_t GetSerializedSize () const
    {
      return sizeof(int32_t);
    }

    void  Serialize (TagBuffer i) const
    {
      i.WriteU32 (m_oif);
    }

    void  Deserialize (TagBuffer i)
    {
      m_oif = i.ReadU32 ();
    }

    void  Print (std::ostream &os) const
    {
      os << "DeferredRouteOutputTag: output interface = " << m_oif;
    }

  private:
    /// Positive if output device is fixed in RouteOutput
    int32_t m_oif;
};

NS_OBJECT_ENSURE_REGISTERED (DeferredRouteOutputTag);




NS_OBJECT_ENSURE_REGISTERED(ThesisInternetRoutingProtocol2);

//Constructor
ThesisInternetRoutingProtocol2::ThesisInternetRoutingProtocol2() :
						m_hasMcast(true), m_IsRSU(false),
						m_CheckPosition(Seconds(10)), m_IsDtnTolerant(false)
{

}

//Destructor
ThesisInternetRoutingProtocol2::~ThesisInternetRoutingProtocol2()
{

}

TypeId
ThesisInternetRoutingProtocol2::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::thesis::ThesisInternetRoutingProtocol2")
    				.SetParent<Ipv6RoutingProtocol> ()
    				.SetGroupName ("thesis")
    				.AddConstructor<ThesisInternetRoutingProtocol2> ()
    				.AddAttribute ("McastEnabled", "Determines if MCast protocol is running on the node (Default false)",
    						BooleanValue (false),
    						MakeBooleanAccessor (&ThesisInternetRoutingProtocol2::m_hasMcast),
    						MakeBooleanChecker ())
    						;
	return tid;
}

bool
ThesisInternetRoutingProtocol2::RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION (this << p << header << header.GetSourceAddress () << header.GetDestinationAddress () << idev);

	std::cout << "ROUTE INPUT" << std::endl;

	if(m_IsRSU)
	{
		RouteInputRsu(p, header,idev,ucb,mcb,lcb,ecb);
	}else
	{
		RouteInputVanet(p, header,idev,ucb,mcb,lcb,ecb);
	}
	return true;
}

bool
ThesisInternetRoutingProtocol2::RouteInputRsu (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this << header);

	//Copy passed packet to a new packet
	Ptr<Packet> packet = p -> Copy();

	if(idev == m_lo)
	{
		std::cout << "RSU RECV ON LOOPBACK <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		//Received on loopback (Will be done later to handle sending into the network
	}else if(idev == m_wi)
	{
		std::cout << "RSU RECV ON WIFI <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		//Received on wifi interface (Must be from VANET)
		//Need to send an ACK msg, add node information to a DB and forward through the internet (OR accept itself)

		//If receiving on wifi then packets should have tags

		//Create new typeHeader and peek
		mcast::TypeHeader theader (mcast::HELLO);
		packet -> PeekHeader(theader);
		if(theader.Get() == 3)
		{
			//Received type header of type internet
			packet -> RemoveHeader(theader);
			std::cout << "Type header with type: " << theader.Get() << std::cout;

			InternetHeader Ih;
			packet -> RemoveHeader(Ih);

			std::cout << "  Internet Header: " << Ih << std::endl;

			Ipv6RoutingTableEntry toHub = m_sr6 -> GetDefaultRoute();
			Ptr<Ipv6Route> route = Create<Ipv6Route> ();
			route -> SetDestination(toHub.GetDestNetwork());
			route -> SetGateway(toHub.GetGateway());
			route -> SetOutputDevice(m_ipv6 -> GetNetDevice(toHub.GetInterface()));
			route -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());

			ucb (route -> GetOutputDevice(),route, packet, header);

			std::cout << " RSU Header Properties " << header.GetDestinationAddress() << std::endl;

			std::cout << " Header Destination " << header.GetDestinationAddress() << std::endl;
			std::cout << " Header  " << header.GetSourceAddress() << std::endl;

			std::cout << "  >>>>>>>>>>>>> Route to Hub: <<<<<<<<<<<<<<<<< " << std::endl;
			std::cout << " Dest " << toHub.GetDest() << std::endl;
			std::cout << " Destination " << toHub.GetDestNetwork() << std::endl;
			std::cout << " Gateway " << toHub.GetGateway() << std::endl;
			std::cout << " Interface " << toHub.GetInterface() << std::endl;


		}else if(theader.Get() == 4)
		{
			//Type 4 is RSU to VANET (Handle later)
			//Should not really happen on RSU
		}

	}else if(idev == m_pp)
	{
		std::cout << "RSU RECV ON P2P <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		//Received on P2P interface; packet must have come from internet
		//Check header to see destination; look up destination in DB
		//Forward to other RSU if required or send out of wifi interface node is in the current area. (To be done later)

		//Create new typeHeader and peek
		mcast::TypeHeader theader (mcast::HELLO);
		packet -> PeekHeader(theader);
		if(theader.Get() == 5)
		{
			//Internet redirect from 1 RSU to another (Handle later)
		}else
		{
			//Packet from a normal internet source
			Ipv6Address destination = header.GetDestinationAddress();

			Ptr<Ipv6Route> route = Lookup(destination,m_pp);
			if(route)
			{
				ucb (route -> GetOutputDevice(),route, packet, header);
				return true;
			}

		}


	}


	return false;
}

bool
ThesisInternetRoutingProtocol2::RouteInputVanet (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this << header);

	//Copy passed packet to a new packet
	Ptr<Packet> packet = p -> Copy();


	//Input device was loopback; only packets coming through loopback should be deferred
	if(idev == m_lo)
	{
		std::cout << "VANET RECV ON LOOPBACK <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;

		uint32_t iif = (idev ? m_ipv6->GetInterfaceForDevice (idev) : -1);
		DeferredRouteOutputTag tag(iif);
		if(packet->PeekPacketTag(tag))
		{
			std::cout << "Peek finished; deferred tag found, remove and start IR" << std::endl;
		}

		//Remove DR tag
		packet -> RemovePacketTag(tag);

		int32_t forCurrentNode = m_ipv6 -> GetInterfaceForAddress(header.GetDestinationAddress());
		if(!(forCurrentNode == -1))
		{
			//For current Node
			lcb (packet, header, iif);
			return true;
		}

		//Create new typeHeader
		mcast::TypeHeader theader (mcast::INTERNET);

		//Get mobility model properties and extract values needed for header
		Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();

		Vector position = mobility -> GetPosition();
		Vector velocity = mobility -> GetVelocity();
		Time CurrentTime = Simulator::Now();

		//Instantiate new ThesisInternetRouting header.
		InternetHeader Ih(position,velocity,CurrentTime,m_IsDtnTolerant,position,velocity,m_RsuDestination);

		std::cout << "Sending IH position:  " << position << std::endl;
		std::cout << "Sending IH velocity " << velocity  << std::endl;
		std::cout << "Sending IH currentTime" << CurrentTime  << std::endl;

		//Add headers; IH first than type, read in reverse order on receving end
		packet -> AddHeader(Ih);

		packet -> AddHeader(theader);

		//Find actual route to send packet, send using UCB callback
		Ptr<Ipv6Route> route;
		Ipv6Address destination = header.GetDestinationAddress();
		Ipv6Address source = header.GetSourceAddress();

		std::cout << "Interface " << tag.GetInterface() << " Source" << source << std::endl;

		Ptr<NetDevice> ndev = m_ipv6->GetNetDevice(m_ipv6 -> GetInterfaceForAddress(source));
		route = Lookup(destination,ndev);

		//Found route; forward along
		if(route)
		{
			std::cout << ">>>>>>>>>>CALLBACK SET - ROUTE INPUT FINISHED<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
			std::cout << "Sending on interface " << m_ipv6 -> GetInterfaceForAddress(source) << std::endl;

			ucb (ndev,route, packet, header);
			return true;
		}

	}else
	{
		//Received packet on another interface; must have been wifi since only 2 interfaces

		std::cout << "Removing type header" << std::endl;
		//Create new typeHeader and peek
		mcast::TypeHeader theader (mcast::HELLO);
		packet -> PeekHeader(theader);
		if(theader.Get() == 3)
		{
			//Received type header of type internet
			packet -> RemoveHeader(theader);
			std::cout << "Type header with type: " << theader.Get() << std::cout;

			InternetHeader Ih;
			packet -> RemoveHeader(Ih);

			std::cout << "  Internet Header: " << Ih << std::endl;
		}else if(theader.Get() == 4)
		{
			//Type 4 is RSU to VANET (Handle later)
		}

	}

	return true;
}


Ptr<Ipv6Route>
ThesisInternetRoutingProtocol2::RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif,
		Socket::SocketErrno &sockerr)
{

	NS_LOG_FUNCTION("   " << this << header << oif);

	Ipv6Address destination = header.GetDestinationAddress();
	Ipv6Address source = header.GetSourceAddress();

	std::cout << "Route output: Destination: " << destination << " Source: " << source << " Is RSU? " << m_IsRSU << std::endl;

	sockerr = Socket::ERROR_NOTERROR;
	Ptr<Ipv6Route> route;

	route = Lookup(destination,oif);

	//Found a route, don't send yet though
	//Send to loopback interface to allow packet to correctly form
	//Recieve packet from loopback interface in RouteInput and then add headers or
	//make routing decisions

	if(route)
	{
		//Found a valid route (Send to loopback for further processing)
		uint32_t iif = m_ipv6->GetInterfaceForAddress(m_ipv6 -> GetAddress(1,1).GetAddress());
		DeferredRouteOutputTag tag(iif);
		NS_LOG_LOGIC("Route found - adding deferred tag to allow processing at input");
		if(!p->PeekPacketTag(tag))
		{
			std::cout << "Peek finished - No tag" << std::endl;
			p -> AddPacketTag(tag);
		}

		std::cout << "Added packet tag" << std::endl;

		//return Lookup(Ipv6Address::GetLoopback(),oif);

		std::cout << ">>>>>>>>>> ROUTE OUTPUT RETURNING LOOPBACK ROUTE WITH FOLLOWING PROPERTIES <<<<<<<<<<<<<<<<<<" << std::endl;

		std::cout << "Output Interface (OIF): " << m_ipv6->GetInterfaceForDevice(oif) << std::endl;
		std::cout << "Original destination: " << destination << std::endl;
		std::cout << "Original source     : " << source << std::endl;

		Ptr<Ipv6Route> rtentry = Create<Ipv6Route> ();
		rtentry -> SetDestination(destination);
		rtentry -> SetGateway(Ipv6Address::GetLoopback());
		rtentry -> SetOutputDevice(m_lo);
		rtentry -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());

		std::cout << "Destination:  " << rtentry ->GetDestination() << std::endl;
		std::cout << "Gateway       " << rtentry-> GetGateway() << std::endl;
		std::cout << "Source        " << rtentry -> GetSource() << std::endl;
		std::cout << "Output Device " << rtentry -> GetOutputDevice() -> GetIfIndex() << std::endl;

		std::cout << ">>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;

		return rtentry;
	}else
	{
		//No valid route route - drop
		sockerr = Socket::ERROR_NOROUTETOHOST;
		return route;
	}
}

Ptr<Ipv6Route>
ThesisInternetRoutingProtocol2::Lookup(Ipv6Address destination, Ptr<NetDevice> interface)
{
	NS_LOG_FUNCTION (this << destination << interface);

	Ptr<Ipv6Route> rtentry = 0;
	uint16_t longestMask = 0;

	for(RoutesI it = m_routes.begin(); it != m_routes.end(); it++)
	{
		ThesisInternetRoutingTableEntry2* j = it -> first;

		Ipv6Prefix mask = j->GetDestNetworkPrefix ();
		uint16_t maskLen = mask.GetPrefixLength ();
		Ipv6Address entry = j->GetDestNetwork ();

		NS_LOG_LOGIC ("Searching for route to " << destination << ", mask length " << maskLen);

		if(mask.IsMatch(destination,entry))
		{
			NS_LOG_LOGIC ("Found global network route " << j << ", mask length " << maskLen);

			/* if interface is given, check the route will output on this interface */
			if (!interface || interface == m_ipv6->GetNetDevice (j->GetInterface ()))
			{
				if (maskLen < longestMask)
				{
					NS_LOG_LOGIC ("Previous match longer, skipping");
					continue;
				}
			}
			longestMask = maskLen;

			m_RsuDestination = j->GetRsuAddress();

			Ipv6RoutingTableEntry* route = j;
			uint32_t interfaceIdx = route->GetInterface ();
			rtentry = Create<Ipv6Route> ();

			rtentry->SetSource(m_ipv6->SourceAddressSelection (interfaceIdx, route->GetDest ()));
			rtentry->SetDestination (destination);
			rtentry->SetGateway (route->GetGateway ());
			rtentry->SetOutputDevice (m_ipv6->GetNetDevice (interfaceIdx));
		}
	}

	if (rtentry)
	{
		//NS_LOG_LOGIC ("Matching route to " << rtentry->GetDestination () << " (through " << rtentry->GetGateway () << ") at the end");
		std::cout << "Route found, printing route properties: " << std::endl;
		std::cout << "Destination:  " << rtentry->GetDestination() << std::endl;
		std::cout << "Gateway       " << rtentry-> GetGateway() << std::endl;
		std::cout << "Source        " << rtentry-> GetSource() << std::endl;
		std::cout << "Reference     " << rtentry->GetReferenceCount() << std::endl;
		std::cout << "Output Device " << rtentry-> GetOutputDevice() -> GetIfIndex() << std::endl;
	}
	return rtentry;
}
/*
 ************NEEDS TO BE UPDATED******************
 */
void
ThesisInternetRoutingProtocol2::NotifyInterfaceUp (uint32_t interface)
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
}

void
ThesisInternetRoutingProtocol2::NotifyInterfaceDown (uint32_t interface)
{
	NS_LOG_FUNCTION(this << interface << "Int Down");


	RoutesI it = m_routes.begin ();
	while (it != m_routes.end ())
	{
		if(it -> first ->GetInterface() == interface)
		{
			it = m_routes.erase(it);
		}else
		{
			it++;
		}
	}

}

void
ThesisInternetRoutingProtocol2::DeleteRoute(ThesisInternetRoutingTableEntry2 *route)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
		uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
		uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::SetIpv6 (Ptr<Ipv6> ipv6)
{
	std::cout << "Setting IPv6 in routing protocol \n";

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

	//Set pointer to loopback netdevice
	m_lo = m_ipv6 -> GetNetDevice(m_ipv6 -> GetInterfaceForAddress(Ipv6Address::GetLoopback()));

	//Create loopback route
	AddNetworkRouteTo (Ipv6Address::GetLoopback(), Ipv6Prefix::GetOnes(), m_ipv6 -> GetInterfaceForAddress(Ipv6Address::GetLoopback()));

}

void
ThesisInternetRoutingProtocol2::SetRsuDatabase(Ptr<Db> db)
{
	NS_LOG_FUNCTION(this);
	m_Db = db;
}

void
ThesisInternetRoutingProtocol2::SetIsRSU(bool isRSU)
{
	m_IsRSU = isRSU;
}

void
ThesisInternetRoutingProtocol2::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
	NS_LOG_FUNCTION (this << stream);
	std::ostream* os = stream->GetStream ();

	*os << "Node: " << m_ipv6->GetObject<Node> ()->GetId ()
	      		<< " Time: " << Simulator::Now ().GetSeconds () << "s "
	      		<< "Ipv6 RIPng table" << std::endl;

	if (!m_routes.empty ())
	{
		*os << "Destination                    Next Hop                   Flag Met Ref Use If" << std::endl;
		for (RoutesIC it = m_routes.begin (); it != m_routes.end (); it++)
		{
			ThesisInternetRoutingTableEntry2* route = it->first;
			//RipNgRoutingTableEntry::Status_e status = route->GetRouteStatus();

			if (true)
			{
				std::ostringstream dest, gw, mask;

				dest << route->GetDest () << "/" << int(route->GetDestNetworkPrefix ().GetPrefixLength ());
				*os << std::setiosflags (std::ios::left) << std::setw (31) << dest.str ();
				gw << route->GetGateway ();
				*os << std::setiosflags (std::ios::left) << std::setw (27) << gw.str ();
				*os << std::setiosflags (std::ios::left) << std::setw (4) << int(route->GetRouteMetric ());
				// Ref ct not implemented
				*os << "-" << "   ";
				// Use not implemented
				*os << "-" << "   ";
				*os << route->GetInterface ();
				*os << std::endl;
			}
		}
	}
}

void
ThesisInternetRoutingProtocol2::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);

	//	ThesisInternetRoutingTableEntry* route = new ThesisInternetRoutingTableEntry (network, networkPrefix, interface);
	ThesisInternetRoutingTableEntry2* route = new ThesisInternetRoutingTableEntry2(network, networkPrefix,nextHop,interface,prefixToUse);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry2::ROUTE_VALID);
	route->SetRouteChanged (true);

	m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisInternetRoutingProtocol2::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);
	ThesisInternetRoutingTableEntry2* route = new ThesisInternetRoutingTableEntry2 (network, networkPrefix, interface);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry2::ROUTE_VALID);
	route->SetRouteChanged (true);

	m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisInternetRoutingProtocol2::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse, Ipv6Address RsuAddress)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);

	//	ThesisInternetRoutingTableEntry* route = new ThesisInternetRoutingTableEntry (network, networkPrefix, interface);
	ThesisInternetRoutingTableEntry2* route = new ThesisInternetRoutingTableEntry2(network, networkPrefix,nextHop,interface,prefixToUse);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry2::ROUTE_VALID);
	route->SetRouteChanged (true);
	route->SetRsuAddress(RsuAddress);

	m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisInternetRoutingProtocol2::DoDispose()
{
	//Dispose; update later?
}

void
ThesisInternetRoutingProtocol2::DoInitialize()
{

	if(!m_IsRSU)
	{
		SetIpToZone();
	}else if (m_IsRSU)
	{
		SetInterfacePointers();
	}

	Ipv6RoutingProtocol::DoInitialize ();

	if(m_IsRSU)
	{
		SetStaticRoutePointer();
	}

}

void
ThesisInternetRoutingProtocol2::SetIpToZone()
{
	NS_LOG_FUNCTION(this);

	Ptr<Node> theNode = GetObject<Node> ();
	Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();
	Vector position = mobility -> GetPosition();

	std::cout << "Node position: " << position << std::endl;
	DbEntry t1 = m_Db -> GetEntryForCurrentPosition(position);


	Ipv6Address network = t1.GetRsuAddress().CombinePrefix(Ipv6Prefix(64));

	std::cout << "Nearest RSU position: " << t1.GetRsuPosition() << std::endl;
	std::cout << "Vanet node network based on position: " << network << std::endl;


	Ptr<Ipv6L3Protocol> l3 = theNode ->GetObject<Ipv6L3Protocol>();

	//Try to use Ipv6L3Protocol instead of ipv6 to reset interface addresses

	for(uint32_t i = 0; i < m_ipv6 ->GetNInterfaces();i++)
	{
		for(uint32_t j = 0; j < m_ipv6 ->GetNAddresses(i);j++)
		{
			Ipv6Address currentAdd = m_ipv6 -> GetAddress(i,j).GetAddress();
			if(!currentAdd.IsLinkLocal() && !currentAdd.IsLocalhost())
			{
				if(!(currentAdd.CombinePrefix(Ipv6Prefix(64)).IsEqual(network)))
				{
					Ptr<WifiNetDevice> wifi = DynamicCast<WifiNetDevice>(m_ipv6 ->GetNetDevice(i));
					Mac48Address mac = wifi ->GetMac() ->GetAddress();

					Ipv6Address newAddress;
					newAddress = newAddress.MakeAutoconfiguredAddress(mac,network);
					std::cout << "New Address: " << newAddress << std::endl;
					std::cout << "" << std::endl;

					/*
					 * 0. Remove all routes to this interface
					 * 1. Get interface index for device
					 * 2. Remove old address
					 * 3. Add new address to interface
					 * 4. Set interface to up
					 * 5. Add default route
					 * 6. Set loopback to up
					 * 7. Add internet routes to this interface
					 */

					NotifyInterfaceDown(i);

					std::cout << "Remove Address" << " interface = " << i << " index= " << j << std::endl;
					m_ipv6 ->RemoveAddress(i,j);

					std::cout << "Adding new address: " << newAddress << std::endl;
					m_ipv6 -> AddAddress(i,newAddress);

					std::cout << "Setting interface " << i << " to up" << std::endl;
					m_ipv6 -> SetUp(i);

					Ipv6Address RsuAddress = t1.GetRsuAddress();
					//std::cout << "Adding default route, next hop:  " << nextHop << std::endl;
					AddNetworkRouteTo(Ipv6Address("::"),Ipv6Prefix::GetZero(),Ipv6Address(VANET_TO_RSU),i,Ipv6Address ("::"),RsuAddress);

					std::cout << "Setting loopback to up "<< std::endl;
					m_ipv6 -> SetUp(0);

					NotifyInterfaceUp(i);

		      Ptr<Icmpv6L4Protocol> icmpv6 = l3 ->GetIcmpv6();
		      Ptr<NdiscCache> nCache = icmpv6 -> FindCache(m_ipv6->GetNetDevice(i));

		      NdiscCache::Entry* nEntry = nCache -> Lookup(RsuAddress);

		      if(nEntry)
		      {
		      	std::cout << "Neighbor Entry FOUND!!!!" << std::endl;
		      }else
		      {
		      	//gateway neighbor not in cache; manually add to cache
		      	std::cout << "Neighbor Entry NOT FOUND!!!!" << std::endl;

		      	nEntry = nCache -> Add(RsuAddress);

		      	nEntry -> SetRouter(true);
		      	nEntry -> SetMacAddress(t1.GetRsuMacAddress());
		      	nEntry -> MarkReachable();

		      }

					std::cout << "Address refreshed to current zone" << std::endl;

					break;
				}
			}
		}
	}

	m_CheckPositionTimer.SetFunction(&ThesisInternetRoutingProtocol2::SetIpToZone,this);
	m_CheckPositionTimer.Schedule(m_CheckPosition);
}

void
ThesisInternetRoutingProtocol2::SetInterfacePointers()
{
	NS_LOG_FUNCTION(this);

	int32_t interface = -1;

	interface = GetP2pInterface();
	NS_ASSERT(interface > -1);
	m_pp = m_ipv6 ->GetNetDevice(interface);


	std::cout << "First wifi interface " << interface << std::endl;

	interface = -1;
	interface = GetWirelessInterface();
	NS_ASSERT(interface > -1);
	m_wi = m_ipv6 -> GetNetDevice(interface);

	std::cout << "First physical interface " << interface << std::endl;

}

int32_t
ThesisInternetRoutingProtocol2::GetP2pInterface()
{
	uint32_t interface = -1;

	PointToPointNetDevice p2pd;

	for(uint32_t j = 0; j < m_ipv6 -> GetNInterfaces(); j++)
	{
		if(m_ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(p2pd.GetTypeId().GetName()) == 0)
		{
			interface = j;
			break;
		}
	}
	return interface;
}

int32_t
ThesisInternetRoutingProtocol2::GetWirelessInterface()
{
	uint32_t interface = -1;
	WifiNetDevice wifi;

	for(uint32_t j = 0; j < m_ipv6 -> GetNInterfaces(); j++)
	{
		if(m_ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(wifi.GetTypeId().GetName()) == 0)
		{
			interface = j;
			break;
		}
	}
	return interface;
}

void
ThesisInternetRoutingProtocol2::SetStaticRoutePointer()
{
	Ptr<Node> theNode = GetObject<Node> ();
	Ptr<Ipv6ListRouting> lr = DynamicCast<Ipv6ListRouting>(m_ipv6 -> GetRoutingProtocol());
	Ipv6StaticRouting s;

	int16_t interface;
	for(uint32_t i = 0; i < lr ->GetNRoutingProtocols(); i++)
	{
		if(lr -> GetRoutingProtocol(i,interface) -> GetInstanceTypeId().GetName().compare(s.GetTypeId().GetName()) == 0)
		{
			m_sr6 = DynamicCast<Ipv6StaticRouting>(lr -> GetRoutingProtocol(i,interface));
			break;
		}
	}

}

void
ThesisInternetRoutingProtocol2::RemoveDefaultRoute()
{
	NS_LOG_FUNCTION(this);

	for(RoutesI it = m_routes.begin(); it != m_routes.end(); it++)
	{
		Ipv6Address destination = it -> first -> GetDestNetwork();

		Ipv6Address defaultRouteAddress("::");

//		std::cout << " >>>>>>>>>>>>>>>> Attempting to remove route with destination: <<<<<<<<<<<<<" << destination << std::endl;

		if(destination.IsEqual(defaultRouteAddress))
		{
			std::cout << " >>>>>>>>>>>>>>>> Attempting to remove route with destination: <<<<<<<<<<<<<" << destination << std::endl;
			it = m_routes.erase(it);
			return;
		}

	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 ()
: m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{

}

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
: Ipv6RoutingTableEntry ( ThesisInternetRoutingTableEntry2::CreateNetworkRouteTo (network, networkPrefix, nextHop, interface, prefixToUse) ),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
: Ipv6RoutingTableEntry ( Ipv6RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, interface) ),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse, Ipv6Address RsuAddress)
: Ipv6RoutingTableEntry ( Ipv6RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, interface)),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false),m_RsuAddress(RsuAddress)
{
}

ThesisInternetRoutingTableEntry2::~ThesisInternetRoutingTableEntry2 ()
{
}

void
ThesisInternetRoutingTableEntry2::SetRouteTag (uint16_t routeTag)
{
	m_tag = routeTag;
}

void
ThesisInternetRoutingTableEntry2::SetRouteStatus(Status_e status)
{
	m_status = status;
}

void
ThesisInternetRoutingTableEntry2::SetRouteChanged (bool changed)
{
	m_changed = changed;
}

bool
ThesisInternetRoutingTableEntry2::IsRouteChanged (void) const
{
	return m_changed;
}

void
ThesisInternetRoutingTableEntry2::SetRouteMetric (uint8_t routeMetric)
{
	m_metric = routeMetric;
}

uint8_t
ThesisInternetRoutingTableEntry2::GetRouteMetric (void) const
{
	return m_metric;
}

void
ThesisInternetRoutingTableEntry2::SetRsuAddress (Ipv6Address RsuAddress)
{
	m_RsuAddress = RsuAddress;
}

Ipv6Address
ThesisInternetRoutingTableEntry2::GetRsuAddress (void) const
{
	return m_RsuAddress;
}



}
}

