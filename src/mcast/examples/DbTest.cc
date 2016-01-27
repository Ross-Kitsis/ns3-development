/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/random-variable-stream.h"

#include <iostream>
#include <cmath>
#include <string>
#include <sstream>

#include "ns3/Db.h"

using namespace ns3;


int
main (int argc, char *argv[])
{
  bool verbose = true;
  //Number of nodes to create
  int numNodes = 20;
  //Distance between nodes (Meters)
  int step = 10;
  //total time to run simulation
  int totalTime = 90;
  //Probability to send an mcast packet

  int row = 5;

  uint32_t length = step*step;
  uint32_t width = step;

  /*
  //Time to wait between successful transmissions
  Time SuccessTimer = Seconds(20);
  //Time between unsuccessful events triggered, wait to try again
  Time RetyTimer = Seconds(5);
	*/


  Time retryInterval = Seconds(1);
  Time successInterval = Seconds(10);


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
                                 "MinX", DoubleValue (0),
                                 "MinY", DoubleValue (0),
                                 "DeltaX", DoubleValue (step * step),
                                 "DeltaY", DoubleValue (step),
                                 "GridWidth", UintegerValue (row),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

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



   ///////////////////////// NETWORK AND MCAST ////////////////////////////

   InternetStackHelper stack;
   stack.Install (nodes);

   Ipv6AddressHelper address;

   //uint32_t zone = 0x1;

   //IPv6 Base address, split int 128 bytes for induvidual byte manupilation
   unsigned char baseAd[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

   baseAd[0] = 32;
   baseAd[1] = 1;

   int addStart = 7;

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
   Db testDb;

   testDb.CreateDatabase(nodes,length,width);


   Vector test1(0,0,0);
   Vector test2(110,23,0);
   Vector test3(16,15,0);

   DbEntry t1 = testDb.GetEntryForCurrentPosition(test1);
   DbEntry t2 = testDb.GetEntryForCurrentPosition(test2);
   DbEntry t3 = testDb.GetEntryForCurrentPosition(test3);

   std::cout << "Rsu address of t1: " << t1.GetRsuAddress() <<std::endl;
   std::cout << "Rsu address of t2: " << t2.GetRsuAddress() <<std::endl;
   std::cout << "Rsu address of t2: " << t3.GetRsuAddress() <<std::endl;


  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}


