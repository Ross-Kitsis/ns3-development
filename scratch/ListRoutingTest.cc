/**
 *
 *											List											 List
 *										 Routing										Routing
 * 		WifiSta -------- WifiAP ------ Hub -------- WifiAp ------ WifiSta
 *						  AODV					 RIPng       RIPng					AODV
 *
 *
 */


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/bridge-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/vector.h"
#include "ns3/RSURoutingStarHelper.h"
#include "ns3/mcast-helper2.h"

#include <vector>
#include <stdint.h>
#include <sstream>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ListRoutingTest");

int main (int argc, char *argv[])
{
	uint32_t nWifi = 2; //Number of wifi nodes
	uint32_t nSta = 2; //Number of wifi stations

	CommandLine cmd;
	cmd.AddValue ("nWifi", "Number of wifi nodes", nWifi);
	cmd.AddValue ("nSta", "Number backbone nodes", nSta);
	cmd.Parse (argc, argv);

	NodeContainer BackboneNodes;
	NodeContainer BackboneAP;
	NodeContainer WifiNodes;
	NodeContainer Hub;

	//Create backbone nodes
	BackboneAP.Create(nSta);

	//Create WifiNodes
	WifiNodes.Create(nWifi);

	//Create Hub node
	Hub.Create(1);

	//Create p2p channel and set characteristics
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate",StringValue("1Gbps"));
	pointToPoint.SetChannelAttribute("Delay",StringValue("2ms"));

	//Install internet stack on nodes
	InternetStackHelper internet;

	//Create RIPng
	RipNgHelper ripNgRouting;


	//Place RSU nodes into star topology
	RSURoutingStarHelper star(Hub,BackboneAP,pointToPoint);


	//Set RipNG attributes

	//ripNgRouting.ExcludeInterface(BackboneAP.Get(0),1);
	//ripNgRouting.ExcludeInterface(BackboneAP.Get(1),1);

	//ripNgRouting.SetInterfaceMetric(BackboneAP.Get(0),1,5);
	//ripNgRouting.SetInterfaceMetric(BackboneAP.Get(1),1,5);

//	ripNgRouting.Set("UnsolicitedRoutingUpdate",TimeValue(Seconds(100000)));
//	ripNgRouting.Set("TimeoutDelay",TimeValue(Seconds(1000000)));

	//Create Mcast routing protocol
	McastHelper2 mcast;
  mcast.Set("HelloBroadcast",BooleanValue(true));

	Ipv6StaticRoutingHelper staticRoutingHelper;

	//Create List routing to allow for multiple routing protocols
	Ipv6ListRoutingHelper listRH;
	//listRH.Add(mcast,5);
//	listRH.Add(ripNgRouting,10);
	listRH.Add(staticRoutingHelper,11);
	//Install routing
	internet.SetRoutingHelper(listRH);

	//Create Wifi interfaces on APs
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	wifiMac.SetType ("ns3::AdhocWifiMac");

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

	NetDeviceContainer devices;
	devices = wifi.Install (wifiPhy, wifiMac, BackboneAP);

	//Install dual routing stack on AP nodes
	internet.Install(BackboneAP);
	internet.Install(Hub);


		//Install single routing stack on hub
	//internet.SetRoutingHelper(ripNgRouting);

//	InternetStackHelper internethub;
//	internethub.SetRoutingHelper(ripNgRouting);

//	internethub.Install(Hub);

	star.AssignIpv6Addresses(Ipv6Address("2115::1"),Ipv6Prefix(64));

	//Add hub and AP to node container for management
	BackboneNodes.Add(BackboneAP);
	BackboneNodes.Add(Hub);


	Ipv6InterfaceContainer hubInterfaces = star.GetHubInterfaces();
	hubInterfaces.SetForwarding(0,true);
	hubInterfaces.SetForwarding(1,true);

	Ipv6InterfaceContainer spokeInterfaces = star.GetSpokeInterfaces();
	spokeInterfaces.SetForwarding(0,true);
	spokeInterfaces.SetForwarding(1,true);



	//Print routing tables
	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
	listRH.PrintRoutingTableEvery(Seconds(15),BackboneNodes.Get(0),routingStream);
	listRH.PrintRoutingTableEvery(Seconds(15),BackboneNodes.Get(1),routingStream);
	ripNgRouting.PrintRoutingTableEvery(Seconds(15),BackboneNodes.Get(2),routingStream);


	Ipv6AddressHelper address;

	//IPv6 Base address, split int 128 bytes for induvidual byte manupilation
	unsigned char baseAd[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	baseAd[0] = 32;
	baseAd[1] = 1;

	int addStart = 7;
	Ipv6InterfaceContainer interfaces;
	//   address.SetBase(Ipv6Address("2001:0:0::"), Ipv6Prefix(64));
	for( unsigned int i = 0; i < devices.GetN(); i++)
	{
		address.SetBase(Ipv6Address(baseAd), Ipv6Prefix(64));
		interfaces = address.Assign(devices.Get(i));

		if(baseAd[addStart] == 255)
		{
			if(baseAd[addStart - 1] == 255)
			{

			}else
			{
				baseAd[addStart -1]++;
				baseAd[addStart] = 0;
			}
		}
		baseAd[addStart]++;

	}

	MobilityHelper mobility;
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0),
			"MinY", DoubleValue (0),
			"DeltaX", DoubleValue (200),
			"DeltaY", DoubleValue (300),
			"GridWidth", UintegerValue (1),
			"LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");


	mobility.Install(BackboneNodes);

	Ptr<ConstantPositionMobilityModel> HubLoc = Hub.Get(0) ->GetObject<ConstantPositionMobilityModel>();
	Vector hubPos(100,0,0);
	HubLoc -> SetPosition(hubPos);

	star.CreateStaticRoutes(Hub,BackboneAP,staticRoutingHelper);

	/*
	 * Routes between APs and hub created
	 *
	 * Create wifi nodes and routes to the APs?
	 *
	 */



	NS_LOG_INFO ("Run Simulation.");
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

	return 0;
}
