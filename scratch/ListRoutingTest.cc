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

#include "ns3/thesisinternetrouting.h"
#include "ns3/thesisinternetrouting-helper.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ListRoutingTest");

void SetDefaultRoutes(NodeContainer Hub, NodeContainer spokes, Ipv6StaticRoutingHelper helper, RSURoutingStarHelper star)
{
	star.CreateStaticRoutes(Hub,spokes,helper);
}

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
	NodeContainer AllWirelessNode;
	NodeContainer Hub;
	NodeContainer AllNodes;

	//Create backbone nodes
	BackboneAP.Create(nSta);

	//Create WifiNodes
	WifiNodes.Create(nWifi);

	//Create Hub node
	Hub.Create(1);

	AllNodes.Add(BackboneAP);
	AllNodes.Add(WifiNodes);
	AllNodes.Add(Hub);

	//Create p2p channel and set characteristics
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate",StringValue("1Mbps"));
	pointToPoint.SetChannelAttribute("Delay",StringValue("200ms"));

	//Install internet stack on nodes
	InternetStackHelper internet;

	//Create RIPng
	//RipNgHelper ripNgRouting;


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
	ThesisInternetRoutingHelper tihelper;

	//tihelper.

	//Create List routing to allow for multiple routing protocols
	Ipv6ListRoutingHelper listRH;
	//listRH.Add(mcast,5);
//	listRH.Add(ripNgRouting,10);

	listRH.Add(staticRoutingHelper,5);
	listRH.Add(tihelper,10);
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

	NetDeviceContainer wifiDevices;
	wifiDevices = wifi.Install (wifiPhy, wifiMac, WifiNodes);


	//Install dual routing stack on AP nodes
	internet.Install(BackboneAP);
	internet.Install(Hub);

	InternetStackHelper wifiInternet;
	wifiInternet.SetRoutingHelper(staticRoutingHelper);
	wifiInternet.Install(WifiNodes);


	star.AssignIpv6Addresses(Ipv6Address("3115::"),Ipv6Prefix(64));

	//Add hub and AP to node container for management
	BackboneNodes.Add(BackboneAP);
	BackboneNodes.Add(Hub);



	//Print routing tables
	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
	listRH.PrintRoutingTableAt(Seconds(0),BackboneNodes.Get(0),routingStream);
	listRH.PrintRoutingTableAt(Seconds(0),BackboneNodes.Get(1),routingStream);
	listRH.PrintRoutingTableAt(Seconds(0),BackboneNodes.Get(2),routingStream);
	staticRoutingHelper.PrintRoutingTableAt(Seconds(0),WifiNodes.Get(0),routingStream);
	staticRoutingHelper.PrintRoutingTableAt(Seconds(0),WifiNodes.Get(1),routingStream);


	//Print routing tables
	//Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
	listRH.PrintRoutingTableAt(Seconds(3),BackboneNodes.Get(0),routingStream);
	listRH.PrintRoutingTableAt(Seconds(3),BackboneNodes.Get(1),routingStream);
	listRH.PrintRoutingTableAt(Seconds(3),BackboneNodes.Get(2),routingStream);
	staticRoutingHelper.PrintRoutingTableAt(Seconds(3),WifiNodes.Get(0),routingStream);
	staticRoutingHelper.PrintRoutingTableAt(Seconds(3),WifiNodes.Get(1),routingStream);

	//Print routing tables
//	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
	listRH.PrintRoutingTableEvery(Seconds(10),BackboneNodes.Get(0),routingStream);
	listRH.PrintRoutingTableEvery(Seconds(10),BackboneNodes.Get(1),routingStream);
	listRH.PrintRoutingTableEvery(Seconds(10),BackboneNodes.Get(2),routingStream);
	staticRoutingHelper.PrintRoutingTableEvery(Seconds(10),WifiNodes.Get(0),routingStream);
	staticRoutingHelper.PrintRoutingTableEvery(Seconds(10),WifiNodes.Get(1),routingStream);



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
		address.Assign(wifiDevices.Get(i));
		//Assign to wifi nodes

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
	mobility.Install(WifiNodes);

	Ptr<ConstantPositionMobilityModel> HubLoc = Hub.Get(0) ->GetObject<ConstantPositionMobilityModel>();
	Vector hubPos(100,0,0);
	HubLoc -> SetPosition(hubPos);

	Ptr<ConstantPositionMobilityModel> Wnode1 = WifiNodes.Get(0) ->GetObject<ConstantPositionMobilityModel>();
	Vector wnodepos(-50,0,0);
	Wnode1 -> SetPosition(wnodepos);

	Ptr<ConstantPositionMobilityModel> Wnode2 = WifiNodes.Get(1) ->GetObject<ConstantPositionMobilityModel>();
	Vector wnodepos2(0,350,0);
	Wnode2 -> SetPosition(wnodepos2);

	for(int i = 0; i < 2; i++)
	{
		Ptr<Ipv6StaticRouting> r = staticRoutingHelper.GetStaticRouting(WifiNodes.Get(i) -> GetObject<Ipv6>());

		uint32_t gatewayint = star.GetWirelessInterface(BackboneAP.Get(i) -> GetObject<Ipv6>());
		Ptr<Ipv6> gatewayv6 = BackboneAP.Get(i) -> GetObject<Ipv6>();
		Ipv6Address gatewayadd = gatewayv6 -> GetAddress(gatewayint,1).GetAddress();
		uint32_t toSendInt = star.GetWirelessInterface(WifiNodes.Get(i)->GetObject<Ipv6>());


		r ->SetDefaultRoute(gatewayadd,toSendInt,gatewayadd.CombinePrefix(64),1);
	}

	/*

	Ipv6InterfaceContainer hubInterfaces = star.GetHubInterfaces();
	hubInterfaces.SetForwarding(0,true);
	hubInterfaces.SetForwarding(1,true);

	Ipv6InterfaceContainer spokeInterfaces = star.GetSpokeInterfaces();
	spokeInterfaces.SetForwarding(0,true);
	spokeInterfaces.SetForwarding(1,true);
*/


	/*
	 * Set forwarding on all ports on all backbone nodes to true
	 */
