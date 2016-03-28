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
//#include "ns3/mcast-helper2.h"
#include "ns3/names.h"

#include <vector>
#include <stdint.h>
#include <sstream>
#include <fstream>

#include "ns3/thesisinternetrouting2.h"
#include "ns3/thesisinternetrouting-helper2.h"
#include "ns3/ThesisPing6Helper.h"

#include "ns3/Db.h"

#include "ns3/ThesisUdpEchoHelper.h"
#include "ns3/ThesisUdpEchoServer.h"
#include "ns3/ThesisUdpEchoClient.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ListRoutingTest2");

int main (int argc, char *argv[])
{
	int debug = 1;
	if(debug)
	{
		//Define compnents to log
		//LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
		//LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_ALL);
	  //LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
		//LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ALL);
		LogComponentEnable ("ThesisInternetRoutingProtocol2", LOG_LEVEL_ALL);
		//LogComponentEnable ("ThesisUdpEchoServerApplication", LOG_LEVEL_ALL);

	}

	uint32_t nVeh = 1; //Number of vehicle
	uint32_t nRSU = 2; //Number of RSU stations
	uint32_t hstep = 1000; //Horizontal step
	uint32_t vstep = 1000; //Vertical step
	uint32_t numRsuRow = 4; //Number of RSU to place in a row

	CommandLine cmd;
	cmd.AddValue ("nVeh", "Number of vehicle nodes", nVeh);
	cmd.AddValue ("nRSU", "Number backbone nodes", nRSU);
	cmd.Parse (argc, argv);

	NodeContainer RSU;
	NodeContainer Hub;
	NodeContainer VehNodes;
	NodeContainer AllNodes;

	//Create Nodes
	RSU.Create(nRSU);

	Hub.Create(1);

	VehNodes.Create(nVeh);

	//Aggregate nodes into allNodes node container
	AllNodes.Add(RSU);
	AllNodes.Add(Hub);
	AllNodes.Add(VehNodes);

	//Create p2p channel and set characteristics
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate",StringValue("1Mbps"));
	pointToPoint.SetChannelAttribute("Delay",StringValue("200ms"));

	// Create static grid and install onto RSU
	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (hstep/2),
			"MinY", DoubleValue (vstep/2),
			"DeltaX", DoubleValue (hstep),
			"DeltaY", DoubleValue (vstep),
			"GridWidth", UintegerValue (numRsuRow),
			"LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	mobility.Install(RSU);
	mobility.Install(Hub);

	MobilityHelper VehicleMobility;
	VehicleMobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
			"X",StringValue ("ns3::UniformRandomVariable[Min=400|Max=600]"),
			"Y",StringValue ("ns3::UniformRandomVariable[Min=400|Max=600]"),
			"Z",StringValue ("ns3::UniformRandomVariable[Min=0|Max=0]")
	);

	VehicleMobility.Install(VehNodes);

	//Set hub position
	Ptr<ConstantPositionMobilityModel> HubLoc = Hub.Get(0) ->GetObject<ConstantPositionMobilityModel>();
	Vector hubPos((hstep*numRsuRow)/2 ,vstep,0);
	HubLoc -> SetPosition(hubPos);

	//Install internet stack on nodes
	InternetStackHelper internet;

	//Place RSU nodes into star topology
	RSURoutingStarHelper star(Hub,RSU,pointToPoint);

	//Create Routing Helpers
	Ipv6StaticRoutingHelper staticRoutingHelper;
	ThesisInternetRoutingHelper2 tihelper;

	//m_rng = CreateObject<UniformRandomVariable>();
	Ptr<Db> RsuDatabase = CreateObject<Db>();

	//Set blank RsuDatabase into thesis internet routing helper
	tihelper.SetRsuDatabase(RsuDatabase);
	tihelper.SetIsRSU(true);

	//Create list routing helper and add routing protocols
	Ipv6ListRoutingHelper listRH;

	listRH.Add(tihelper,15);
	listRH.Add(staticRoutingHelper,10);

	//Install routing
	internet.SetRoutingHelper(listRH);

	///////////////WIFI/////////////////////////////////

	//Create Wifi interfaces on APs
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	wifiMac.SetType ("ns3::AdhocWifiMac");

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

	NetDeviceContainer RsuWifiDevices;
	RsuWifiDevices = wifi.Install (wifiPhy, wifiMac, RSU);

	NetDeviceContainer VehWifiDevices;
	VehWifiDevices = wifi.Install (wifiPhy, wifiMac, VehNodes);

	//Install Internet onto RSUs and Hub
	internet.Install(RSU);

	/////////////////////// HUB

	//Create list routing helper and add routing protocols
	Ipv6ListRoutingHelper listRHub;

	listRHub.Add(staticRoutingHelper,10);

	//Install internet stack on HUB
	InternetStackHelper HubInternet;

	//Install routing
	HubInternet.SetRoutingHelper(listRHub);

	HubInternet.Install(Hub);
