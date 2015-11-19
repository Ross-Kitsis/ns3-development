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
 *			Based on AODV packet implementation as provided in NS-3 source code
 *
 * 			Authors: Ross Kitsis
 */
#ifndef AODVPACKET_H
#define AODVPACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv6-address.h"
#include <map>
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {
namespace mcast {

enum MessageType
{
  HELLO  = 1,   //!< MCAST Hello type
};

/**
* \ingroup aodv
* \brief MCAST types
*/
class TypeHeader : public Header
{
public:
  /// c-tor
  TypeHeader (MessageType t = HELLO);

  // Header serialization/deserialization
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  /// Return type
  MessageType Get () const { return m_type; }
  /// Check that type if valid
  bool IsValid () const { return m_valid; }
  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type;
  bool m_valid;
};

std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

/**
* \ingroup aodv
* \brief   Route Request (RREQ) Message Format
  \verbatim
  0                   1                   2                   3										4										5										6										7										8									  9									  10                  11                  12
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |																										Road ID																																			|    Hop Count  |        Neighbor Lifetime      |       Mobicast Radius         |          Reserved            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Destination IP Address                                                                                                                                                                                                                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP Address                                                                                                                                                                                                                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Position                                                                                                                                                                                                                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Velocity                                                                                                                                                                                                                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

0-7 - Type
8-71 - Road ID
72-79 - Hop count
80-95 - Lifetime
96-111 - Mobicast radius
112-127 - Reserved

0-127 Destination IP
0-127 Source IP
? - Source position
? - Source velocity



  \endverbatim
*/
class HelloHeader : public Header
{
public:
  /// c-tor
  HelloHeader (uint8_t type = 0, uint64_t roadId = 0, uint8_t hopCount = 0,
              uint16_t neighborLifeTime = 0, uint16_t mCastRadius = 0,
              uint16_t reserved = 0,
              Ipv6Address dst = Ipv6Address (),
              Ipv6Address origin = Ipv6Address (),
              Vector position = Vector(),
              Vector veloity = Vector());

  // Header serialization/deserialization
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  void SetHopCount (uint8_t count) { m_hopCount = count; }
  uint8_t GetHopCount () const { return m_hopCount; }

  void SetRadius (uint16_t radius) { m_mCastRadius = radius; }
  uint16_t GetMcastRadius () const { return m_mCastRadius; }

  void SetDst (Ipv6Address a) { m_dst = a; }
  Ipv6Address GetDst () const { return m_dst; }

  void SetOrigin (Ipv6Address a) { m_origin = a; }
  Ipv6Address GetOrigin () const { return m_origin; }

  void SetRoadID (uint64_t rId) { m_roadId = rId; }
  uint64_t GetOriginSeqno () const { return m_roadId; }

  void SetPosition(Vector pos) {m_position = pos;}
  Vector getPosition() const {return m_position;}

  void SetVelocity(Vector pos) {m_velocity = pos;}
  Vector getVelocity() const {return m_velocity;}

  bool operator== (HelloHeader const & o) const;
private:
  uint8_t        m_type;          		///< Type of packet
  uint8_t        m_hopCount;       		///< Hop Count
  Ipv6Address    m_dst;            		///< Destination IP Address
  Ipv6Address    m_origin;         		///< Originator IP Address
  uint16_t       m_neighborLifeTime;	///< Neighbor lifetime before expires
  uint16_t 			 m_mCastRadius;				///< mcast radius
  uint64_t 			 m_roadId;						///< Road ID
  Vector				 m_position;					///< Current position of node
  Vector				 m_velocity;					///< Current velocity of node
  uint16_t			 m_reserved; 					///< reserved for future development
};

std::ostream & operator<< (std::ostream & os, HelloHeader const &);

} //MCAST namespace
} //
#endif /* MCASTPACKET_H */
