/*
 * GeoRequestHeader.cc
 *
 *  Created on: Feb 19, 2016
 *      Author: ross Kitsis
 */

#include "GeoRequestHeader.h"

#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GeoRequestHeader");
//NS_OBJECT_ENSURE_REGISTERED(GeoRequestHeader);

namespace thesis
{
GeoRequestHeader::GeoRequestHeader(Vector OriginPosition, Vector OriginVelocity, Time timestamp, bool isDtnTolerant, Vector senderPosition, Vector senderVelocity, Ipv6Address RsuAddress, uint8_t hopCount, Vector QueryPosition) :
		m_OriginPosition(OriginPosition), m_OriginVelocity(OriginVelocity), m_timestamp(timestamp), m_isDtnTolerant(isDtnTolerant),
		m_SenderPosition(senderPosition), m_SenderVelocity(senderVelocity), m_RsuAddress(RsuAddress), m_hopCount(hopCount), m_QueryPosition(QueryPosition)

{
	// TODO Auto-generated constructor stub

}


GeoRequestHeader::~GeoRequestHeader()
{
	// TODO Auto-generated destructor stub
}

TypeId
GeoRequestHeader::GetTypeId()
{
	static TypeId tid = TypeId ("ns3::thesis::GeoRequestHeader")
    		.SetParent<Header> ()
    		.SetGroupName("thesis")
    		.AddConstructor<GeoRequestHeader> ()
    		;
	return tid;
}

TypeId
GeoRequestHeader::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t
GeoRequestHeader::GetSerializedSize() const
{
	//Size of header in BYTES!!!!!
//	return 94;

	//Updated size value
	return 110;

}

void
GeoRequestHeader::Serialize(Buffer::Iterator i) const
{
	//Serialize Origin Position
	i.WriteHtolsbU64((uint64_t) abs (m_OriginPosition.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_OriginPosition.y * 1000));

	//Serialize velocity
  i.WriteHtolsbU64  ((uint64_t)abs(m_OriginVelocity.x*1000));
  if (m_OriginVelocity.x >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  i.WriteHtolsbU64  ((uint64_t)abs(m_OriginVelocity.y*1000));
  if (m_OriginVelocity.y >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  //Serialize timestamp (Milliseconds) and write to buffer
  i.WriteHtolsbU64(m_timestamp.ToInteger(Time::MS));


  //Serialize DTN tolerance
  if (m_isDtnTolerant == false)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

	//Serialize sender position
	i.WriteHtolsbU64((uint64_t) abs (m_SenderPosition.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_SenderPosition.y * 1000));

	//Serialize sender velocity
  i.WriteHtolsbU64  ((uint64_t)abs(m_SenderVelocity.x*1000));
  if (m_SenderVelocity.x >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  i.WriteHtolsbU64  ((uint64_t)abs(m_SenderVelocity.y*1000));
  if (m_SenderVelocity.y >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  WriteTo(i,m_RsuAddress);

  i.WriteU8(m_hopCount);

	//Serialize query position
	i.WriteHtolsbU64((uint64_t) abs (m_QueryPosition.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_QueryPosition.y * 1000));

}

uint32_t
GeoRequestHeader::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_OriginPosition.x =(double) (i.ReadLsbtohU64 ()/1000.0);
	m_OriginPosition.y =(double) (i.ReadLsbtohU64 ()/1000.0);

  m_OriginVelocity.x =(double) (i.ReadLsbtohU64 ()/1000.0);
	uint8_t tmpsign;
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
	  m_OriginVelocity.x = -1 * m_OriginVelocity.x;

	m_OriginVelocity.y =(double) (i.ReadLsbtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_OriginVelocity.y = -1 * m_OriginVelocity.y;

  m_timestamp = MilliSeconds(i.ReadLsbtohU64());

  tmpsign = i.ReadU8 ();
  if (tmpsign == 0)
  {
  	m_isDtnTolerant = false;
  }else
  {
  	m_isDtnTolerant = true;
  }

	m_SenderPosition.x =(double) (i.ReadLsbtohU64 ()/1000.0);
	m_SenderPosition.y =(double) (i.ReadLsbtohU64 ()/1000.0);

	m_SenderVelocity.x =(double) (i.ReadLsbtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_SenderVelocity.x = -1 * m_SenderVelocity.x;

	m_SenderVelocity.y =(double) (i.ReadLsbtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_SenderVelocity.y = - m_SenderVelocity.y;

	ReadFrom (i, m_RsuAddress);

	m_hopCount = i.ReadU8();

	//Deserialize query position
	m_QueryPosition.x =(double) (i.ReadLsbtohU64 ()/1000.0);
	m_QueryPosition.y =(double) (i.ReadLsbtohU64 ()/1000.0);

	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
GeoRequestHeader::Print (std::ostream &os) const
{
	os << "GeoRequest Header: Original Position: " << m_OriginPosition <<
												" Origin Velocity: " << m_OriginVelocity << " "
												" Time: " << m_timestamp <<
												" DTN: " << m_isDtnTolerant <<
												" Sender Position: " << m_SenderPosition <<
												" Sender Velocity: " << m_SenderVelocity <<
												" RSU address: " << m_RsuAddress <<
												std::endl;
}

bool
GeoRequestHeader::operator== (GeoRequestHeader const & o) const
{
	return (m_OriginPosition.x == o.m_OriginPosition.x
			 && m_OriginPosition.y == o.m_OriginPosition.y
			 && m_OriginVelocity.x == o.m_OriginVelocity.x
			 && m_OriginVelocity.y == o.m_OriginVelocity.y
			 && m_timestamp == o.m_timestamp
			 && m_isDtnTolerant == o.m_isDtnTolerant);

}

} /* namespace thesis */
} /* namespace ns3 */