///////////////////// HUB END

	InternetStackHelper VanetInternet;
	tihelper.SetIsRSU(false);
	VanetInternet.SetRoutingHelper(tihelper);
	VanetInternet.Install(VehNodes);

	star.AssignIpv6Addresses(Ipv6Address("3115::"),Ipv6Prefix(64));

	//Assign Wifi addresses to both wifi nodes as well as RSU
	Ipv6AddressHelper wifiNetworks;

	//IPv6 Base address, split int 128 bytes for induvidual byte manupilation
	unsigned char baseAd[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	baseAd[0] = 32;
	baseAd[1] = 1;

	int addStart = 7;

	for(unsigned int i = 0; i < RsuWifiDevices.GetN(); i++)
	{
		wifiNetworks.SetBase(Ipv6Address(baseAd), Ipv6Prefix(64));
		wifiNetworks.Assign(RsuWifiDevices.Get(i));

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

	//Schedule creation of static routes between nodes
	star.ScheduleCreateStaticRoutes(Seconds(5),Hub,RSU,staticRoutingHelper);

	std::cout << "Attempting to build RSU Database ---" << std::endl;


	//Populate database
	RsuDatabase -> CreateDatabase(RSU,hstep,vstep);

	std::cout << "RSU Database built" << std::endl;


	Ipv6AddressHelper wifiAdd;
	wifiNetworks.SetBase(Ipv6Address("4001::"), Ipv6Prefix(64));
	wifiNetworks.Assign(VehWifiDevices);

	/*
	 * Create Test application to see if tags can be removed or modified
	 * Select a wireless node and have it ping the hub
	 * Result not very important at this point
	 *
	 */
////////////////////////////////////////////////////////
	Ptr<Node> source = VehNodes.Get(0);
	Ptr<Node> sink = Hub.Get(0);

	Ipv6Address sinkAdd = sink -> GetObject<Ipv6>() -> GetAddress(1,1).GetAddress();
	Ipv6Address sourceAdd = source -> GetObject<Ipv6>() -> GetAddress(1,1).GetAddress();

  NS_LOG_INFO ("Create Applications.");
//
// Create a UdpEchoServer application hub
//



  uint16_t port = 9;  // well-known echo port number
  ThesisUdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (sink);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (90.0));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 100;
  Time interPacketInterval = Seconds (3.);
  ThesisUdpEchoClientHelper client (sinkAdd, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (source);
  apps.Start (Seconds (6.0));
  apps.Stop (Seconds (90.0));



/*
	Time packetInterval = Seconds(3);
	uint32_t packetSize = 1024;
	uint32_t packetCount = 100;

	ThesisPing6Helper ping;
	ping.SetLocal(sourceAdd);
	ping.SetRemote(sinkAdd);

	ping.SetAttribute ("MaxPackets", UintegerValue (packetCount));
	ping.SetAttribute ("Interval", TimeValue (packetInterval));
	ping.SetAttribute ("PacketSize", UintegerValue (packetSize));

  ApplicationContainer apps = ping.Install (VehNodes.Get (0));
  apps.Start (Seconds (6.0));
  apps.Stop (Seconds (90.0));
*/

//////////////////////////////////////////////////////////////////////////

  Names::Add("WifiNode1", source);
  Names::Add("Hub", sink);

  //Set ipv6 forwarding
	for(uint32_t i = 0; i < AllNodes.GetN();i++)
	{
		Ptr<Ipv6> v6 = AllNodes.Get(i)->GetObject<Ipv6>();
		v6 ->SetAttribute("IpForward",BooleanValue(true));
		v6 ->SetAttribute("SendIcmpv6Redirect",BooleanValue(false));
	}

//  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
//  listRH.PrintRoutingTableEvery(Seconds(10),VehNodes.Get(0),routingStream);

	Packet::EnablePrinting();


	NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop(Time(Seconds(90)));
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

	return 0;

}
