/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "thesisinternetrouting.h"
#include "ns3/ipv6-route.h"

#include <iomanip>
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include "ns3/wifi-module.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThesisInternetRoutingProtocol");

namespace thesis
{
NS_OBJECT_ENSURE_REGISTERED(ThesisInternetRoutingProtocol);

//Constructor
ThesisInternetRoutingProtocol::ThesisInternetRoutingProtocol() :
						m_hasMcast(true), m_IsRSU(false),
						m_CheckPosition(Seconds(10)), m_IsDtnTolerant(false)
{

}

//Destructor
ThesisInternetRoutingProtocol::~ThesisInternetRoutingProtocol()
{

}

TypeId
ThesisInternetRoutingProtocol::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::thesis::ThesisInternetRoutingProtocol")
    				.SetParent<Ipv6RoutingProtocol> ()
    				.SetGroupName ("thesis")
    				.AddConstructor<ThesisInternetRoutingProtocol> ()
    				.AddAttribute ("McastEnabled", "Determines if MCast protocol is running on the node (Default false)",
    						BooleanValue (false),
    						MakeBooleanAccessor (&ThesisInternetRoutingProtocol::m_hasMcast),
    						MakeBooleanChecker ())
    						/*
	static TypeId tid = TypeId ("ns3::thesis::ThesisInternetRoutingProtocol")
    		.SetParent<Ipv6RoutingProtocol> ()
    		.SetGroupName ("thesis")
    		.AddConstructor<ThesisInternetRoutingProtocol> ()
    		.AddAttribute ("McastEnabled", "Determines if MCast protocol is running on the node (Default false)",
    				BooleanValue (false),
    				MakeBooleanAccessor (&ThesisInternetRoutingProtocol::m_hasMcast),
    				MakeBooleanChecker ())
    						 */
    						;
	return tid;
}

bool
ThesisInternetRoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << header << header.GetSourceAddress () << header.GetDestinationAddress () << idev);

  mcast::TypeHeader tHeader(mcast::INTERNET);
  p->PeekHeader(tHeader);

  std::cout << "Header type: " << tHeader.Get() << std::endl;

  if(m_IsRSU)
  {
  	//RouteInput via RouteInputRsu
  	return RouteInputRsu(p,header,idev,ucb,mcb,lcb,ecb);
  }else
  {
  	return RouteInputVanet(p,header,idev,ucb,mcb,lcb,ecb);
  }
}

bool
ThesisInternetRoutingProtocol::RouteInputRsu (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this << header);
	Ipv6Address destination = header.GetDestinationAddress();
	uint32_t iif = m_ipv6 -> GetInterfaceForDevice(idev);

	if (destination.IsMulticast ())
	{
		NS_LOG_LOGIC ("Multicast routing not supported");

		for (uint32_t j = 0; j < m_ipv6->GetNInterfaces (); j++)
		{
			for (uint32_t i = 0; i < m_ipv6->GetNAddresses (j); i++)
			{
				Ipv6InterfaceAddress iaddr = m_ipv6->GetAddress (j, i);
				Ipv6Address addr = iaddr.GetAddress ();
				if (addr.IsEqual (header.GetDestinationAddress ()))
				{
					if (j == iif)
					{
						NS_LOG_LOGIC ("For me (destination " << addr << " match)");
					}
					else
					{
						NS_LOG_LOGIC ("For me (destination " << addr << " match) on another interface " << header.GetDestinationAddress ());
					}
					lcb (p, header, iif);
					return true;
				}
				NS_LOG_LOGIC ("Address " << addr << " not a match");
			}
		}
	}
	return false;
}

bool
ThesisInternetRoutingProtocol::RouteInputVanet (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this << header);

	Ipv6Address destination = header.GetDestinationAddress();
	uint32_t iif = m_ipv6 -> GetInterfaceForDevice(idev);

