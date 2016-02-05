/*
 * RSURoutingStarHelper.h
 *
 *  Created on: Feb 1, 2016
 *      Author: ross
 */

#ifndef SRC_MCAST_HELPER_RSUROUTINGSTARHELPER_H_
#define SRC_MCAST_HELPER_RSUROUTINGSTARHELPER_H_

#include <string>

#include "ns3/point-to-point-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv6-interface-container.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3
{

class RSURoutingStarHelper
{
public:
	//Constructor
	RSURoutingStarHelper(NodeContainer hub, NodeContainer spokes, PointToPointHelper p2p);
	//Destructor
	virtual ~RSURoutingStarHelper();

	void AssignIpv6Addresses(Ipv6Address network, Ipv6Prefix prefix);

	Ipv6InterfaceContainer GetHubInterfaces();
	Ipv6InterfaceContainer GetSpokeInterfaces();

	/**
	 * Creates static routes between hub and spokes
	 * Spokes: Spokes have a default route towards the hub
	 * Hub: Hub has routes to wireless networks, next hop will be the spokes wired address.
	 *
	 */
	void CreateStaticRoutes(NodeContainer Hub, NodeContainer Spokes, Ipv6StaticRoutingHelper StaticRouting);

  /**
   * \brief Used for nodes with both a wireless and wired interface
   * Returns the interface index for the wired interface
   */
  uint32_t
  GetP2pInterface(Ptr<Ipv6> ipv6);

  /**
   * \brief Used for the hub node with multiple IPv6 interfaces
   * Returns the interface index with an address on the same network
   */
  uint32_t
  GetHubNetworkInterface(Ptr<Ipv6> ipv6, Ipv6Address network);

  /**
   * \brief Used for nodes with both a wireless and wired interface
   * Returns the interface index for the wireless interface
   */
  uint32_t
  GetWirelessInterface(Ptr<Ipv6> ipv6);

private:

	NetDeviceContainer m_hubDevices;
	NetDeviceContainer m_spokeDevices;
  Ipv6InterfaceContainer m_hubInterfaces6;    //!< IPv6 hub interfaces
  Ipv6InterfaceContainer m_spokeInterfaces6;  //!< IPv6 spoke nodes interfaces


};

} /* namespace ns3 */

#endif /* SRC_MCAST_HELPER_RSUROUTINGSTARHELPER_H_ */
