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
#include "ns3/GeoQueryClient.h"

#include "ns3/Db.h"

#include "ns3/ThesisUdpEchoHelper.h"
#include "ns3/ThesisUdpEchoServer.h"
#include "ns3/ThesisUdpEchoClient.h"

#include "ns3/thesisinternetrouting2.h"


#include "ns3/mpi-interface.h"

#include "ns3/flow-monitor.h"

#include "ns3/GeoClientHelper.h"
#include "ns3/GeoQueryServerHelper.h"
#include "ns3/GeoQueryClient.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThesisRoutingGeo");

int main (int argc, char *argv[])
{
	int debug = 0;
	if(debug)
	{
		//Define compnents to log
		LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
		LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_ALL);
	  //LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
		LogComponentEnable ("Ipv6Interface", LOG_LEVEL_ALL);
		LogComponentEnable ("ThesisInternetRoutingProtocol2", LOG_LEVEL_ALL);
		//LogComponentEnable ("ThesisUdpEchoServerApplication", LOG_LEVEL_ALL);

	}

	uint32_t nVeh = 2; //Number of vehicle
	uint32_t nRSU = 2; //Number of RSU stations
	uint32_t hstep = 1000; //Horizontal step
	uint32_t vstep = 1000; //Vertical step
	uint32_t numRsuRow = 4; //Number of RSU to place in a row
	uint32_t simTime = 20; //Simulation time
	uint32_t packetSize = 1024; //Size of packet to send in bytes
	uint32_t maxPacketCount = 200; //Maximum number of packets to send
	double transmittingPercentage = 0.1; //Percentage of vanet nodes generating packets
	std::string m_CSVfileName = "GeoQuery.csv";
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
	cmd.AddValue ("pSize" , "Size of packet to send", packetSize);
	cmd.AddValue ("maxPacketCount" , "Maximum number of packets to send", maxPacketCount);
	cmd.Parse (argc, argv);

	NodeContainer RSU;
	NodeContainer Hub;
	NodeContainer VehNodes;
	NodeContainer AllNodes;

	//Create Nodes
	RSU.Create(nRSU);

	Hub.Create(1);

	VehNodes.Create(nVeh);
	//Create vehicle nodes on differnt processors
//	MpiInterface::Enable(&argc, &argv);
//	VehNodes.Create(60,0);
//	VehNodes.Create(60,1);
//	VehNodes.Create(60,2);
//	VehNodes.Create(61,3);


	//Aggregate nodes into allNodes node container
	AllNodes.Add(RSU);
	AllNodes.Add(Hub);
	AllNodes.Add(VehNodes);

	//Create p2p channel and set characteristics
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate",StringValue("10Gbps"));

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


//	mobility.Install(VehNodes);

	/////////////////////////SET vehicles to same mobility as RSU and set postions to test retransmit

	//Set hub position
	Ptr<ConstantPositionMobilityModel> HubLoc = Hub.Get(0) ->GetObject<ConstantPositionMobilityModel>();
	Vector hubPos((hstep*numRsuRow)/2 ,vstep,0);
	HubLoc -> SetPosition(hubPos);


/*
	MobilityHelper vehMobility;

	//Create Mobility allocator and add to vehicular nodes
  vehMobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (450, 550, 550, 750)),
                             "Speed", StringValue ("ns3::UniformRandomVariable[Min=5.0|Max=15.0]"),
                             "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));

	vehMobility.Install(VehNodes);
*/
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_TraceFile);
  ns2.Install(VehNodes.Begin(), VehNodes.End());

/*
	//Set VehNode position 0 (Sending node)
	Ptr<ConstantPositionMobilityModel> VehNode0 = VehNodes.Get(0) ->GetObject<ConstantPositionMobilityModel>();
	Vector vehPos0(500,700,0);
	VehNode0 -> SetPosition(vehPos0);

	//Set VehNode position 1 (Retransmitting node)
	Ptr<ConstantPositionMobilityModel> VehNode1 = VehNodes.Get(1) ->GetObject<ConstantPositionMobilityModel>();
	Vector vehPos1(500,600,0);
	VehNode1 -> SetPosition(vehPos1);
*/
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
	wifiPhy.Set ("TxPowerStart", DoubleValue (20.0) );
	wifiPhy.Set ("TxPowerEnd", DoubleValue (20.0) );
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

  NS_LOG_INFO ("Create Applications.");

  uint16_t port = 123;  // well-known echo port number