//	std::cout << "Route input: Destination: " << destination << std::endl;

	if(m_IsRSU)
	{
		return false;
	}else
	{

		if(destination.IsMulticast() || destination.IsLinkLocalMulticast() || destination.IsLinkLocal())
		{
			//Handle multicast and link local packets
      NS_LOG_LOGIC ("Multicast route not supported by ThesisInternet");
			return false;
		}else
		{
			//Check if packet for current node
			for(uint32_t j = 0; j < m_ipv6->GetNInterfaces(); j++)
			{
				for (uint32_t i = 0; i < m_ipv6->GetNAddresses (j); i++)
				{
					Ipv6InterfaceAddress iaddr = m_ipv6->GetAddress (j, i);
					Ipv6Address addr = iaddr.GetAddress ();

					if(addr.IsEqual(header.GetDestinationAddress()))
					{
						//For this node (Possibly)
						if(j == iif)
						{
							Ptr<Packet> packet = p -> Copy();

							InternetHeader Iheader;
							packet -> RemoveHeader(Iheader);

							lcb(packet,header,iif);
							return true;
						}
					}

					//Destination address did not match any of this nodes addresses; queue to packet for retransmission



					//				std::cout << "Original Size: " << p -> GetSize() << std::endl;
					//				std::cout << "Copy Size: " << packet -> GetSize() << std::endl;
					//				p ->PeekHeader(Iheader);

					//  		p -> PeekHeader(Iheader);
					/////// MAY NEED TO CHANGE THIS


					//				Iheader.SetSenderPosition(Vector(1,2,3));

					//				const Ptr<const Packet> newPacket = packet ->Copy();

					//ucb()
					//NS_ASSERT(0 == 1);
		      //ecb (p, header, Socket::ERROR_NOROUTETOHOST);
				}
			}
		}
	}
	return false;
}


Ptr<Ipv6Route>
ThesisInternetRoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif,
		Socket::SocketErrno &sockerr)
{

	NS_LOG_FUNCTION("   " << this << header << oif);

	Ipv6Address destination = header.GetDestinationAddress();
	Ipv6Address source = header.GetSourceAddress();

	std::cout << "Route output: Destination: " << destination << " Source: " << source << " Is RSU? " << m_IsRSU << std::endl;

	Ptr<Ipv6Route> route;

	if(m_IsRSU)
	{
		//Node is an RSU - Sending to VANET
		std::cout << "RSU RECIEVED PACKET" << std::endl;
		sockerr = Socket::ERROR_NOROUTETOHOST;
	}else
	{
		//Node is a VANET Node

		if(destination.IsMulticast() || destination.IsLinkLocalMulticast() || destination.IsLinkLocal())
		{
			//Do nothing - do not route these protocols
			//sockerr = Socket::ERROR_NOROUTETOHOST;
			return route;
		}else
		{
			route = Lookup(destination, oif);
		}
		/*
		 * Set socket error type
		 *
		 * If route found - No error
		 * No route found - No route to host
		 *
		 */
		if (route)
		{

			sockerr = Socket::ERROR_NOTERROR;

			//Create thesis internet routing header and atatch to the packet

			Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();

			Vector position = mobility -> GetPosition();
			Vector velocity = mobility -> GetVelocity();
			Time CurrentTime = Simulator::Now();
			InternetHeader Ih(position,velocity,CurrentTime,m_IsDtnTolerant,position,velocity);
/*
			std::cout << "Route to destination: " << destination << " found" << std::endl;
			std::cout << "Destination              Gateway                Interface" << std::endl;
			std::cout << route ->GetDestination() << "     " << route -> GetGateway() << "   Interface Index: " << route->GetOutputDevice()->GetInstanceTypeId().GetName() << std::endl;
*/
			p ->AddHeader(Ih);

//			InternetHeader Ih2;
//			p ->RemoveHeader(Ih2);
		}
		else
		{
			sockerr = Socket::ERROR_NOROUTETOHOST;
		}
	}
	return route;
}