/*
	for(uint32_t i = 0; i < BackboneAP.GetN(); i++)
	{
		Ptr<Node> node = BackboneAP.Get(i);
		Ptr<Ipv6> nodev6 = node -> GetObject<Ipv6>();

		for(uint32_t j = 0; j < nodev6 -> GetNInterfaces(); j++)
		{
			nodev6 -> SetForwarding(j,true);
		}
	}


	Ptr<Node> node = Hub.Get(0);
	Ptr<Ipv6> hubv6 = node -> GetObject<Ipv6>();
	for(uint32_t j = 1; j < hubv6 -> GetNInterfaces(); j++)
	{
		hubv6 -> SetForwarding(j,true);
	}
*/

	for(uint32_t i = 0; i < BackboneNodes.GetN();i++)
	{
		Ptr<Ipv6> v6 = BackboneNodes.Get(i)->GetObject<Ipv6>();
		v6 ->SetAttribute("IpForward",BooleanValue(true));
	}



	std::cout << "Num backbone nodes: " << BackboneNodes.GetN() << std::endl;

/*
	 * Create applications to test routing
	 *
	 *   // We want the source to be the first node created outside of the backbone
  // Conveniently, the variable "backboneNodes" holds this node index value
  Ptr<Node> appSource = NodeList::GetNode (backboneNodes);
  // We want the sink to be the last node created in the topology.
  uint32_t lastNodeIndex = backboneNodes + backboneNodes*(lanNodes - 1) + backboneNodes*(infraNodes - 1) - 1;
  Ptr<Node> appSink = NodeList::GetNode (lastNodeIndex);
  // Let's fetch the IP address of the last node, which is on Ipv4Interface 1
  Ipv4Address remoteAddr = appSink->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();

  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     Address (InetSocketAddress (remoteAddr, port)));

  ApplicationContainer apps = onoff.Install (appSource);
  apps.Start (Seconds (3));
  apps.Stop (Seconds (stopTime - 1));

  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  apps = sink.Install (appSink);
  apps.Start (Seconds (3));
	 *
	 */

	/*
	Ptr<Node> appSource = WifiNodes.Get(0);
	Ptr<Node> appSink = WifiNodes.Get(1);
	Ipv6Address sinkAdd = appSink -> GetObject<Ipv6>() -> GetAddress(1,1).GetAddress();
	//std::cout << "sink address " << sinkAdd << std::endl;

	int port = 9;

  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     Address (Inet6SocketAddress (sinkAdd, port)));

  onoff.SetConstantRate(DataRate ("500kb/s"));
  ApplicationContainer apps = onoff.Install(appSource);
  apps.Start(Seconds(5));
  apps.Stop(Seconds(90));

  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Inet6SocketAddress (Ipv6Address::GetAny(), port));
  apps = sink.Install (appSink);
  apps.Start (Seconds (5));
*/

	Ptr<Node> source = WifiNodes.Get(0);
	Ptr<Node> sink = WifiNodes.Get(1);
	Ipv6Address sinkAdd = sink -> GetObject<Ipv6>() -> GetAddress(1,1).GetAddress();
	Ipv6Address sourceAdd = source -> GetObject<Ipv6>() -> GetAddress(1,1).GetAddress();


	std::cout << "Source address: " << sourceAdd << std::endl;
	std::cout << "Sink address: " << sinkAdd << std::endl;

	Time packetInterval = Seconds(3);
	uint32_t packetSize = 1024;
	uint32_t packetCount = 100;


	Ping6Helper ping;
	ping.SetLocal(sourceAdd);
	ping.SetRemote(sinkAdd);

  ping.SetAttribute ("MaxPackets", UintegerValue (packetCount));
  ping.SetAttribute ("Interval", TimeValue (packetInterval));
  ping.SetAttribute ("PacketSize", UintegerValue (packetSize));

  ApplicationContainer apps = ping.Install (WifiNodes.Get (0));
  apps.Start (Seconds (5.0));
  apps.Stop (Seconds (90.0));

  //Simulator::Schedule (printTime, &Ipv6RoutingHelper::Print, node, stream);

  //Simulator::Schedule(Seconds(3), &ListRoutingTest::SetDefaultRoutes, Hub, BackboneAP, staticRoutingHelper, star);

	star.ScheduleCreateStaticRoutes(Seconds(4),Hub,BackboneAP,staticRoutingHelper);

	//Insert database into ThesisInternetRoutingProtocol
	for(uint32_t i = 0; i < BackboneNodes.GetN(); i++)
	{
		Ptr<Node> node = BackboneNodes.Get(i);
		Ptr<Ipv6> nodev6 = node -> GetObject<Ipv6>();
		Ptr<Ipv6RoutingProtocol> routing = nodev6 -> GetRoutingProtocol();
		Ptr<Ipv6ListRouting> listRouting = DynamicCast<Ipv6ListRouting> (routing);

		if(listRouting)
		{
			std::cout << "Successful cast" << std::endl;

			unsigned int index = 1;
			short prio = 10;

			Ptr<Ipv6RoutingProtocol> listItem = listRouting ->GetRoutingProtocol(index,prio);


		}else
		{
			std::cout << "Unsuccessful cast" << std::endl;
		}

	}

	NS_LOG_INFO ("Run Simulation.");
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

	return 0;
}


