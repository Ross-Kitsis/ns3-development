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
		m_hasMcast(true), m_IsRSU(false), m_CheckPosition(Seconds(10))
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
	return true;
}

Ptr<Ipv6Route>
ThesisInternetRoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif,
		Socket::SocketErrno &sockerr)
{
	Ptr<Ipv6Route> route;
	return route;
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
	//TODO
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
					m_ipv6 ->RemoveAddress(i,j);
					m_ipv6 -> AddAddress(i,newAddress);
					m_ipv6 ->SetUp(i);

					//Remove default route
					RemoveDefaultRoute();

					uint32_t defaultPrefix = 0;

					//Add default route
					AddNetworkRouteTo(Ipv6Address("::"),Ipv6Prefix(defaultPrefix),t1.GetRsuAddress(),i,network);
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

		if(destination.IsEqual(defaultRouteAddress))
		{
			std::cout << "Attempting to remove route with destination: " << destination << std::endl;
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

