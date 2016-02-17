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

#include "ns3/Db.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ListRoutingTest");

int main (int argc, char *argv[])
{
	uint32_t nVeh = 2; //Number of vehicle
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
			"X",StringValue ("ns3::UniformRandomVariable[Min=0|Max=2000]"),
			"Y",StringValue ("ns3::UniformRandomVariable[Min=0|Max=2000]"),
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
	ThesisInternetRoutingHelper tihelper;

	//m_rng = CreateObject<UniformRandomVariable>();
	Ptr<Db> RsuDatabase = CreateObject<Db>();

	//Set blank RsuDatabase into thesis internet routing helper
	tihelper.SetRsuDatabase(RsuDatabase);
	tihelper.SetIsRSU(true);

	//Create list routing helper and add routing protocols
	Ipv6ListRoutingHelper listRH;

	listRH.Add(staticRoutingHelper,5);
	listRH.Add(tihelper,10);

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
	internet.Install(Hub);

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
	star.ScheduleCreateStaticRoutes(Seconds(4),Hub,RSU,staticRoutingHelper);

	//Populate database
	RsuDatabase -> CreateDatabase(RSU,hstep,vstep);

	Ipv6AddressHelper wifiAdd;
	wifiNetworks.SetBase(Ipv6Address("4001::"), Ipv6Prefix(64));
	wifiNetworks.Assign(VehWifiDevices);

	NS_LOG_INFO ("Run Simulation.");
	Simulator::Run ();
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

	return 0;

}
