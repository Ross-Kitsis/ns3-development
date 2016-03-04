/*
 * RSURoutingStarHelper.cc
 *
 *  Created on: Feb 1, 2016
 *      Author: ross
 */

#include "RSURoutingStarHelper.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RSURoutingStarHelper");

RSURoutingStarHelper::RSURoutingStarHelper(NodeContainer hub, NodeContainer spokes, PointToPointHelper p2p)
{
	for(uint32_t i = 0; i < spokes.GetN(); i++)
	{
		NetDeviceContainer nd = p2p.Install(hub.Get(0), spokes.Get(i));
		m_hubDevices.Add(nd.Get(0));
		m_spokeDevices.Add(nd.Get(1));
	}
}

RSURoutingStarHelper::~RSURoutingStarHelper()
{
	// TODO Auto-generated destructor stub
}

void
RSURoutingStarHelper::AssignIpv6Addresses(Ipv6Address addrBase, Ipv6Prefix prefix)
{
	/*
	Ipv6AddressGenerator::Init (addrBase, prefix);
	Ipv6Address v6network;
	Ipv6AddressHelper addressHelper;

	for (uint32_t i = 0; i < m_spokeDevices.GetN (); ++i)
	{
		v6network = Ipv6AddressGenerator::GetNetwork (prefix);

		std::cout << "Network: " << v6network << std::endl;

		addressHelper.SetBase (v6network, prefix);

		Ipv6InterfaceContainer ic = addressHelper.Assign (m_hubDevices.Get (i));
		m_hubInterfaces6.Add (ic);
		ic = addressHelper.Assign (m_spokeDevices.Get (i));
		m_spokeInterfaces6.Add (ic);

		Ipv6AddressGenerator::NextNetwork (prefix);
	}
	*/
	Ipv6AddressHelper addressHelper;
	addressHelper.SetBase(addrBase,prefix);
	for (uint32_t i = 0; i < m_spokeDevices.GetN (); ++i)
	{
		Ipv6InterfaceContainer ic = addressHelper.Assign (m_hubDevices.Get (i));
		m_hubInterfaces6.Add (ic);
		ic = addressHelper.Assign (m_spokeDevices.Get (i));
		m_spokeInterfaces6.Add (ic);

		Ipv6AddressGenerator::NextNetwork(prefix);
	}
}

Ipv6InterfaceContainer
RSURoutingStarHelper::GetHubInterfaces()
{
	return m_hubInterfaces6;
}

Ipv6InterfaceContainer
RSURoutingStarHelper::GetSpokeInterfaces()
{
	return m_spokeInterfaces6;
}

void
RSURoutingStarHelper::CreateRouteBetweenHubAndSpoke(Ptr<Node> Hub, Ptr<Node> Spoke)
{
	WifiNetDevice wi;
	PointToPointNetDevice p2pd;
	Ipv6StaticRoutingHelper StaticRouting;

	Ptr<Ipv6> hubV6 = Hub-> GetObject<Ipv6>();

	Ptr<Ipv6> spokeV6 = Spoke-> GetObject<Ipv6>();
	uint32_t spokeP2pInt = GetP2pInterface(spokeV6);

	std::cout << "Got here, spokeP2pInt: " << spokeP2pInt << std::endl;

  Ipv6Address spokeAddress = spokeV6 ->GetAddress(spokeP2pInt, 1).GetAddress();

  uint32_t hubInterface = GetHubNetworkInterface(hubV6,spokeAddress);
  std::cout << "Got here, hubInt: " << hubInterface << std::endl;

  Ipv6Address hubAddress = hubV6 ->GetAddress(hubInterface,1).GetAddress();

  Ptr<Ipv6StaticRouting> routing = StaticRouting.GetStaticRouting(spokeV6);
  routing -> SetDefaultRoute(hubAddress,spokeP2pInt,Ipv6Address("::"),1);

  std::cout << "Inital route added " << std::endl;

	//Create routes from hub to spokes
	uint32_t spokeWifiInt = GetWirelessInterface(spokeV6);
	Ipv6Address spokeWirelessAddress = spokeV6 ->GetAddress(spokeWifiInt, 1).GetAddress();

	Ptr<Ipv6StaticRouting> hubRouting = StaticRouting.GetStaticRouting(hubV6);

	std::cout << "Adding hub route to: " << spokeWirelessAddress.CombinePrefix(64) << " via: " << spokeAddress << std::endl;

	hubRouting -> AddNetworkRouteTo(spokeWirelessAddress.CombinePrefix(Ipv6Prefix(64)),Ipv6Prefix(64), spokeAddress, hubInterface, 1);

	std::cout << "Adding hub route to: " << spokeWirelessAddress.CombinePrefix(64) << " via: " << spokeAddress << "DONE---" << std::endl;


}

