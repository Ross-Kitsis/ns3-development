#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/v4ping-helper.h"
#include <iostream>
#include <cmath>
#include "ns3/udp-echo-helper.h"
#include "ns3/udp-echo-client.h"
#include "ns3/dsdv-helper.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/olsr-helper.h"

using namespace ns3;

/**
 * \brief Test script.
 *
 * This script creates 1-dimensional grid topology and then ping last node from the first one:
 *
 * [10.0.0.1] <-- step --> [10.0.0.2] <-- step --> [10.0.0.3] <-- step --> [10.0.0.4]
 *
 * ping 10.0.0.4
 */
class AodvTest2
{
public:
  AodvTest2 ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /// Report results
  void Report (std::ostream & os);

private:

  // parameters
  /// Number of nodes
  uint32_t size;



  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;

  uint32_t nRSU;
  uint32_t nVeh;

  // network
  NodeContainer nodes;
  NodeContainer RsuNodes;
  NodeContainer VehNodes;
  NetDeviceContainer wifiDevices;
  NetDeviceContainer p2pDevices;
  Ipv4InterfaceContainer wifiInterfaces;
  Ipv4InterfaceContainer p2pInterfaces;

private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
};

int main (int argc, char **argv)
{

	int debug = 1;
	if(debug)
	{
		//Define compnents to log
		LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
		//LogComponentEnable ("Ipv4L4Protocol", LOG_LEVEL_ALL);
		//LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_ALL);
		//LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
		//LogComponentEnable ("Ipv4Interface", LOG_LEVEL_ALL);
		//LogComponentEnable ("ThesisInternetRoutingProtocol2", LOG_LEVEL_ALL);
		LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);
		LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
		LogComponentEnable ("AodvRoutingProtocol", LOG_LEVEL_ALL);
		LogComponentEnable ("DsdvRoutingProtocol", LOG_LEVEL_ALL );
	}

	AodvTest2 test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

AodvTest2::AodvTest2() :
  	  		size (10),
  	  		step (100),
  	  		totalTime (95),
  	  		pcap (false),
  	  		printRoutes (true),
  	  		nRSU(2),
  	  		nVeh(2)
{

}

bool
AodvTest2::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);
  cmd.AddValue ("nRsu", "Number of RSU",nRSU);
  cmd.AddValue ("nVeh", "Number of Vehicle nodes", nVeh);

  cmd.Parse (argc, argv);
  return true;
}

void
AodvTest2::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();


  //Ptr<Node> source = VehNodes.Get(1);

  Ptr<Application> app = VehNodes.Get(1) -> GetApplication(0);
  Ptr<UdpEchoClient> udpa = DynamicCast<UdpEchoClient>(app);

  std::cout << "Number of packets sent: " << udpa -> GetNumSourced() << std::endl;
  std::cout << "Number of packets received" << udpa -> GetNumReceived() << std::endl;
  std::cout << "Receive rate: " << udpa -> GetReceiveRate() << std::endl;
  std::cout << "Average RTT: " << udpa -> GetAverageLatency() << std::endl;

  Simulator::Destroy ();
}

void
AodvTest2::Report (std::ostream &)
{
}

void
AodvTest2::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";

  /*
  nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), nodes.Get (i));
    }
  */

  //Create RSU nodes
  RsuNodes.Create(nRSU);
  for (uint32_t i = 0; i < RsuNodes.GetN(); ++i)
  {
  	std::ostringstream os;
  	os << "RSU Node-" << i;
  	Names::Add (os.str (), RsuNodes.Get (i));
  }

  //Create Veh Nodes
  VehNodes.Create(nVeh);
  for (uint32_t i = 0; i < VehNodes.GetN(); ++i)
  {
  	std::ostringstream os;
  	os << "VANET Node-" << i;
  	Names::Add (os.str (), VehNodes.Get (i));
  }

  // Create static grid
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (step),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (size),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (RsuNodes);
  mobility.Install (VehNodes);

}

void
AodvTest2::CreateDevices ()
{
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate",StringValue("10Gbps"));
	p2pDevices = pointToPoint.Install(RsuNodes);

  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  wifiDevices = wifi.Install (wifiPhy, wifiMac, RsuNodes.Get(1));
  wifiDevices.Add(wifi.Install (wifiPhy, wifiMac, VehNodes));

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}

void
AodvTest2::InstallInternetStack ()
{

	 OlsrHelper olsr;

	//DsdvHelper dsdv;
	//dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (5)));
	//dsdv.Set ("SettlingTime", TimeValue (Seconds (5)));


  //AodvHelper aodv;

  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  //stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  //stack.SetRoutingHelper(dsdv);
  stack.SetRoutingHelper(olsr);
  stack.Install (VehNodes);
  stack.Install (RsuNodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  wifiInterfaces = address.Assign (wifiDevices);

//  p2pInterfaces = address.Assign(p2pDevices);

  Ipv4AddressHelper p2paddress;
  p2paddress.SetBase ("11.0.0.0", "255.0.0.0");
  p2pInterfaces = p2paddress.Assign(p2pDevices);

  Ptr<Ipv4> v4 = RsuNodes.Get(1) -> GetObject<Ipv4>();
  v4 -> SetAttribute("IpForward" , BooleanValue(true));


  if (printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
      olsr.PrintRoutingTableAllAt (Seconds (8), routingStream);
    }
}

void
AodvTest2::InstallApplications ()
{
  //V4PingHelper ping (interfaces.GetAddress (size - 1));
  //ping.SetAttribute ("Verbose", BooleanValue (true));

  //ApplicationContainer p = ping.Install (nodes.Get (0));
  //p.Start (Seconds (0));
  //p.Stop (Seconds (totalTime) - Seconds (0.001));


  ////////////////////////////////////////////////////////////////////

  Ptr<Node> sink = RsuNodes.Get(0);

  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (sink);
  apps.Start (Seconds (0));
  apps.Stop (Seconds (totalTime -1));

	Ptr<Ipv4> sinkv4 = sink -> GetObject<Ipv4>();
	Ipv4Address sinkAdd = sinkv4 -> GetAddress(1,0).GetLocal();

	std::cout << "-----------------------" << std::endl;
	std::cout << "Sink Add: " << sinkAdd << std::endl;
	std::cout << "-----------------------" << std::endl;


  Ptr<Node> source = VehNodes.Get(1);

	uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 100;
  Time interPacketInterval = Seconds (1.0);
  UdpEchoClientHelper client (sinkAdd, port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));


	ApplicationContainer sourceApps;
  sourceApps = client.Install (source);
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds (totalTime -1));

///////////////////////////////////////////////////////////////////////////////////////


  // move node away
//  Ptr<Node> node = nodes.Get (size/2);
//  Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
//  Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (1e5, 1e5, 1e5));
}
