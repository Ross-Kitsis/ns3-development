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
#include "ns3/aodv-helper.h"

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

NS_LOG_COMPONENT_DEFINE("ThesisRoutingNsMobility");

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
		//LogComponentEnable ("ThesisInternetRoutingProtocol2", LOG_LEVEL_ALL);
		//LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
		//LogComponentEnable ("AodvRoutingProtocol", LOG_LEVEL_ALL);
	}

	uint32_t nVeh = 1; //Number of vehicle
	uint32_t nRSU = 2; //Number of RSU stations
	uint32_t hstep = 1000; //Horizontal step
	uint32_t vstep = 1000; //Vertical step
	uint32_t numRsuRow = 4; //Number of RSU to place in a row
	uint32_t simTime = 50; //Simulation time
	double transmittingPercentage = 0.1; //Percentage of vanet nodes generating packets
	std::string m_CSVfileName = "AODVNSMobility.csv";
	std::string m_TraceFile = "";
	int packetSendFrequency = 1;

	CommandLine cmd;
	cmd.AddValue ("nVeh", "Number of vehicle nodes", nVeh);
	cmd.AddValue ("nRSU", "Number backbone nodes", nRSU);
	cmd.AddValue ("nRsuRow", "Number of RSU in a row", numRsuRow);
	cmd.AddValue ("nSendPerc", "Percentage of vehicular nodes acting as sources",transmittingPercentage);
	cmd.AddValue ("trace","Location of the mobility trace",m_TraceFile);
	cmd.AddValue ("simTime","Simulation time",simTime);
	cmd.AddValue ("sFreq", "Number of packets sent per second",packetSendFrequency);
	cmd.Parse (argc, argv);

	NodeContainer RSU;
	NodeContainer Hub;
	NodeContainer VehNodes;
	NodeContainer AllNodes;

	//Create Nodes
	//RSU.Create(nRSU);

	//Hub.Create(1);

	VehNodes.Create(nVeh);
	//Create vehicle nodes on differnt processors
//	MpiInterface::Enable(&argc, &argv);
//	VehNodes.Create(60,0);
//	VehNodes.Create(60,1);
//	VehNodes.Create(60,2);
//	VehNodes.Create(61,3);



	//Create p2p channel and set characteristics
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate",StringValue("10Gbps"));

	NetDeviceContainer m_hubDevices;
	NetDeviceContainer m_spokeDevices;

	//Configure P2P links
	for (uint32_t i = 0; i < nRSU ; ++i)
	{
		//NetDeviceContainer nd = pointToPoint.Install (Hub.Get (0), RSU.Get (i));
		//m_hubDevices.Add (nd.Get (0));
		//m_spokeDevices.Add (nd.Get (1));
	}



   PointToPointStarHelper star (nRSU, pointToPoint);
   Hub.Add(star.GetHub());

  for(uint32_t i = 0; i < nRSU; i++)
  {
  	RSU.Add(star.GetSpokeNode(i));
  }




