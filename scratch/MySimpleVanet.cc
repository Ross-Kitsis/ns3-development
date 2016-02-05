/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 * Copyright (c) 2013 Dalian University of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Junling Bu <linlinjavaer@gmail.com>
 *
 */
/**
 * This example shows basic construction of an 802.11p node.  Two nodes
 * are constructed with 802.11p devices, and by default, one node sends a single
 * packet to another node (the number of packets and interval between
 * them can be configured by command-line arguments).  The example shows
 * typical usage of the helper classes for this mode of WiFi (where "OCB" refers
 * to "Outside the Context of a BSS")."
 */

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include <iostream>

#include "ns3/v4ping-helper.h"

#include "ns3/rectangle.h"

#include "ns3/aodv-module.h"
#include "ns3/ipv4-list-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MySimpleVanet");

static void PingRtt(std::string context, Time rtt)
{
	std::cout << context << " " << rtt << std::endl;
}

int
main(int argc, char *argv[])
{

	//Define parameters and default values
	uint32_t numNodes = 2;
	uint32_t packetSize = 1000;
	uint32_t numPackets = 10;
	double interval = 1.0; //Seconds
	bool verbose = false;
	std::string phyMode("OfdmRate6MbpsBW10MHz");

	//Define command line arguments (May be expanded level)
	CommandLine cmd;
	cmd.AddValue("numNodes", "Number of nodes in the simulation", numNodes);
	cmd.AddValue("verbose", "Set verbose to print all logs to console", verbose);
	cmd.AddValue("numPackets", "Number of packets to send", numPackets);
	cmd.AddValue("packetSize", "Size of packet to send", packetSize);
	cmd.AddValue("interval", "Interval between packets", interval);

	//Parse command line arguments
	cmd.Parse(argc, argv);

	//Create container for nodes and create nodes
	NodeContainer vanetNodes;
	vanetNodes.Create(numNodes);

	//Add and initialize helps to create wifi NICs

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
	Ptr<YansWifiChannel> channel = wifiChannel.Create();
	wifiPhy.SetChannel(channel);

	//Set pcap tracing
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

	//Data-link layer
	NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();

	//802.11p Helper
	Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
	if (verbose)
	{
		wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
	}

	wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
			"DataMode",StringValue (phyMode),
			"ControlMode",StringValue (phyMode));

	NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, vanetNodes);

	//Enable tracing on all devices
	wifiPhy.EnablePcap ("wave-simple-80211p", devices);



	//Mobility helpers
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (vanetNodes);

  //Network layer
  InternetStackHelper INetStack;

  AodvHelper aodv;
  Ipv4ListRoutingHelper list;

  list.Add(aodv,100);

  INetStack.SetRoutingHelper(list);
  INetStack.Install(vanetNodes);

  //IP Address helpers
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipic = ipv4.Assign (devices);

  /*
  //Install echo server(s)
  ApplicationContainer serverApps;
  for(uint32_t i = 0; i < numNodes/2; i=i+2)
  {
  	UdpEchoServerHelper echoServer(90);
  	serverApps = echoServer.Install(vanetNodes.Get(i));
  	serverApps.Start(Seconds(1 + ((double)(i))/10 ));
  	serverApps.Stop(Seconds(10 + ((double)(i))/10));
  }

  //Initialize echo clients
  for(uint32_t i= 1; i < numNodes/2; i = i+2)
  {
  	UdpEchoClientHelper echoClient();
  }
	*/


  /*

  //Instantiate and install ping server(s)
  ApplicationContainer serverApps;
  V4PingHelper ping = V4PingHelper()
  for(uint32_t i = 0; i < numNodes/2; i = i+2)
  {
  	ping.V4PingHelper(ipic.Get(i));
  }

  //Instantiate and install ping clients
  NodeContainer pingSource;
  for(uint32_t i = 1; i < numNodes/2; i=i+2)
  {
  	pingSource.Add(vanetNodes.Get(i));
  }
  serverApps = ping.Install(pingSource);
  serverApps.Start(Seconds(2.0));
  serverApps.Start(Seconds(10.0));

	*/



  ApplicationContainer serverApps;
  NodeContainer pingSource;

  V4PingHelper ping = V4PingHelper(ipic.GetAddress(0));




  for(uint32_t i = 1; i < numNodes; i++)
  {
  	pingSource.Add(vanetNodes.Get(i));
  }

  serverApps = ping.Install(pingSource);
  serverApps.Stop(Seconds(10.0));
  serverApps.Start(Seconds (2.0));




  //Print ping rtt
  Config::Connect("/NodeList/*/ApplicationList/*/$ns3::V4Ping/Rtt",
  		MakeCallback(&PingRtt));

 // Packet::EnablePrinting();

  NS_LOG_INFO("Running simulation");
  Simulator::Stop(Seconds(15.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Simulation finished");

}