Ptr<Ipv6Route>
ThesisInternetRoutingProtocol::Lookup(Ipv6Address destination, Ptr<NetDevice> interface)
{
	NS_LOG_FUNCTION (this << destination << interface);

	Ptr<Ipv6Route> rtentry = 0;
	uint16_t longestMask = 0;

	for(RoutesI it = m_routes.begin(); it != m_routes.end(); it++)
	{
		ThesisInternetRoutingTableEntry* j = it -> first;

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

			Ipv6RoutingTableEntry* route = j;
			uint32_t interfaceIdx = route->GetInterface ();
			rtentry = Create<Ipv6Route> ();


			/*
			if (route->GetGateway ().IsAny ())
			{
				rtentry->SetSource (m_ipv6->SourceAddressSelection (interfaceIdx, route->GetDest ()));
			}
			else if (route->GetDest ().IsAny ())
					{
				rtentry->SetSource (m_ipv6->SourceAddressSelection (interfaceIdx, route->GetPrefixToUse ().IsAny () ? destination : route->GetPrefixToUse ()));
					}
			else
			{
				rtentry->SetSource (m_ipv6->SourceAddressSelection (interfaceIdx, route->GetDest ()));
			}
			*/

			rtentry -> SetSource(m_ipv6->SourceAddressSelection (interfaceIdx, route->GetDest ()));
			rtentry->SetDestination (destination);
			rtentry->SetGateway (route->GetGateway ());
			rtentry->SetOutputDevice (m_ipv6->GetNetDevice (interfaceIdx));
		}
	}

	if (rtentry)
	{
		//NS_LOG_LOGIC ("Matching route to " << rtentry->GetDestination () << " (through " << rtentry->GetGateway () << ") at the end");
		std::cout << "Route found, printing route properties: " << std::endl;
		std::cout << "Destination:  " << rtentry ->GetDestination() << std::endl;
		std::cout << "Gateway       " << rtentry-> GetGateway() << std::endl;
		std::cout << "Source        " << rtentry -> GetSource() << std::endl;
		std::cout << "Reference     " << rtentry ->GetReferenceCount() << std::endl;
		std::cout << "Output Device " << rtentry -> GetOutputDevice() -> GetIfIndex() << std::endl;
	}
	return rtentry;
}
/*
 ************NEEDS TO BE UPDATED******************
 */
void
ThesisInternetRoutingProtocol::NotifyInterfaceUp (uint32_t interface)
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
ThesisInternetRoutingProtocol::NotifyInterfaceDown (uint32_t interface)
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
ThesisInternetRoutingProtocol::DeleteRoute(ThesisInternetRoutingTableEntry *route)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
		uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol::NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
		uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol::SetIpv6 (Ptr<Ipv6> ipv6)
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
}

void
ThesisInternetRoutingProtocol::SetRsuDatabase(Ptr<Db> db)
{
	NS_LOG_FUNCTION(this);
	m_Db = db;
}

void
ThesisInternetRoutingProtocol::SetIsRSU(bool isRSU)
{
	m_IsRSU = isRSU;
}

void
ThesisInternetRoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
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
			ThesisInternetRoutingTableEntry* route = it->first;
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
ThesisInternetRoutingProtocol::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);

	//	ThesisInternetRoutingTableEntry* route = new ThesisInternetRoutingTableEntry (network, networkPrefix, interface);
	ThesisInternetRoutingTableEntry* route = new ThesisInternetRoutingTableEntry(network, networkPrefix,nextHop,interface,prefixToUse);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry::ROUTE_VALID);
	route->SetRouteChanged (true);

	m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisInternetRoutingProtocol::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);
	ThesisInternetRoutingTableEntry* route = new ThesisInternetRoutingTableEntry (network, networkPrefix, interface);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry::ROUTE_VALID);
	route->SetRouteChanged (true);

	m_routes.push_back (std::make_pair (route, EventId ()));
}


void
ThesisInternetRoutingProtocol::DoDispose()
{
	//Dispose; update later?
}

void
ThesisInternetRoutingProtocol::DoInitialize()
{


	if(!m_IsRSU)
	{
		SetIpToZone();
	}

	Ipv6RoutingProtocol::DoInitialize ();

}

void
ThesisInternetRoutingProtocol::SetIpToZone()
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
					 * Set interface to new address
					 * 1. Remove old address to interface
					 * 2. Add new address to interface
					 * 3. Set interface to up to ensure interface correctly initialized
					 * 4. Remove old default route (If it exists)
					 * 5. Set new default route.
					 */
