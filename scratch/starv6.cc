#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/mobility-module.h"

#include "ns3/wifi-module.h"
#include "ns3/Db.h"

#include "ns3/RSURoutingStarHelper.h"
// Network topology (default)
//
//        n2 n3 n4              .
//         \ | /                .
//          \|/                 .
//     n1--- n0---n5            .
//          /|\                 .
//         / | \                .
//        n8 n7 n6              .
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Starv6");

int
main(int argc, char *argv[])
{
  //
  // Set up some default values for the simulation.
  //
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (137));

  // ??? try and stick 15kb/s into the data rate
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("14kb/s"));

  //
    // Default number of nodes in the star.  Overridable by command line argument.
    //

  int numNodes = 20;
  //Distance between nodes (Meters)
  int step = 10;
  //total time to run simulation
  int totalTime = 90;
  //Probability to send an mcast packet

  int row = 5;

  uint32_t length = step*step;
  uint32_t width = step;

    uint32_t nSpokes = 8;

    CommandLine cmd;
    cmd.AddValue ("numNodes","Number of nodes to include in the simulation", numNodes);
    cmd.AddValue("step","Distance between nodes on the grid (meters)",step);
    cmd.AddValue("totalTime", "Total time to run simulation",totalTime);
    cmd.Parse (argc, argv);

    nSpokes = numNodes;

    NS_LOG_INFO ("Build star topology.");
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));


    NodeContainer Hub;
    Hub.Create(1);

    NodeContainer Spokes;
    Spokes.Create(nSpokes);

    InternetStackHelper internet;
    internet.Install(Hub);
    internet.Install(Spokes);

    RSURoutingStarHelper star(Hub, Spokes,pointToPoint);
    star.AssignIpv6Addresses(Ipv6Address("2005:0:0:0:0:0:0:1"),Ipv6Prefix(64));

    NodeContainer AllNodes;
    AllNodes.Add(Hub);
    AllNodes.Add(Spokes);

    // Create static grid
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0),
                                   "MinY", DoubleValue (0),
                                   "DeltaX", DoubleValue (step * step),
                                   "DeltaY", DoubleValue (step),
                                   "GridWidth", UintegerValue (row),
                                   "LayoutType", StringValue ("RowFirst"));
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    mobility.Install (AllNodes);

    //Nodes created and places on grid; add wireless interfaces to all nodes

    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
    wifiMac.SetType ("ns3::AdhocWifiMac");

    std::cout << "Creating Wifi" << std::endl;

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

    /*
     * Yans Default helper uses the LogDistancePropogation Model
     * Default distance is 250m
     * Signals get weaker as go further away
     *
     * Good for suburban enviroments (May want to switch it later based on the
     * paper detailing ns3 propogation models)
     *
     */

    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    WifiHelper wifi = WifiHelper::Default ();
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

    NetDeviceContainer devices;
    devices = wifi.Install (wifiPhy, wifiMac, AllNodes);

    Ipv6AddressHelper address;

    //uint32_t zone = 0x1;

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

    //Create database

    Db testDb;

    testDb.CreateDatabase(AllNodes,length,width);

    /*

    NS_LOG_INFO ("Create applications.");
		//
		// Create a packet sink on the star "hub" to receive packets.
		//
		uint16_t port = 50000;
		Address hubLocalAddress (InetSocketAddress (Ipv6Address::GetAny (), port));
		PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", hubLocalAddress);
		ApplicationContainer hubApp = packetSinkHelper.Install (star.GetHub ());
		hubApp.Start (Seconds (1.0));
		hubApp.Stop (Seconds (10.0));

		//
		// Create OnOff applications to send TCP to the hub, one on each spoke node.
		//
		OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
		onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

		ApplicationContainer spokeApps;

		for (uint32_t i = 0; i < star.SpokeCount (); ++i)
			{
				AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv6Address (i), port));
				onOffHelper.SetAttribute ("Remote", remoteAddress);
				spokeApps.Add (onOffHelper.Install (star.GetSpokeNode (i)));
			}
		spokeApps.Start (Seconds (1.0));
		spokeApps.Stop (Seconds (10.0));

		NS_LOG_INFO ("Enable static global routing.");
		//
		// Turn on global static routing so we can actually be routed across the star.
		//
		//Ipv6GlobalRoutingHelper::

		NS_LOG_INFO ("Enable pcap tracing.");
		//
		// Do pcap tracing on all point-to-point devices on all nodes.
		//
		pointToPoint.EnablePcapAll ("starv6");

     *
     */
		NS_LOG_INFO ("Run Simulation.");
		Simulator::Run ();
		Simulator::Destroy ();
		NS_LOG_INFO ("Done.");

		return 0;
}