/*
  ThesisUdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (sink);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (simTime -1));
*/

  //Install geoquery server on all nodes
  /*
  for(uint32_t i = 0; i < VehNodes.GetN(); i++)
  {
  	Ptr<Node> sink = VehNodes.Get(i);

  	Ipv6Address sinkAdd = sink -> GetObject<Ipv6>() -> GetAddress(1,1).GetAddress();

  	NS_LOG_INFO ("Add Geo Servers to all Veh Nodes");

  	//uint16_t port = 123;  // well-known echo port number
  	GeoQueryServerHelper server (port);
  	ApplicationContainer apps = server.Install (sink);
  	apps.Start (Seconds (1.0));
  	apps.Stop (Seconds (simTime -1));
  }*/

  //Server integrated into routing protocol now

	////////////////// Create container with nodes which will be sending data
	//Will need to be modified later to add more source nodes
	NodeContainer SourceNodes;

	//Populate SourceNodes
	uint32_t numToAdd = std::ceil(VehNodes.GetN() * transmittingPercentage);
	for(uint32_t i = 0; i < numToAdd;i++)
	{
		SourceNodes.Add(VehNodes.Get(i));
	}

	Ptr<Node> source;
	Ipv6Address sourceAdd;
	ApplicationContainer sourceApps;

	for(uint32_t i = 0; i < SourceNodes.GetN();i++)
	{
		source = SourceNodes.Get(i);
		sourceAdd = source -> GetObject<Ipv6>() -> GetAddress(1,1).GetAddress();
		//
		// Create a GeoQuery application hub
		//
		//uint32_t packetSize = 1024;
	  //uint32_t maxPacketCount = 200;
	  Time interPacketInterval = Seconds (1.0/packetSendFrequency);
		GeoClientHelper geoclient(port);
		geoclient.SetRsuDatabase(RsuDatabase);

		geoclient.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
		geoclient.SetAttribute ("Interval", TimeValue (interPacketInterval));

	 	sourceApps = geoclient.Install (source);
	  sourceApps.Start (Seconds (6.0));
	  sourceApps.Stop (Seconds (simTime -1));
	}

///////////////////////////////////////////////////////

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
	Simulator::Stop(Time(Seconds(simTime)));
	Simulator::Run ();

	///////////////////// Try to print values (To CSV now)