void
RSURoutingStarHelper::CreateStaticRoutes(NodeContainer Hub, NodeContainer Spokes, Ipv6StaticRoutingHelper StaticRouting)
{

	WifiNetDevice wi;
	PointToPointNetDevice p2pd;
	Ptr<Ipv6> hubV6 = Hub.Get(0) -> GetObject<Ipv6>();

	for(uint32_t i = 0; i < Spokes.GetN(); i++)
	{
		//Create routes from spokes to hub
		Ptr<Ipv6> spokeV6 = Spokes.Get(i) -> GetObject<Ipv6>();


		uint32_t spokeP2pInt = GetP2pInterface(spokeV6);
		Ipv6Address spokeAddress = spokeV6 ->GetAddress(spokeP2pInt, 1).GetAddress();

		uint32_t hubInterface = GetHubNetworkInterface(hubV6,spokeAddress);
		Ipv6Address hubAddress = hubV6 ->GetAddress(hubInterface,1).GetAddress();
	//	Ipv6Address hubAddress = hubV6 ->GetAddress(hubInterface,0).GetAddress();

		Ptr<Ipv6StaticRouting> routing = StaticRouting.GetStaticRouting(spokeV6);
		routing -> SetDefaultRoute(hubAddress,spokeP2pInt,Ipv6Address("::"),1);


		//Create routes from hub to spokes
		uint32_t spokeWifiInt = GetWirelessInterface(spokeV6);
		Ipv6Address spokeWirelessAddress = spokeV6 ->GetAddress(spokeWifiInt, 1).GetAddress();

		Ptr<Ipv6StaticRouting> hubRouting = StaticRouting.GetStaticRouting(hubV6);

		std::cout << "Adding hub route to: " << spokeWirelessAddress.CombinePrefix(64) << " via: " << spokeAddress << std::endl;

		hubRouting -> AddNetworkRouteTo(spokeWirelessAddress.CombinePrefix(Ipv6Prefix(64)),Ipv6Prefix(64), spokeAddress, hubInterface, 1);

	}

}

void
RSURoutingStarHelper::CreateHubStaticRoutes(NodeContainer Hub, NodeContainer Spokes, Ipv6StaticRoutingHelper StaticRouting)
{
	Ptr<Node> hub = Hub.Get(0);
	Ptr<Ipv6> hubv6 = hub -> GetObject<Ipv6>();

}

uint32_t
RSURoutingStarHelper::GetP2pInterface(Ptr<Ipv6> ipv6)
{
	uint32_t interface = 0;

	PointToPointNetDevice p2pd;

	for(uint32_t j = 0; j < ipv6 -> GetNInterfaces(); j++)
	{
		std::cout << ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName() << std::endl;
		if(ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(p2pd.GetTypeId().GetName()) == 0)
		{
			interface = j;
			break;
		}
	}
	return interface;
}

uint32_t
RSURoutingStarHelper::GetWirelessInterface(Ptr<Ipv6> ipv6)
{
	uint32_t interface = 0;
	WifiNetDevice wifi;

	for(uint32_t j = 0; j < ipv6 -> GetNInterfaces(); j++)
	{
		if(ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(wifi.GetTypeId().GetName()) == 0)
		{
			interface = j;
			break;
		}
	}
	return interface;
}

uint32_t
RSURoutingStarHelper::GetHubNetworkInterface(Ptr<Ipv6> ipv6, Ipv6Address network)
{
	uint32_t interface = 0;
	Ipv6Address address;
	PointToPointNetDevice p2pd;

	for(uint32_t j = 0; j < ipv6 -> GetNInterfaces(); j++)
	{
		if(ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(p2pd.GetTypeId().GetName()) == 0)
		{
			//		Ipv6Prefix  = ipv6 ->GetAddress(j,1).GetPrefix();


			Ipv6Address net = ipv6 ->GetAddress(j,1).GetAddress().CombinePrefix(Ipv6Prefix(64));

			std::cout << "GetHubNetworkInterface: Network -> " << network.CombinePrefix(Ipv6Prefix(64)) << std::endl;
			std::cout << "GetHubNetworkInterface: Net -> " << net << std::endl;

			if(net.IsEqual(network.CombinePrefix(Ipv6Prefix(64))))
			{
				interface = j;
				break;
			}
		}
	}

	return interface;
}

void
RSURoutingStarHelper::ScheduleCreateStaticRoutes(Time t, NodeContainer Hub, NodeContainer Spokes, Ipv6StaticRoutingHelper staticRouting)
{
  //Simulator::Schedule(t, &RSURoutingStarHelper::CreateStaticRoutes, Hub, Spokes, staticRouting);
	for(uint32_t i = 0; i < Spokes.GetN(); i++)
	{
		Ptr<Node> hubNode = Hub.Get(0);
		Ptr<Node> spokeNode = Spokes.Get(i);

		//  Simulator::Schedule (printTime, &Ipv6RoutingHelper::Print, node, stream);
		Simulator::Schedule(t,&RSURoutingStarHelper::CreateRouteBetweenHubAndSpoke,hubNode, spokeNode);
	}
}

} /* namespace ns3 */
