/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */



#include "ns3/mcast-helper2.h"
//#include "ns3/mcast.h"
//#include "ns3/mcast-neighbor.h"
#include "ns3/mcast-packet.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"

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
  int step = 25;
  //total time to run simulation
  int totalTime = 90;

  //Capture packets
  bool pcap = false;

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

  // Create static grid
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (step),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (numNodes),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  /////////////////////////////MAC//////////////////////////////////////

  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
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

  InternetStackHelper stack;
  stack.SetRoutingHelper (mcast); // has effect on the next Install ()
  stack.Install (nodes);

  Ipv6AddressHelper address;
  address.SetBase(Ipv6Address("2001::"), Ipv6Prefix(64));
  interfaces = address.Assign(devices);


  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}