/*
	for(uint32_t i = 0; i < AllNodes.GetN(); i++)
	{
		Ptr<Ipv4> v4 = AllNodes.Get(i) -> GetObject<Ipv4>();
		v4 ->SetAttribute("IpForward",BooleanValue(true));
	}
*/
//	star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.0.0", "255.255.0.0"));


	//Aggregate nodes into allNodes node container
	AllNodes.Add(RSU);
	AllNodes.Add(Hub);
	AllNodes.Add(VehNodes);

	//Create Wifi interfaces on APs
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	wifiMac.SetType ("ns3::AdhocWifiMac");

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	wifiPhy.Set ("TxPowerStart", DoubleValue (25.0) );
	wifiPhy.Set ("TxPowerEnd", DoubleValue (25.0) );
	wifiPhy.Set ("TxPowerLevels", UintegerValue(1) );
	wifiPhy.Set ("TxGain", DoubleValue (1) );
	wifiPhy.Set ("RxGain", DoubleValue (1) );

	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	wifiPhy.SetChannel (wifiChannel.Create ());

	WifiHelper wifi = WifiHelper::Default ();
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate24Mbps"), "RtsCtsThreshold", UintegerValue (0));

	NetDeviceContainer RsuWifiDevices;
	RsuWifiDevices = wifi.Install (wifiPhy, wifiMac, RSU);

	NetDeviceContainer VehWifiDevices;
	VehWifiDevices = wifi.Install (wifiPhy, wifiMac, VehNodes);

	//////////////////////////////////////////////////////////////////////////

	//Install internet stack on nodes
	InternetStackHelper internet;

  AodvHelper aodv;

  aodv.Set("EnableHello", BooleanValue(true));
  // you can configure AODV attributes here using aodv.Set(name, value)

  aodv.SetIsHub(false);
  aodv.SetIsRsu(false);
	internet.SetRoutingHelper(aodv);

  //star.InstallStack (internet);
  internet.Install(VehNodes);

  aodv.SetIsHub(false);
  aodv.SetIsRsu(true);
  internet.SetRoutingHelper(aodv);
  internet.Install(RSU);

  aodv.SetIsHub(true);
  aodv.SetIsRsu(false);
  internet.SetRoutingHelper(aodv);
  internet.Install(Hub);


	//////////////////////////////////////////////////////////////////////////

	Ipv4AddressHelper wifiNetworks;

	wifiNetworks.SetBase(Ipv4Address("20.10.0.0"), "255.255.0.0");
	wifiNetworks.Assign(VehWifiDevices);
	wifiNetworks.Assign(RsuWifiDevices);
	//wifiNetworks.Assign(m_hubDevices);
	//wifiNetworks.Assign(m_spokeDevices);

	star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.0.0", "255.255.0.0"));


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


	/////////////////////////SET vehicles to same mobility as RSU and set postions to test retransmit

	//Set hub position
	Ptr<ConstantPositionMobilityModel> HubLoc = Hub.Get(0) ->GetObject<ConstantPositionMobilityModel>();
	Vector hubPos((hstep*numRsuRow)/2 ,vstep,0);
	HubLoc -> SetPosition(hubPos);



	MobilityHelper vehMobility;

	//Create Mobility allocator and add to vehicular nodes
  vehMobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (450, 550, 550, 750)),
                             "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.1|Max=0.2]"),
                             "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));

	vehMobility.Install(VehNodes);




//  Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_TraceFile);
//  ns2.Install(VehNodes.Begin(), VehNodes.End());

	Ptr<Node> sink = Hub.Get(0);

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

	uint32_t packetSize = 1024;
	uint32_t maxPacketCount = 100;
	Time interPacketInterval = Seconds (1.0);
	UdpEchoClientHelper client (sinkAdd, port);
	client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
	client.SetAttribute ("Interval", TimeValue (interPacketInterval));
	client.SetAttribute ("PacketSize", UintegerValue (packetSize));

	Ptr<Node> source =VehNodes.Get(0);


	ApplicationContainer sourceApps;
	sourceApps = client.Install (source);
	sourceApps.Start (Seconds (5.0));
	sourceApps.Stop (Seconds (simTime -1));


  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
	aodv.PrintRoutingTableAllEvery(Seconds(5),routingStream);

	NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop(Time(Seconds(simTime)));

	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll ();


	Simulator::Run ();
///////////////////////////////////////////////////////////////////////////////////////

  Ptr<Application> app = source -> GetApplication(0);
  Ptr<UdpEchoClient> udpa = DynamicCast<UdpEchoClient>(app);

  std::cout << "Number of packets sent: " << udpa -> GetNumSourced() << std::endl;
  std::cout << "Number of packets received" << udpa -> GetNumReceived() << std::endl;
  std::cout << "Receive rate: " << udpa -> GetReceiveRate() << std::endl;
  std::cout << "Average RTT: " << udpa -> GetAverageLatency() << std::endl;

  std::cout << "----------------- Flow Monitor -------------" << std::endl;

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  uint64_t bytesTotal = 0;
  double lastRxTime=-1;
  double firstRxTime=-1;
  for (std::map<FlowId,FlowMonitor::FlowStats>::const_iterator i=stats.begin();i!=stats.end();
  		++i)
  {
  	if (firstRxTime < 0)
  		firstRxTime = i->second.timeFirstRxPacket.GetSeconds();
  	else if (firstRxTime > i->second.timeFirstRxPacket.GetSeconds() )
  		firstRxTime = i->second.timeFirstRxPacket.GetSeconds();
  	if (lastRxTime < i->second.timeLastRxPacket.GetSeconds() )
  		lastRxTime = i->second.timeLastRxPacket.GetSeconds();
  	bytesTotal = bytesTotal + i->second.rxBytes;
  }


  std::cout << "Num clients = " << 1 << " "
   << "Avg throughput = "
   << bytesTotal*8/(lastRxTime-firstRxTime)/1024
   << " kbits/sec" << std::endl;



////////////////////////////////////////////////////////////////////////////////////////////////////

	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

	return 0;

}
