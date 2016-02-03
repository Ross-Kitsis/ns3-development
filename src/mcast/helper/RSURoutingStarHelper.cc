/*
 * RSURoutingStarHelper.cc
 *
 *  Created on: Feb 1, 2016
 *      Author: ross
 */

#include "RSURoutingStarHelper.h"

namespace ns3
{

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
	Ipv6AddressGenerator::Init (addrBase, prefix);
	Ipv6Address v6network;
	Ipv6AddressHelper addressHelper;

	for (uint32_t i = 0; i < m_spokeDevices.GetN (); ++i)
	{
		v6network = Ipv6AddressGenerator::GetNetwork (prefix);
		addressHelper.SetBase (v6network, prefix);

		Ipv6InterfaceContainer ic = addressHelper.Assign (m_hubDevices.Get (i));
		m_hubInterfaces6.Add (ic);
		ic = addressHelper.Assign (m_spokeDevices.Get (i));
		m_spokeInterfaces6.Add (ic);

		Ipv6AddressGenerator::NextNetwork (prefix);
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

} /* namespace ns3 */