/*
	Ptr<Ipv6> sourcev6 = source -> GetObject<Ipv6>();
	Ptr<Ipv6ListRouting> lr = DynamicCast<Ipv6ListRouting>(sourcev6 -> GetRoutingProtocol());
	thesis::ThesisInternetRoutingProtocol2 th;
	Ptr<thesis::ThesisInternetRoutingProtocol2> toExtract;


	int16_t interface;
	for(uint32_t i = 0; i < lr ->GetNRoutingProtocols(); i++)
	{
		if(lr -> GetRoutingProtocol(i,interface) -> GetInstanceTypeId().GetName().compare(th.GetTypeId().GetName()) == 0)
		{
			toExtract = DynamicCast<thesis::ThesisInternetRoutingProtocol2>(lr -> GetRoutingProtocol(i,interface));
			break;
		}
	}
*/

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  //Build first row with simulation parameters
  out << "TotalSimulationTime," << simTime << ",SendingPercentage," << transmittingPercentage <<
  		   ",NumRsu," << nRSU << ",TotalNumberOfVehicles," << nVeh <<
  		   ",SimulationArea (Km^2)," << ((nRSU/numRsuRow * hstep) * (nRSU/numRsuRow * vstep))/ (1000 * 1000) <<
  		   ",PacketSendFrequency: ," << packetSendFrequency << " packets/s" <<
  		   ",Packet every," << 1.0/ packetSendFrequency << "s" <<std::endl;

  //Build second row of values with node numbers
  out << ""<<",";
  for(uint32_t i = 0; i < SourceNodes.GetN(); i++)
  {
  	Ptr<Node> sNode = SourceNodes.Get(i);
  	out << "Node: " << sNode->GetId() << ",";
  }
  out << std::endl;

  //Print number of packets sent
  out << "NumSent,";
  for(uint32_t i = 0; i < SourceNodes.GetN(); i++)
  {
  	Ptr<Node> sNode = SourceNodes.Get(i);
  	Ptr<Application> app = sNode -> GetApplication(0);
  	//Ptr<ThesisUdpEchoClient> udpEcho =  DynamicCast<ThesisUdpEchoClient>(app);
  	//Ptr<Ipv6> sNodev6 = sNode -> GetObject<Ipv6>();
  	//Ptr<thesis::ThesisInternetRoutingProtocol2> sntr = DynamicCast<thesis::ThesisInternetRoutingProtocol2>(sNodev6 -> GetRoutingProtocol());
  	Ptr<GeoQueryClient> gc =  DynamicCast<GeoQueryClient>(app);

  	out << gc -> GetNumSourced() << ",";
  }
  out << std::endl;

  //Print number of packets received
  out << "NumReceived,";
  for(uint32_t i = 0; i < SourceNodes.GetN(); i++)
  {
  	Ptr<Node> sNode = SourceNodes.Get(i);
  	Ptr<Application> app = sNode -> GetApplication(0);
  	//Ptr<ThesisUdpEchoClient> udpEcho =  DynamicCast<ThesisUdpEchoClient>(app);
  	//Ptr<Ipv6> sNodev6 = sNode -> GetObject<Ipv6>();
  	//Ptr<thesis::ThesisInternetRoutingProtocol2> sntr = DynamicCast<thesis::ThesisInternetRoutingProtocol2>(sNodev6 -> GetRoutingProtocol());


  	Ptr<GeoQueryClient> gc =  DynamicCast<GeoQueryClient>(app);
  	out << gc -> GetNumReceived() << ",";

  }
  out << std::endl;

  //Print number of packets received
  out << "ReceiveRate,";
  for(uint32_t i = 0; i < SourceNodes.GetN(); i++)
  {
  	Ptr<Node> sNode = SourceNodes.Get(i);
  	Ptr<Application> app = sNode -> GetApplication(0);
  	//Ptr<ThesisUdpEchoClient> udpEcho =  DynamicCast<ThesisUdpEchoClient>(app);
  	//Ptr<Ipv6> sNodev6 = sNode -> GetObject<Ipv6>();
  	//Ptr<thesis::ThesisInternetRoutingProtocol2> sntr = DynamicCast<thesis::ThesisInternetRoutingProtocol2>(sNodev6 -> GetRoutingProtocol());

  	//out << sntr -> GetReceiveRate() << ",";

  	Ptr<GeoQueryClient> gc =  DynamicCast<GeoQueryClient>(app);
  	out << gc -> GetReceiveRate() << ",";

  }
  out << std::endl;

  //Print average route trip time
  out << "AverageRTT,";
  for(uint32_t i = 0; i < SourceNodes.GetN(); i++)
  {
  	Ptr<Node> sNode = SourceNodes.Get(i);
  	Ptr<Application> app = sNode -> GetApplication(0);
  	//Ptr<ThesisUdpEchoClient> udpEcho =  DynamicCast<ThesisUdpEchoClient>(app);
  	//Ptr<Ipv6> sNodev6 = sNode -> GetObject<Ipv6>();
  	//Ptr<thesis::ThesisInternetRoutingProtocol2> sntr = DynamicCast<thesis::ThesisInternetRoutingProtocol2>(sNodev6 -> GetRoutingProtocol());

  	//out << sntr -> GetAverageLatency() << ",";
  	Ptr<GeoQueryClient> gc =  DynamicCast<GeoQueryClient>(app);
  	out << gc -> GetAverageLatency() << ",";

  }
  out << std::endl;

  out << std::endl;
  out << std::endl;
	////////////////////////////////////////////

	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");

	return 0;

}
