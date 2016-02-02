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

private:

	NetDeviceContainer m_hubDevices;
	NetDeviceContainer m_spokeDevices;
  Ipv6InterfaceContainer m_hubInterfaces6;    //!< IPv6 hub interfaces
  Ipv6InterfaceContainer m_spokeInterfaces6;  //!< IPv6 spoke nodes interfaces

};

} /* namespace ns3 */

#endif /* SRC_MCAST_HELPER_RSUROUTINGSTARHELPER_H_ */