/*
					std::cout << "Set Down" << std::endl;
					m_ipv6-> SetDown(i);
					std::cout << "Remove Address" << " interface = " << i << " index= " << j << std::endl;
					m_ipv6 ->RemoveAddress(i,j);
					std::cout << "Add new address" << std::endl;
					m_ipv6 -> AddAddress(i,newAddress);
					std::cout << "Set Up" << std::endl;
					m_ipv6 -> SetUp(i); //(CAUSES SIMULATION TO CRASH)

					//Remove default route
//					RemoveDefaultRoute();

					//NotifyInterfaceDown(i);
					//NotifyInterfaceUp(i);

					//uint32_t defaultPrefix = 0;
					Ipv6Address nextHop = t1.GetRsuAddress();

//					std::cout << "Attempting to add a default route " << " Is RSU? " << m_IsRSU << std::endl;
					//Add default route
					//  AddNetworkRouteTo (Ipv6Address ("::"), Ipv6Prefix::GetZero (), nextHop, interface, Ipv6Address ("::"));
		//			AddNetworkRouteTo(Ipv6Address("::"),Ipv6Prefix::GetZero(),nextHop,i,network);
					AddNetworkRouteTo(Ipv6Address("::"),Ipv6Prefix::GetZero(),nextHop,i,Ipv6Address ("::"));
//					std::cout << "ROUTE ADDED " << std::endl;
 *
 */
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

					Ipv6Address nextHop = t1.GetRsuAddress();
					std::cout << "Adding default route, next hop:  " << nextHop << std::endl;
					AddNetworkRouteTo(Ipv6Address("::"),Ipv6Prefix::GetZero(),nextHop,i,Ipv6Address ("::"));

					std::cout << "Setting loopback to up "<< std::endl;
					m_ipv6 -> SetUp(0);

					NotifyInterfaceUp(i);

					//m_ipv6 -> GetNetDevice(i) ->Get
		      //Ptr<Icmpv6L4Protocol> icmpv6 = ipv6->GetIcmpv6 ();
		      Ptr<Icmpv6L4Protocol> icmpv6 = l3 ->GetIcmpv6();
		      Ptr<NdiscCache> nCache = icmpv6 -> FindCache(m_ipv6->GetNetDevice(i));

		      NdiscCache::Entry* nEntry = nCache -> Lookup(nextHop);

		      if(nEntry)
		      {
		      	std::cout << "Neighbor Entry FOUND!!!!" << std::endl;
		      }else
		      {
		      	//gateway neighbor not in cache; manually add to cache
		      	std::cout << "Neighbor Entry NOT FOUND!!!!" << std::endl;

		      	nEntry = nCache -> Add(nextHop);

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

	m_CheckPositionTimer.SetFunction(&ThesisInternetRoutingProtocol::SetIpToZone,this);
	m_CheckPositionTimer.Schedule(m_CheckPosition);
}

void
ThesisInternetRoutingProtocol::RemoveDefaultRoute()
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

ThesisInternetRoutingTableEntry::ThesisInternetRoutingTableEntry ()
: m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{

}

ThesisInternetRoutingTableEntry::ThesisInternetRoutingTableEntry (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
: Ipv6RoutingTableEntry ( ThesisInternetRoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, nextHop, interface, prefixToUse) ),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisInternetRoutingTableEntry::ThesisInternetRoutingTableEntry (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
: Ipv6RoutingTableEntry ( Ipv6RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, interface) ),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisInternetRoutingTableEntry::~ThesisInternetRoutingTableEntry ()
{
}

void
ThesisInternetRoutingTableEntry::SetRouteTag (uint16_t routeTag)
{
	m_tag = routeTag;
}

void
ThesisInternetRoutingTableEntry::SetRouteStatus(Status_e status)
{
	m_status = status;
}

void
ThesisInternetRoutingTableEntry::SetRouteChanged (bool changed)
{
	m_changed = changed;
}

bool
ThesisInternetRoutingTableEntry::IsRouteChanged (void) const
{
	return m_changed;
}

void
ThesisInternetRoutingTableEntry::SetRouteMetric (uint8_t routeMetric)
{
	m_metric = routeMetric;
}

uint8_t
ThesisInternetRoutingTableEntry::GetRouteMetric (void) const
{
	return m_metric;
}

}
}

