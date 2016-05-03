/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
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
 * Based on 
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 * 
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */
#include "mcast-packet.h"

#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "vector-utils.h"

namespace ns3
{
namespace mcast
{

//-----------------------------------------------------------------------------
// TypeHeader
//-----------------------------------------------------------------------------

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

//Define packet sizes
const uint32_t HelloHeader::m_headerSize = 99;
const uint32_t ControlHeader::m_headerSize = 88;


TypeHeader::TypeHeader (MessageType t) :
  		m_type (t), m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::mcast::TypeHeader")
    		.SetParent<Header> ()
    		.SetGroupName("Mcast")
    		.AddConstructor<TypeHeader> ()
    		;
	return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
	return 1;
}

void
TypeHeader::Serialize (Buffer::Iterator i) const
{
	//std::cout << "Serializing packet with type: " << m_type << std::endl;


	i.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	uint8_t type = i.ReadU8 ();
	m_valid = true;

	switch (type)
	{
	case HELLO:
	{
		m_type = (MessageType) type;
		break;
	}
	case MCAST_CONTROL:
	{
		m_type = (MessageType) type;
		break;
	}
	case INTERNET:
	{
		m_type = (MessageType) type;
		break;
	}case INTERNET_RSU_TO_VANET:
	{
		m_type = (MessageType) type;
		break;
	}case INTERNET_RSU_ACK:
	{
		m_type = (MessageType) type;
		break;
	}case INTERNET_VANET_ACK:
	{
		m_type = (MessageType) type;
		break;
	}case INTERNET_RSU_TO_RSU_REDIRECT:
	{
		m_type = (MessageType) type;
		break;
	}case GEOQUERY_REQUEST:
	{
		m_type = (MessageType) type;
		break;
	}case GEOQUERY_REPLY:
	{
		m_type = (MessageType) type;
		break;
	}case GEOQUERY_RSU_ACK:
	{
		m_type = (MessageType) type;
		break;
	}case GEOREPLY_RSU_ACK:
	{
		m_type = (MessageType) type;
		break;
	}case GEOREPLY_VANET_ACK:
	{
		m_type = (MessageType) type;
		break;
	}case GEOQUERY_SENDINGREPLY:
	{
		m_type = (MessageType) type;
		break;
	}

	default:
		m_valid = false;
		m_type = UNKNOWN;
	}

//	std::cout << "Deserialized type header with type: " << m_type << std::endl;


	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
TypeHeader::Print (std::ostream &os) const
{
	switch (m_type)
	{
	case HELLO:
	{
		os << "HELLO";
		break;
	}
	case MCAST_CONTROL:
	{
		os << "MCAST_CONTROL";
		break;
	}
	case INTERNET:
	{
		os << "INTERNET - TEST";
		break;
	}
	case INTERNET_RSU_TO_VANET:
	{
		os << "INTERNET_RSU_TO_VANET";
		break;
	}
	case INTERNET_RSU_ACK:
	{
		os << "INTERNET_RSU_ACK";
		break;
	}
	case INTERNET_VANET_ACK:
	{
		os << "INTERNET_VANET_ACK";
		break;
	}
	case INTERNET_RSU_TO_RSU_REDIRECT:
	{
		os << "RSU_TO_RSU_REDIRECT" << std::endl;
		break;
	}case GEOQUERY_REQUEST:
	{
		os << "GEOQUERY REQUEST" << std::endl;
		break;
	}case GEOQUERY_REPLY:
	{
		os << "GEOQUERY REPLY" << std::endl;;
		break;
	}case GEOQUERY_RSU_ACK:
	{
		os << "Qeo Query ACK" << std::endl;
		break;
	}case GEOREPLY_RSU_ACK:
	{
		os << "GeoReply RSU ACK" << std::endl;
		break;
	}case GEOREPLY_VANET_ACK:
	{
		os << "GeoReply VANET ACK" << std::endl;
		break;
	}case GEOQUERY_SENDINGREPLY:
	{
		os  << "GeoQuery Sending Reply" << std::endl;
		break;
	}

	default:
		os << "UNKNOWN_TYPE";
		break;
	}
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
	return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
	h.Print (os);
	return os;
}

//-----------------------------------------------------------------------------
// HELLO
//-----------------------------------------------------------------------------

HelloHeader::HelloHeader(uint8_t type, uint64_t roadId, uint8_t hopCount, uint16_t neighborLifeTime,
		uint16_t mCastRadius,uint16_t reserved, Ipv6Address dst, Ipv6Address origin,
		Vector position,Vector velocity) :
				m_type(type), m_roadId(roadId), m_hopCount(hopCount), m_neighborLifeTime(neighborLifeTime),
				m_mCastRadius(mCastRadius),m_reserved(reserved), m_dst(dst), m_origin(origin), m_position(position), m_velocity(velocity)
{

}

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::mcast::HelloHeader")
    		.SetParent<Header> ()
    		.SetGroupName("mcast")
    		.AddConstructor<HelloHeader> ()
    		;
	return tid;
}

TypeId
HelloHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t
HelloHeader::GetSerializedSize () const
{
	return m_headerSize;
}

//Serialize header (convert vector double to int64 by multiplying by 1000 -
void
HelloHeader::Serialize (Buffer::Iterator i) const
{
	i.WriteU8(m_type);
	i.WriteU64(m_roadId);
	i.WriteU8(m_hopCount);
	i.WriteU16(m_neighborLifeTime);
	i.WriteU16(m_mCastRadius);
	i.WriteU16(m_reserved);
	WriteTo(i,m_dst);
	WriteTo(i,m_origin);

	i.WriteHtolsbU64((uint64_t) abs (m_position.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs ( m_position.y * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_position.z * 1000));

  i.WriteHtonU64  ((uint64_t)abs(m_velocity.x*1000));
  i.WriteHtonU64  ((uint64_t)abs(m_velocity.y*1000));
  i.WriteHtonU64  ((uint64_t)abs(m_velocity.z*1000));


  if (m_velocity.x >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  if (m_velocity.y >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  if (m_velocity.z >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

}

uint32_t
HelloHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_type = i.ReadU8 ();
	m_roadId = i.ReadU64 ();
	m_hopCount = i.ReadU8 ();
	m_neighborLifeTime = i.ReadU16 ();
	m_mCastRadius = i.ReadU16 ();
	m_reserved = i.ReadU16();
	ReadFrom (i, m_dst);
	ReadFrom (i, m_origin);

	m_position.x =(double) (i.ReadNtohU64 ()/1000.0);
	m_position.y =(double) (i.ReadNtohU64 ()/1000.0);
	m_position.z =(double) (i.ReadNtohU64 ()/1000.0);

  m_velocity.x =(double) (i.ReadNtohU64 ()/1000.0);
	m_velocity.y =(double) (i.ReadNtohU64 ()/1000.0);
	m_velocity.z =(double) (i.ReadNtohU64 ()/1000.0);

	uint8_t tmpsign;
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
	  m_velocity.x = - m_velocity.x;

  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_velocity.y = - m_velocity.y;

  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_velocity.z = - m_velocity.z;


	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
HelloHeader::Print (std::ostream &os) const
{
	os << "Hello Packet received; Origin: " << m_origin << " Current Address: " << m_dst << std::endl;
}

std::ostream &
operator<< (std::ostream & os, HelloHeader const & h)
{
	h.Print (os);
	return os;
}

bool
HelloHeader::operator== (HelloHeader const & o) const
{
	return (m_type == o.m_type && m_reserved == o.m_reserved &&
			m_hopCount == o.m_hopCount &&
			m_origin == o.m_origin && m_origin == o.m_origin);

}

//-----------------------------------------------------------------------------
// MCAST_CONTROL
//-----------------------------------------------------------------------------

NS_OBJECT_ENSURE_REGISTERED (ControlHeader);

/*
ControlHeader::ControlHeader(Ipv6Address Id, Ipv6Address Source, uint32_t a, uint32_t b, uint64_t xpl, uint64_t ypl, uint64_t xpr, uint64_t ypr) :
				m_Id(Id), m_Source(Source), m_a(a), m_b(b), m_xPl(xpl), m_yPl(ypl),m_xPr(xpr),m_yPr(ypr)
{

}
*/

ControlHeader::ControlHeader(Ipv6Address Id, Ipv6Address Source, uint32_t a, uint32_t b, Vector apxL, Vector apexR, Vector center) :
				m_Id(Id), m_Source(Source), m_a(a), m_b(b), m_Apxl(apxL), m_Apxr(apexR), m_center(center)
{

}

ControlHeader::~ControlHeader()
{
}

TypeId
ControlHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::mcast::ControlHeader")
    		.SetParent<Header> ()
    		.SetGroupName("mcast")
    		.AddConstructor<ControlHeader> ()
    		;
	return tid;
}

TypeId
ControlHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t
ControlHeader::GetSerializedSize () const
{
	return m_headerSize;
}

//Serialize header (convert vector double to int64 by multiplying by 1000 -
void
ControlHeader::Serialize (Buffer::Iterator i) const
{

	WriteTo(i,m_Id);
	WriteTo(i,m_Source);
	i.WriteU32(m_a);
	i.WriteU32(m_b);

	i.WriteHtolsbU64((uint64_t) abs (m_Apxl.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_Apxl.y * 1000));

	i.WriteHtolsbU64((uint64_t) abs (m_Apxr.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_Apxr.y * 1000));

	i.WriteHtolsbU64((uint64_t) abs (m_center.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_center.y * 1000));

	/*
	i.WriteU64(m_xPl);
	i.WriteU64(m_yPl);
	i.WriteU64(m_xPr);
	i.WriteU64(m_yPl);
	*/
}

uint32_t
ControlHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;

	ReadFrom (i, m_Id);
	ReadFrom (i, m_Source);

	m_a = i.ReadU32();
	m_b = i.ReadU32();

	m_Apxl.x =(double) (i.ReadNtohU64 ()/1000.0);
	m_Apxl.y =(double) (i.ReadNtohU64 ()/1000.0);

	m_Apxr.x =(double) (i.ReadNtohU64 ()/1000.0);
	m_Apxr.y =(double) (i.ReadNtohU64 ()/1000.0);

	m_center.x =(double) (i.ReadNtohU64 ()/1000.0);
	m_center.y =(double) (i.ReadNtohU64 ()/1000.0);

	/*
	m_xPl = i.ReadU64();
	m_yPl = i.ReadU64();

	m_xPr = i.ReadU64();
	m_yPr = i.ReadU64();
	*/

	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
ControlHeader::Print (std::ostream &os) const
{
	os << "MCAST Control Packet received" << std::endl;
}

std::ostream &
operator<< (std::ostream & os, ControlHeader const & h)
{
	h.Print (os);
	return os;
}

bool
ControlHeader::operator== (ControlHeader const & o) const
{
	return (m_Id == o.m_Id && m_Apxl.x == o.m_Apxl.x && m_Apxl.y == o.m_Apxl.y);
}

}
}
