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
  MCAST_CONTROL = 2, //!< MCAST Control type
  INTERNET = 3, //!< ThesisInternetRouting type (VANET TO RSU)
  INTERNET_RSU_TO_VANET = 4, //!< ThesisInternetRouting type (RSU TO VANET)


  UNKNOWN = 128
};

/**
* \ingroup mcast
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

  0                   1                   2                   3										4										5										6										7										8									  9									  10                  11                  12
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |																										Road ID																																			|    Hop Count  |        Neighbor Lifetime      |       Mobicast Radius         |          Reserved            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Destination IP Address                                                                                                                                                                                                                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP Address                                                                                                                                                                                                                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    PositionX                                  |                          PositionY                            |                              PositionZ                        |                          VelocityX                           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Velocity Y                                 |                          Velocity Z                           |                                                                                                                             |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

0-7 - Type
8-71 - Road ID
72-79 - Hop count
80-95 - Lifetime
96-111 - Mobicast radius
112-127 - Reserved

0-127 Destination IP
0-127 Source IP
0 - 31 Position x
32 - 63 Position y
64 - 95 Position z
96 - 127 Velocity x

0-31 - Source velocity y
32 - 63 - Source velocity


  0                   1                   2                   3										4										5										6										7										8									  9									  10                  11                  12
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |																										Road ID																																			|    Hop Count  |        Neighbor Lifetime      |       Mobicast Radius         |          Reserved            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Destination IP Address                                                                                                                                                                                                                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP Address                                                                                                                                                                                                                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                      PositionX                                                                |                              PositionY                                                                                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                      Position Z                                                               |                              Velocity X                                                                                      |                                                                                                                              |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                      Velocity Y                                                               |                              Velocity Z                                                                                      |                                                                                                                              |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Sign x    |    Sign y     |    Sign z     |                                                                                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

0-7 - Type
8-71 - Road ID
72-79 - Hop count
80-95 - Lifetime
96-111 - Mobicast radius
112-127 - Reserved

0-127 Destination IP
0-127 Source IP
0 - 31 Position x
32 - 63 Position y
64 - 95 Position z
96 - 127 Velocity x

0-31 - Source velocity y
32 - 63 - Source velocity




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
              Vector velocity = Vector());

  //Header size in bytes
  static const uint32_t 		m_headerSize;

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
  Vector GetPosition() const {return m_position;}

  void SetVelocity(Vector pos) {m_velocity = pos;}
  Vector GetVelocity() const {return m_velocity;}

  void SetXPos(float xPos){m_xPos = xPos;}
  float_t GetXPos() const{return m_xPos;}

  void SetYPos(float yPos){m_yPos = yPos;}
  float_t GetYPos() const{return m_yPos;}

  void SetZPos(float zPos){m_zPos = zPos;}
  float_t GetZPos() const{return m_xPos;}

  void SetXVelocity(float xVel){m_xVel = xVel;}
  float_t GetXVelocity() const{return m_xVel;}

  void SetYVelocity(float yVel){m_yVel = yVel;}
  float_t GetYVelocity() const{return m_yVel;}

  void SetZVelocity(float zVel){m_zVel = zVel;}
  float_t GetZVelocity() const{return m_zVel;}

  bool operator== (HelloHeader const & o) const;
private:
  uint8_t        m_type;          		///< Type of packet
  uint64_t 			 m_roadId;						///< Road ID
  uint8_t        m_hopCount;       		///< Hop Count
  uint16_t       m_neighborLifeTime;	///< Neighbor lifetime before expires
  uint16_t 			 m_mCastRadius;				///< mcast radius
  uint16_t			 m_reserved; 					///< reserved for future development
  Ipv6Address    m_dst;            		///< Destination IP Address
  Ipv6Address    m_origin;         		///< Originator IP Address
  Vector				 m_position;					///< Current position of node
  Vector				 m_velocity;					///< Current velocity of node

  //Alternate position variables

  float_t			m_xPos;								///< xPosition
  float_t			m_yPos;								///< yPosition
  float_t			m_zPos;								///< zPosition
  float_t			m_xVel;								///< xVelocty
  float_t			m_yVel;								///< yVelocty
  float_t			m_zVel;								///< zVelocty



};

std::ostream & operator<< (std::ostream & os, HelloHeader const &);

/////////////////////////////////////////////////////////////////////////////////

class ControlHeader : public Header
{
public:
  /// c-tor
	/*
  ControlHeader (Ipv6Address Id = Ipv6Address(), Ipv6Address Source = Ipv6Address(),
  							 uint32_t a=0, uint32_t b=0, uint64_t xPl=0, uint64_t yPl=0,
  							 uint64_t xPr=0, uint64_t yPr = 0);
	*/
  ControlHeader (Ipv6Address Id = Ipv6Address(), Ipv6Address Source = Ipv6Address(),
  							 uint32_t a=0, uint32_t b=0, Vector Apxl=Vector(), Vector Apxr=Vector()
  							 ,Vector center = Vector());


  ~ControlHeader();

  //Header size in bytes
  static const uint32_t 		m_headerSize;

  // Header serialization/deserialization
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  void SetId (Ipv6Address Id) { m_Id = Id; }
  Ipv6Address GetId () const { return m_Id; }

  void SetSource (Ipv6Address Id) { m_Source = Id; }
  Ipv6Address GetSource () const { return m_Source; }

  void SetA (uint32_t a) { m_a = a; }
  uint32_t GetA () const { return m_a; }

  void SetB (uint32_t b) { m_b = b; }
  uint32_t GetB () const { return m_b; }

  void SetApxL(Vector v) {m_Apxl = v; }
  Vector getApxL() const { return m_Apxl;}

  void SetApxR(Vector v) {m_Apxr = v; }
  Vector getApxR() const { return m_Apxr;}

  void SetCenter(Vector c) {m_center = c; }
  Vector GetCenter() const { return m_center;}

  ///////////////////////////////////////////////

  /*
  void SetXpl (uint64_t xPl) { m_xPl = xPl; }
  uint64_t GetXpl () const { return m_xPl; }

  void SetYpl (uint64_t yPl) { m_yPl = yPl; }
  uint64_t GetYpl () const { return m_yPl; }

  void SetXpr (uint64_t xPr) { m_xPr = xPr; }
  uint64_t GetXpr () const { return m_xPr; }

  void SetYpr (uint64_t yPr) { m_yPr = yPr; }
  uint64_t GetYpr () const { return m_yPr; }

	*/

  bool operator== (ControlHeader const & o) const;
private:
  Ipv6Address    m_Id;         				///< Origin and ID of sending node (IP Address)
  Ipv6Address		 m_Source;					  ///< IP Address of the node which sent the packet
  uint32_t			 m_a;								  ///< a value for mcast
  uint32_t			 m_b;								  ///< b value for mcast

  Vector				 m_Apxl;							///< Vector containing coordinates of left apex
  Vector				 m_Apxr;							///< Vector containing coordinates of right apex
  Vector 				 m_center;						///< Vector containing coordinates of Ve (Center of ZoR)

  /////////////////////////////////////////////////////////////////////
  /*
  uint64_t			 m_xPl;								///< x Coordinate of left apex
  uint64_t			 m_yPl;								///< y Coordinate of left apex
  uint64_t			 m_xPr;								///< x Coordinate of right apex
  uint64_t			 m_yPr;								///< y Coordinate of right apex
	*/
};

std::ostream & operator<< (std::ostream & os, ControlHeader const &);


} //MCAST namespace
} //
#endif /* MCASTPACKET_H */
