/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "mcast.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
//#include "ns3/adhoc-wifi-mac.h" Needed for hellos?
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <limits>

namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("MobicastProtocol");
	namespace mcast
	{
	NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);

	const uint32_t RoutingProtocol::MCAST_PORT = 701;

	RoutingProtocol::RoutingProtocol() :
			HelloInterval(3),
			EnableBroadcast(true),
			EnableHello(true),


	}
}

