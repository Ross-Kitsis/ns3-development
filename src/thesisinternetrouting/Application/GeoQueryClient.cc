/*
 * GeoQueryClient.cc
 *
 *  Created on: Apr 30, 2016
 *      Author: ross
 */

#include "../../thesisinternetrouting/Application/GeoQueryClient.h"

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ipv6.h"

#include "ns3/thesisinternetrouting2.h"
#include "ns3/Db.h"

#define GEOQUERY "ff02::118"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("GeoQueryClientApplication");

NS_OBJECT_ENSURE_REGISTERED (GeoQueryClient);

GeoQueryClient::GeoQueryClient()
{
	// TODO Auto-generated constructor stub
	NS_LOG_FUNCTION (this);
	m_sent = 0;
	m_socket = 0;
	m_sendEvent = EventId ();
	m_peerAddress = Ipv6Address(GEOQUERY);
}

GeoQueryClient::~GeoQueryClient()
{
	// TODO Auto-generated destructor stub
	m_sendEvent.Cancel();
}

void
GeoQueryClient::SetRsuDatabase(Ptr<Db> db)
{
	m_db = db;
}

TypeId
GeoQueryClient::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::GeoQueryClient")
    						.SetParent<Application> ()
    						.SetGroupName("Applications")
    						.AddConstructor<GeoQueryClient> ()
    						.AddAttribute ("MaxPackets",
    								"The maximum number of packets the application will send",
    								UintegerValue (100),
    								MakeUintegerAccessor (&GeoQueryClient::m_count),
    								MakeUintegerChecker<uint32_t> ())
    								.AddAttribute ("Interval",
    										"The time to wait between packets",
    										TimeValue (Seconds (1.0)),
    										MakeTimeAccessor (&GeoQueryClient::m_interval),
    										MakeTimeChecker ())
    										.AddAttribute ("RemoteAddress",
    												"The destination Address of the outbound packets",
    												AddressValue (),
    												MakeAddressAccessor (&GeoQueryClient::m_peerAddress),
    												MakeAddressChecker ())
    												.AddAttribute ("RemotePort",
    														"The destination port of the outbound packets",
    														UintegerValue (0),
    														MakeUintegerAccessor (&GeoQueryClient::m_peerPort),
    														MakeUintegerChecker<uint16_t> ())
    														;
	return tid;
}

void
GeoQueryClient::DoDispose (void)
{
	NS_LOG_FUNCTION (this);
	m_sendEvent.Cancel();
	Application::DoDispose ();
}

void GeoQueryClient::StartApplication (void)
{
	NS_LOG_FUNCTION (this);

	std::srand(std::time(0));
	if (m_socket == 0)
	{
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		m_socket = Socket::CreateSocket (GetNode (), tid);
		if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
		{
			m_socket->Bind();
			m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
		}
		else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
		{
			//m_socket->Bind6();
			m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
		}
	}
	m_socket->SetRecvCallback (MakeCallback (&GeoQueryClient::HandleRead, this));


	ScheduleTransmit (Seconds (0.));
}

void GeoQueryClient::StopApplication (void)
{
	NS_LOG_FUNCTION (this);

	if (m_socket != 0)
	{
		m_socket->Close ();
		m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
		m_socket = 0;
	}

	Simulator::Cancel (m_sendEvent);
}

void
GeoQueryClient::ScheduleTransmit (Time dt)
{
	NS_LOG_FUNCTION (this << dt);

	Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();

	//Perform address checking here, re-bind to the correct address
	for(uint32_t i = 0; i < ipv6 ->GetNInterfaces();i++)
	{
		for(uint32_t j = 0; j < ipv6 ->GetNAddresses(i); j++)
		{
			Ipv6Address srcAdd = ipv6 ->GetAddress(i,j).GetAddress();
			if(!srcAdd.IsLinkLocal())
			{
				src = srcAdd;
				break;
			}
		}
	}

	//Bind to socket
	m_socket->Bind (Inet6SocketAddress (src, 50001));
	//m_socket->

	//Set Recv callback again in case re-binding caused problems
	m_socket->SetRecvCallback (MakeCallback (&GeoQueryClient::HandleRead, this));

	//std::cout << " ****** New address bound and callbacks set *******" << std::endl;

	m_sendEvent = Simulator::Schedule (dt, &GeoQueryClient::Send, this);

	//Set parameters to query in zone
	Vector toQuery;
	Ptr<Node> theNode = GetObject<Node>();
	Ptr<Ipv6> v6 = theNode -> GetObject<Ipv6>();
	Ptr<thesis::ThesisInternetRoutingProtocol2> routing = DynamicCast<thesis::ThesisInternetRoutingProtocol2>(v6 -> GetRoutingProtocol());

	Ipv6Address rsu = routing -> m_RsuDestination;
	bool locFound = false;


	int size = m_db -> GetNumEntry();
	DbEntry entry;
	while(!locFound)
	{
		int output = 0 + (rand() % (int)(size - 0 + 1));
		entry = m_db -> GetEntry(output);
		if(entry.GetRsuAddress() != rsu)
		{
			locFound = true;
			toQuery.x = entry.GetRsuPosition().x + 100;
			toQuery.y = entry.GetRsuPosition().y + 100;
		}

	}

	routing -> SetGeoQueryPosition(toQuery);

	Ipv6Address network = entry.GetRsuAddress().CombinePrefix(Ipv6Prefix(64));

	uint8_t currentBytes[16];
	uint8_t netBytes[16];
	uint8_t destBytes[16];

	for(int i = 0; i < 16; i ++)
	{
		if(i < 8)
		{
			destBytes[i] = netBytes[i];
		}else
		{
			destBytes[i] = currentBytes[i];
		}
	}

	m_peerAddress = Ipv6Address(destBytes);

	m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));


//	Ipv6Address peer()

}
/**
 * \brief Send a packet
 */
void
GeoQueryClient::Send (void)
{
	NS_LOG_FUNCTION (this);

	NS_ASSERT (m_sendEvent.IsExpired ());
	Ptr<Packet> p;
	// call to the trace sinks before the packet is actually sent,
	// so that tags added to the packet can be sent as well

	p = Create<Packet> (m_size);
	m_txTrace (p);
	m_socket->Send (p);



	++m_sent;

	if (Ipv4Address::IsMatchingType (m_peerAddress))
	{
		NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
				Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
	}
	else if (Ipv6Address::IsMatchingType (m_peerAddress))
	{
		NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
				Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
	}

	if (m_sent < m_count)
	{
		ScheduleTransmit (m_interval + MilliSeconds(std::rand()%1000));
	}else
	{
		std::cout << src << " Finished sending, not further events" << std::endl;
		if(m_sendEvent.IsRunning())
		{
			m_sendEvent.Cancel();
		}
	}
}

/**
 * \brief Handle a packet reception.
 *
 * This function is called by lower layers.
 *
 * \param socket the socket the packet was received to.
 */
void
GeoQueryClient::HandleRead (Ptr<Socket> socket)
{
	NS_LOG_FUNCTION (this << socket);

	//std::cout << std::endl;
	//std::cout << std::endl;
	//std::cout << "UDP ECHO CLIENT RECEIEVED PACKET BACK" << std:: endl;


	Ptr<Packet> packet;
	Address from;
	while ((packet = socket->RecvFrom (from)))
	{
		if (InetSocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
					InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
					InetSocketAddress::ConvertFrom (from).GetPort ());
		}
		else if (Inet6SocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
					Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
					Inet6SocketAddress::ConvertFrom (from).GetPort ());
		}
	}
}

} /* namespace ns3 */
