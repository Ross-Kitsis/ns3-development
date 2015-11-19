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

namespace ns3
{
namespace mcast
{

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

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
												uint16_t mCastRadius, Ipv6Address dst, Ipv6Address origin, Vector position,
												Vector velocity) :
		m_type(type), m_roadId(roadId), m_hopCount(hopCount), m_neighborLifeTime(neighborLifeTime),
		m_mCastRadius(mCastRadius), m_dst(dst), m_origin(origin), m_position(position), m_velocity(velocity)
{

}


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
  return 23;
}

void
HelloHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_type);
  i.WriteU8 (m_reserved);
  i.WriteU8 (m_hopCount);
  i.WriteHtonU32 (m_requestID);
  WriteTo (i, m_dst);
  i.WriteHtonU32 (m_dstSeqNo);
  WriteTo (i, m_origin);
  i.WriteHtonU32 (m_originSeqNo);
}

uint32_t
HelloHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_flags = i.ReadU8 ();
  m_reserved = i.ReadU8 ();
  m_hopCount = i.ReadU8 ();
  m_requestID = i.ReadNtohU32 ();
  ReadFrom (i, m_dst);
  m_dstSeqNo = i.ReadNtohU32 ();
  ReadFrom (i, m_origin);
  m_originSeqNo = i.ReadNtohU32 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
HelloHeader::Print (std::ostream &os) const
{
  os << "RREQ ID " << m_requestID << " destination: ipv4 " << m_dst
     << " sequence number " << m_dstSeqNo << " source: ipv4 "
     << m_origin << " sequence number " << m_originSeqNo
     << " flags:" << " Gratuitous RREP " << (*this).GetGratiousRrep ()
     << " Destination only " << (*this).GetDestinationOnly ()
     << " Unknown sequence number " << (*this).GetUnknownSeqno ();
}

std::ostream &
operator<< (std::ostream & os, HelloHeader const & h)
{
  h.Print (os);
  return os;
}

void
HelloHeader::SetGratiousRrep (bool f)
{
  if (f)
    m_flags |= (1 << 5);
  else
    m_flags &= ~(1 << 5);
}

bool
HelloHeader::GetGratiousRrep () const
{
  return (m_flags & (1 << 5));
}

void
HelloHeader::SetDestinationOnly (bool f)
{
  if (f)
    m_flags |= (1 << 4);
  else
    m_flags &= ~(1 << 4);
}

bool
HelloHeader::GetDestinationOnly () const
{
  return (m_flags & (1 << 4));
}

void
HelloHeader::SetUnknownSeqno (bool f)
{
  if (f)
    m_flags |= (1 << 3);
  else
    m_flags &= ~(1 << 3);
}

bool
HelloHeader::GetUnknownSeqno () const
{
  return (m_flags & (1 << 3));
}

bool
HelloHeader::operator== (RreqHeader const & o) const
{
  return (m_flags == o.m_flags && m_reserved == o.m_reserved &&
          m_hopCount == o.m_hopCount && m_requestID == o.m_requestID &&
          m_dst == o.m_dst && m_dstSeqNo == o.m_dstSeqNo &&
          m_origin == o.m_origin && m_originSeqNo == o.m_originSeqNo);
}

}
}
