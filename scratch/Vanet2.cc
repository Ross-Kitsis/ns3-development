/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/core-module.h"

#include "ns3/wave-mac-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-80211p-helper.h"

#include "ns3/mobility-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/ns2-mobility-helper.h"

#include "ns3/aodv-module.h"
#include "ns3/ipv4-list-routing-helper.h"

#include "ns3/rectangle.h"

#include "ns3/clwpr-map.h"
#include "ns3/generic-map.h"

using namespace ns3;
using namespace std;

void CourseChange(string context, Ptr<const MobilityModel> model)
{

	Vector position = model->GetPosition();
	Vector velocity = model ->GetVelocity();
	NS_LOG_UNCOND(context << " x = " << position.x << " , y = " << position.y);
	NS_LOG_UNCOND(context << "Velocity x = " << velocity.x << " , Velocity y = " << position.y);
	//std::cout << context << " x = " << position.x << " , y = " << position.y;
}

void PingRtt(std::string context, Time rtt)
{
	std::cout << context << " " << rtt << std::endl;
}


NS_LOG_COMPONENT_DEFINE("Vanet2");
int
main(int argc, char *argv[])
{
	uint32_t numNodes = 2; //Number of nodes (Minimum 2)
	bool verbose =  false;
	std::string phyMode("OfdmRate6MbpsBW10MHz");
	int mobilityType = 0;
	double endTime = 15.0;

	//Trace properties
	std::string traceFile;
	std::string logFile = "vanet2-ns2-mob.log";


	CommandLine cmd;
	cmd.AddValue("numNodes", "Number of nodes in the simulation", numNodes);
	cmd.AddValue("verbose", "Set verbose to print all logs to console", verbose);
	cmd.AddValue("mobility", "Type of mobility (0=Static, 1 = RWP, 2=trace - Must specify location)", mobilityType);
	cmd.AddValue("endTime", "Time to end simulation", endTime);
	cmd.AddValue("traceFile", "Location of trace file", traceFile);

	cmd.Parse(argc,argv);

	NodeContainer vanetNodes;
	vanetNodes.Create(numNodes);


	//Setup Layer 1 channel
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
	Ptr<YansWifiChannel> channel = wifiChannel.Create ();
	wifiPhy.SetChannel (channel);

	//Setup Pcap Tracing
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

	//Setup layer 2
	NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
	Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
	if (verbose)
	{
		wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
	}

	wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
			"DataMode",StringValue (phyMode),
			"ControlMode",StringValue (phyMode));
	NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, vanetNodes);

	//Enable PCAP tracing
	wifiPhy.EnablePcap ("Vanet2", devices);


	// Enable mobility
	if(mobilityType == 0)
	{
		//Enable fixed position mobility (no mobility) in a 100x100 rectangle
		MobilityHelper mobility;
		Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
		srand((unsigned)time(0));
		for(uint32_t i = 0; i < numNodes; i++)
		{
			positionAlloc->Add (Vector (rand()%100, rand()%100, 0.0));
		}
		mobility.SetPositionAllocator (positionAlloc);
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (vanetNodes);
	}else if(mobilityType == 1)
	{
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
	}else if(mobilityType == 2)
	{
		//Set trace mobility
	  // Check command line arguments
	  if (traceFile.empty () || numNodes <= 0 || endTime <= 0 || logFile.empty ())
	    {
	      std::cout << "Usage of " << argv[0] << " :\n\n"
	      "NOTE: ns2-traces-file could be an absolute or relative path."
	      "      included in the same directory of this example file.\n\n"
	      "NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
	      "        be a positive number. Note that you must know it before to be able to load it.\n\n"
	      "NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

	      return 0;
	    }
	  	//Ns2 mobility helper - takes tracefile with node movements as input
	  	Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);

	    // open log file for output
	    std::ofstream os;
	    os.open (logFile.c_str ());

	    ns2.Install();
	}


	//Setup Layer 3
	InternetStackHelper internet;
	internet.Install (vanetNodes);

	//Setup IP Addressing
	Ipv4AddressHelper ipv4;
	NS_LOG_INFO ("Assign IP Addresses.");
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer IpIC = ipv4.Assign (devices);

	//Setup routing
	AodvHelper aodv;
	Ipv4ListRoutingHelper list;
	list.Add(aodv,100);
	internet.SetRoutingHelper(list);


	//Setup application containers
	ApplicationContainer serverApps;
	NodeContainer pingSource;

	//Set ping target
	V4PingHelper ping = V4PingHelper(IpIC.GetAddress(0));

	//Set ping sources
	for(uint32_t i = 1; i < numNodes; i++)
	{
		pingSource.Add(vanetNodes.Get(i));
	}

	serverApps = ping.Install(pingSource);
	serverApps.Stop(Seconds(endTime));
	serverApps.Start(Seconds (2.0));


	std::ostringstream oss;
	/*
	oss << "/NodeList/" <<vanetNodes.Get(numNodes-1)->GetId() <<
			"/$ns3::MobilityModel/CourseChange";
	*/
	oss << "/NodeList/*/$ns3::MobilityModel/CourseChange";

	Config::Connect(oss.str(), MakeCallback(&CourseChange));


	//Print ping rtt
	Config::Connect("/NodeList/*/ApplicationList/*/$ns3::V4Ping/Rtt",
			MakeCallback(&PingRtt));

	Simulator::Stop(Seconds(endTime));
	Simulator::Run ();
	Simulator::Destroy ();
}
