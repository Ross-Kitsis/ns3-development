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

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

//Define mcast packet size
const uint32_t HelloHeader::m_headerSize = 99;

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
	default:
		m_valid = false;
	}
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
	default:
		os << "UNKNOWN_TYPE";
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
// RREQ
//-----------------------------------------------------------------------------
/*
HelloHeader::HelloHeader (uint8_t flags, uint8_t reserved, uint8_t hopCount, uint32_t requestID, Ipv4Address dst,
                        uint32_t dstSeqNo, Ipv4Address origin, uint32_t originSeqNo) :
  m_flags (flags), m_reserved (reserved), m_hopCount (hopCount), m_requestID (requestID), m_dst (dst),
  m_dstSeqNo (dstSeqNo), m_origin (origin),  m_originSeqNo (originSeqNo)
{
}
 */


HelloHeader::HelloHeader(uint8_t type, uint64_t roadId, uint8_t hopCount, uint16_t neighborLifeTime,
		uint16_t mCastRadius,uint16_t reserved, Ipv6Address dst, Ipv6Address origin,
		Vector position,Vector velocity) :
				m_type(type), m_roadId(roadId), m_hopCount(hopCount), m_neighborLifeTime(neighborLifeTime),
				m_mCastRadius(mCastRadius),m_reserved(reserved), m_dst(dst), m_origin(origin), m_position(position), m_velocity(velocity)
{

}


/*
HelloHeader::HelloHeader(uint8_t type, uint64_t roadId, uint8_t hopCount, uint16_t neighborLifeTime,
												uint16_t mCastRadius,uint16_t reserved, Ipv6Address dst, Ipv6Address origin,
												float_t xPos,float_t yPos, float_t zPos,
												float_t xVel, float_t yVel, float_t zVel) :
		m_type(type), m_roadId(roadId), m_hopCount(hopCount), m_neighborLifeTime(neighborLifeTime),
		m_mCastRadius(mCastRadius),m_reserved(reserved), m_dst(dst), m_origin(origin), m_xPos(xPos),
		m_yPos(yPos), m_zPos(zPos), m_xVel(xVel), m_yVel(yVel), m_zVel(zVel)
{

}
 */

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::mcast::HelloHeader")
    		.SetParent<Header> ()
    		.SetGroupName("Mcast")
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

	//WriteTo(i, m_position);
	//WriteTo(i, m_velocity);

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

	/*
  i.WriteU8 (m_type);
  i.WriteU8 (m_reserved);
  i.WriteU8 (m_hopCount);
  i.WriteHtonU32 (m_requestID);
  WriteTo (i, m_dst);
  i.WriteHtonU32 (m_dstSeqNo);
  WriteTo (i, m_origin);
  i.WriteHtonU32 (m_originSeqNo);
	 */
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
	//ReadFrom (i, m_position);
	//ReadFrom (i, m_velocity);

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
}
}
