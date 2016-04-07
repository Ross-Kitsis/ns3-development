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
#include "ns3/aodv-module.h"

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

#include "ns3/thesisinternetrouting2.h"

#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"


#include "ns3/mpi-interface.h"

#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AodvTest");

int main (int argc, char *argv[])
{
	int debug = 0;
	if(debug)
	{
		//Define compnents to log
		//LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
		//LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_ALL);
		//LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
		//LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ALL);
		//LogComponentEnable ("ThesisInternetRoutingProtocol2", LOG_LEVEL_ALL);
		//LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
		LogComponentEnable ("AodvRoutingProtocol", LOG_LEVEL_ALL);
	}

	uint32_t nVeh = 2; //Number of vehicle
	uint32_t nRSU = 2; //Number of RSU stations
	//uint32_t hstep = 1000; //Horizontal step
	//uint32_t vstep = 1000; //Vertical step
	uint32_t numRsuRow = 4; //Number of RSU to place in a row
	uint32_t simTime = 50; //Simulation time
	double transmittingPercentage = 0.1; //Percentage of vanet nodes generating packets
	std::string m_CSVfileName = "AODVNSMobility.csv";
	std::string m_TraceFile = "";
	int packetSendFrequency = 1;
	uint32_t packetSize = 1024;
	uint32_t maxPacketCount = 100;


	CommandLine cmd;
	cmd.AddValue ("nVeh", "Number of vehicle nodes", nVeh);
	cmd.AddValue ("nRSU", "Number backbone nodes", nRSU);
	cmd.AddValue ("nRsuRow", "Number of RSU in a row", numRsuRow);
	cmd.AddValue ("nSendPerc", "Percentage of vehicular nodes acting as sources",transmittingPercentage);
	cmd.AddValue ("trace","Location of the mobility trace",m_TraceFile);
	cmd.AddValue ("simTime","Simulation time",simTime);
	cmd.AddValue ("sFreq", "Number of packets sent per second",packetSendFrequency);
	cmd.AddValue ("pSize", "Size of packets to send", packetSize);
	cmd.AddValue ("maxPacketCount", "Maximum number of packets to send",maxPacketCount);
	cmd.Parse (argc, argv);

	NodeContainer RSU;
	NodeContainer VehNodes;

	//Create Nodes
	RSU.Create(nRSU);
	VehNodes.Create(nVeh);

	/*
	//Create p2p channel and set characteristics
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate",StringValue("10Gbps"));

	//Install P2P net devices
	NetDeviceContainer nd = pointToPoint.Install (RSU);
*/
	//Install Internet stack
	InternetStackHelper ih;

	//Create AODV
	AodvHelper aodv;

	aodv.Set("EnableHello",BooleanValue(true));

	//aodv.isHub = true;
	//aodv.isRsu = false;
	ih.SetRoutingHelper(aodv);
	ih.Install(RSU);

	//aodv.isHub = false;
	//aodv.isRsu = false;
	ih.SetRoutingHelper(aodv);
	ih.Install(VehNodes);

/*
	//Assign addresses to P2P interfaces
	Ipv4AddressHelper v4Helper("10.1.0.0", "255.255.0.0");
	v4Helper.Assign(nd);
*/

	// Create static grid and install onto RSU
	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0),
			"MinY", DoubleValue (0),
			"DeltaX", DoubleValue (100),
			"DeltaY", DoubleValue (0),
			"GridWidth", UintegerValue (4),
			"LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install(VehNodes);
	mobility.Install(RSU);

	//Create Wifi interfaces on APs
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	wifiMac.SetType ("ns3::AdhocWifiMac");

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	//wifiPhy.Set ("TxPowerStart", DoubleValue (25.0) );
	//wifiPhy.Set ("TxPowerEnd", DoubleValue (25.0) );
	//wifiPhy.Set ("TxPowerLevels", UintegerValue(1) );
	//wifiPhy.Set ("TxGain", DoubleValue (1) );
	//wifiPhy.Set ("RxGain", DoubleValue (1) );

	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate24Mbps"), "RtsCtsThreshold", UintegerValue (0));

	NetDeviceContainer RsuWifiDevices;
//	RsuWifiDevices = wifi.Install (wifiPhy, wifiMac, RSU.Get(0));
	RsuWifiDevices = wifi.Install (wifiPhy, wifiMac, RSU);

	NetDeviceContainer VehWifiDevices;
	VehWifiDevices = wifi.Install (wifiPhy, wifiMac, VehNodes);
	//////////////////////////////////////////////////////////////////////////


	//Assign addresses to Wifi interfaces
//	Ipv4AddressHelper v4WifiHelper("22.22.0.0", "255.255.0.0");
//	v4WifiHelper.Assign(RsuWifiDevices);
//	v4WifiHelper.Assign(VehWifiDevices);


//	Assign addresses globally
	Ipv4AddressHelper globalV4Helper("15.14.0.0","255.255.0.0");
//	globalV4Helper.Assign(nd);
	globalV4Helper.Assign(RsuWifiDevices);
	globalV4Helper.Assign(VehWifiDevices);



	//Install applications (Server)
	Ptr<Node> sink = RSU.Get(1);

	uint16_t port = 9;  // well-known echo port number
	UdpEchoServerHelper server (port);
	ApplicationContainer apps = server.Install (sink);
	apps.Start (Seconds (0));
	apps.Stop (Seconds (simTime -1));

	Ptr<Ipv4> sinkv4 = sink -> GetObject<Ipv4>();
	Ipv4Address sinkAdd = sinkv4 -> GetAddress(1,0).GetLocal();

	std::cout << "-----------------------" << std::endl;
	std::cout << "Sink Add: " << sinkAdd << std::endl;
	std::cout << "-----------------------" << std::endl;

	//Install application (Source)
	Ptr<Node> source = VehNodes.Get(0);
	ApplicationContainer sourceApps;

	Time interPacketInterval = Seconds (1.0);
	UdpEchoClientHelper client (sinkAdd, port);
	client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
	client.SetAttribute ("Interval", TimeValue (interPacketInterval));
	client.SetAttribute ("PacketSize", UintegerValue (packetSize));
	sourceApps = client.Install (source);
	sourceApps.Start (Seconds (6.0));
	sourceApps.Stop (Seconds (simTime -1));


	for(uint32_t i = 0; i < RSU.GetN(); i++)
	{
		//Ptr<Ipv4> rv4 = RSU.Get(i) -> GetObject<Ipv4>();
		//rv4 -> SetAttribute("IpForward",BooleanValue(true));
	}


  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
	aodv.PrintRoutingTableAllEvery(Seconds(5),routingStream);



	NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop(Time(Seconds(100)));
	Simulator::Run ();

	std::cout << "Num sourced: " << DynamicCast<UdpEchoClient>(source -> GetApplication(0)) ->GetNumSourced()<< std::endl;
	std::cout << "Num received: " << DynamicCast<UdpEchoClient>(source -> GetApplication(0)) ->GetNumReceived()<< std::endl;




	Simulator::Destroy();

	return 0;
}




