/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 */
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
#include "udp-echo-client.h"

#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpEchoClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpEchoClient);

TypeId
UdpEchoClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpEchoClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpEchoClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpEchoClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&UdpEchoClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpEchoClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpEchoClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpEchoClient::SetDataSize,
                                         &UdpEchoClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpEchoClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

UdpEchoClient::UdpEchoClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
}

UdpEchoClient::~UdpEchoClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
UdpEchoClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
UdpEchoClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void 
UdpEchoClient::SetRemote (Ipv6Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void
UdpEchoClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
UdpEchoClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

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
          m_socket->Bind6();
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&UdpEchoClient::HandleRead, this));

  ScheduleTransmit (Seconds (0.));
}

void 
UdpEchoClient::StopApplication ()
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
UdpEchoClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so 
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
UdpEchoClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
UdpEchoClient::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
UdpEchoClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
UdpEchoClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
UdpEchoClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &UdpEchoClient::Send, this);
}

void 
UdpEchoClient::Send (void)
{
	NS_LOG_FUNCTION (this);

	NS_ASSERT (m_sendEvent.IsExpired ());

	Ptr<Packet> p;
	if (m_dataSize)
	{
		//
		// If m_dataSize is non-zero, we have a data buffer of the same size that we
		// are expected to copy and send.  This state of affairs is created if one of
		// the Fill functions is called.  In this case, m_size must have been set
		// to agree with m_dataSize
		//
		NS_ASSERT_MSG (m_dataSize == m_size, "UdpEchoClient::Send(): m_size and m_dataSize inconsistent");
		NS_ASSERT_MSG (m_data, "UdpEchoClient::Send(): m_dataSize but no m_data");

		////////////////////////////////////////////////////////////

		//Add timestamp
//		Time currentTime = Simulator::Now();

//		int64_t currentNS = currentTime.GetNanoSeconds();
/*
	  WriteU8 ((data >> 0) & 0xff);
	  WriteU8 ((data >> 8) & 0xff);
	  WriteU8 ((data >> 16) & 0xff);
	  WriteU8 ((data >> 24) & 0xff);
	  WriteU8 ((data >> 32) & 0xff);
	  WriteU8 ((data >> 40) & 0xff);
	  WriteU8 ((data >> 48) & 0xff);
	  WriteU8 ((data >> 56) & 0xff);
*/



//		uint8_t * newData;
//		newData = new uint8_t [m_dataSize];


		/*
		uint8_t * newdata;
		newdata = new uint8_t [m_dataSize];

		std::string str;
		std::ostringstream o;
		o << currentNS;
		str += o.str();


		memcpy (&newdata, str.c_str(), 64);

		uint32_t newSize = m_dataSize - 64;

		memcpy (&newdata[64] , m_data, newSize);
		*/
		////////////////////////////////////////////////////////////

//		std::cout << "Created packet with size: " << m_dataSize << std::endl;

		p = Create<Packet> (m_data, m_dataSize);
	}
  else
    {
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //

		//Add timestamp
		Time currentTime = Simulator::Now();

		int64_t currentNS = currentTime.GetNanoSeconds();

		/*
	  uint8_t * timeData = new uint8_t[8];

		for(int i = 0; i < 8; i++)
		{
			timeData[i] = currentNS >> 8*i;
		}
		*/


		uint8_t * newdata;
		newdata = new uint8_t [m_size];

		std::string str;
		std::ostringstream o;
		o << currentNS;
		str += o.str();


//		std::cout << "String length " << str.length() << std::endl;

		strLength = str.length();

		memcpy (newdata, str.c_str(), str.length());
		uint32_t newSize = m_size - str.length();


		uint8_t *data = new uint8_t[1024];

		memcpy (&newdata[str.length()] , data, newSize);

		//std::cout << "(2) Created packet with size: " << m_dataSize << std::endl;
		//std::cout << "(2) Packet size: " << m_size << std::endl;
		p = Create<Packet> (newdata,m_size);

		Transmission t;
		t.Destination = Ipv4Address::ConvertFrom(m_peerAddress);
		t.SendTime = currentTime;
		m_sourcedTrans.push_back(t);

    }




//	std::cout << "Sending transmission at time: " << t.SendTime << std::endl;


  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);

  std::cout << std::endl;
  std::cout << "UDP ECHO CLIENT sending packet to: " << Ipv4Address::ConvertFrom(m_peerAddress) << " At time: " <<
  		Simulator::Now() << std::endl;
  std::cout << std::endl;

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
      ScheduleTransmit (m_interval);
    }
}

void
UdpEchoClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
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

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

    	uint8_t * Data;
    	Data = new uint8_t [strLength];

      packet-> CopyData(Data,strLength);

      std::string s( reinterpret_cast< char const* >(Data) ) ;


//      std::cout << "String s: " << s.c_str() << std::endl;

      std::string final;
      for(int i = 0; i < strLength ; i++) {
          //if(isalpha(s[i]) || isdigit(s[i])) final += s[i];
      	final += s[i];
      }

//      std::cout << "Final string: " << final << std::endl;

      s = final;

      //std::string myString = "45";
      //int value = atoi(myString.c_str()); //value = 45

      //build up value by combining bytes
      uint64_t NSValue = atoll(final.c_str());


      //for(int i = 0; i < 10; i++)
      //{
      	//NSValue += Data[i] << (int)pow(2,7-i);
      //	NSValue += Data[i] << (int)pow(2,i);
      //}


      Time timestamp = Time::FromInteger(NSValue, Time::NS);

      std::cout << "Received packet with contained timestamp: " << timestamp << " Current time: " << Simulator::Now() << std::endl;
      std::cout << "NSValue calculated: " << NSValue << std::endl;

      for(TransmissionsIt it = m_sourcedTrans.begin(); it != m_sourcedTrans.end(); it++)
      {
      	if(it -> SendTime == timestamp)
      	{
      		std::cout << "Match found" << std::endl;
      		std::cout << "DS timestamp: " << it -> SendTime << std::endl;
      		std::cout << "Packet timestamp: " << timestamp << std::endl;
      		std::cout << std::endl;
      		m_numReceived++;

      		m_RTT = m_RTT + (Simulator::Now() - timestamp);

      		m_sourcedTrans.erase(it);
      		break;
      	}else
      	{
      		std::cout << "No match found" << std::endl;
      		std::cout << "DS timestamp: " << it -> SendTime << std::endl;
      		std::cout << "Packet timestamp: " << timestamp << std::endl;
      		//std::cout << "Time equal?" << (bool)(it -> SendTime == timestamp) << std::endl;

      		std::cout << "Int sendtime" << it->SendTime.GetInteger() << std::endl;
      		std::cout << "Int rec" << timestamp.GetInteger() << std::endl;

      		if(it->SendTime.GetInteger() == timestamp.GetInteger())
      		{
      			std::cout << "Int equal" << std::endl;
      		}else
      		{
      			std::cout << "Int not equal" << std::endl;
      		}

      		std::cout << std::endl;
      	}
      }

    }
}

} // Namespace ns3
