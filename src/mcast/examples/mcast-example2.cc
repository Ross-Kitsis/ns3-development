/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */



#include "ns3/mcast-helper2.h"
//#include "ns3/mcast.h"
//#include "ns3/mcast-neighbor.h"
#include "ns3/mcast-packet.h"
#include "ns3/MobiCastAppHelper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"

#include <iostream>
#include <cmath>


using namespace ns3;


int 
main (int argc, char *argv[])
{
  bool verbose = true;
  //Number of nodes to create
  int numNodes = 2;
  //Distance between nodes (Meters)
  int step = 5;
  //total time to run simulation
  int totalTime = 90;
  //Probability to send an mcast packet
  double eventProbability = 0.1;

  /*
  //Time to wait between successful transmissions
  Time SuccessTimer = Seconds(20);
  //Time between unsuccessful events triggered, wait to try again
  Time RetyTimer = Seconds(5);
	*/

  //Capture packets
  bool pcap = false;

  Time retryInterval = Seconds(1);
  Time successInterval = Seconds(10);
  bool SendHello = true;


  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);
  cmd.AddValue ("numNodes","Number of nodes to include in the simulation", numNodes);
  cmd.AddValue("step","Distance between nodes on the grid (meters)",step);
  cmd.AddValue("totalTime", "Total time to run simulation",totalTime);


  cmd.Parse (argc,argv);
  // network
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv6InterfaceContainer interfaces;

  std::cout << "Creating " << (unsigned)numNodes << " nodes " << step << " m apart.\n";

  //Create Nodes
   nodes.Create(numNodes);

   double min = 0.0;
   double max = 10.0;

   Ptr<UniformRandomVariable> speed = CreateObject<UniformRandomVariable> ();
   speed->SetAttribute ("Min", DoubleValue (min));
   speed->SetAttribute ("Max", DoubleValue (max));

  // Create static grid
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (1.0),
                                 "MinY", DoubleValue (1.0),
                                 "DeltaX", DoubleValue (step),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (numNodes),
                                 "LayoutType", StringValue ("RowFirst"));
  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  /*
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
  													"Bounds", RectangleValue (Rectangle (0, 1000, 0, 1000))
  													,"Speed",UniformRandomVariable());
	*/

  mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
                                "Bounds", RectangleValue (Rectangle (0, 500, 0, 500)),
                                "Speed", StringValue ("ns3::UniformRandomVariable[Min=5.0|Max=10.0]"),
                                "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));
  mobility.Install (nodes);

  /////////////////////////////MAC//////////////////////////////////////

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



//  YansWifiChannelHelper wifiChannel;
//  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //Propogration loss model is Friis, maximum distance a packet can travel is 250m, -1000 dB past that
//  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel","MinDistance",DoubleValue (250));
  wifiPhy.SetChannel (wifiChannel.Create ());


  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

  devices = wifi.Install (wifiPhy, wifiMac, nodes);



  if (pcap)
  {
  	wifiPhy.EnablePcapAll (std::string ("mcast"));
  }

  ///////////////////////// NETWORK AND MCAST ////////////////////////////

  McastHelper2 mcast;
  //Configure mcast attirbutes if needed
  mcast.Set("HelloBroadcast",BooleanValue(SendHello));

  InternetStackHelper stack;
  stack.SetRoutingHelper (mcast); // has effect on the next Install ()
  stack.Install (nodes);

  Ipv6AddressHelper address;
  address.SetBase(Ipv6Address("2001::"), Ipv6Prefix(64));
  interfaces = address.Assign(devices);


  //Create applications

  MobiCastAppHelper mc;

  mc.SetInterval(retryInterval);
  mc.SetSafetyInterval(successInterval);
  mc.SetEventProbability(eventProbability);

  ApplicationContainer apps = mc.Install(nodes);
  apps.Start(Seconds(5));
  apps.Stop(Seconds(totalTime - 5));

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}


