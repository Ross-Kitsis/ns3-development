/*
 * InternetHeader.cc
 *
 *  Created on: Feb 19, 2016
 *      Author: ross
 */

#include "InternetHeader.h"

namespace ns3
{

namespace thesis
{
InternetHeader::InternetHeader(Vector position, Vector velocity, Time timestamp, bool isDtnTolerant, Vector senderVelocity, Vector senderPosition) :
		m_position(position), m_velocity(velocity), m_timestamp(timestamp), m_isDtnTolerant(isDtnTolerant),
		m_SenderPosition(senderPosition), m_SenderVelocity(senderVelocity)

{
	// TODO Auto-generated constructor stub

}


InternetHeader::~InternetHeader()
{
	// TODO Auto-generated destructor stub
}

TypeId
InternetHeader::GetTypeId()
{
	static TypeId tid = TypeId ("ns3::thesis::InternetHeader")
    		.SetParent<Header> ()
    		.SetGroupName("thesis")
    		.AddConstructor<InternetHeader> ()
    		;
	return tid;
}

TypeId
InternetHeader::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t
InternetHeader::GetSerializedSize() const
{
	return 616;
}

void
InternetHeader::Serialize(Buffer::Iterator i) const
{
	//Serialize position
	i.WriteHtolsbU64((uint64_t) abs (m_position.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_position.y * 1000));

	//Serialize velocity
  i.WriteHtonU64  ((uint64_t)abs(m_velocity.x*1000));
  if (m_velocity.x >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  i.WriteHtonU64  ((uint64_t)abs(m_velocity.y*1000));
  if (m_velocity.y >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  //Serialize timestamp (Milliseconds)
  m_timestamp.ToInteger(Time::MS);

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
  i.WriteHtonU64  ((uint64_t)abs(m_SenderVelocity.x*1000));
  if (m_SenderVelocity.x >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  i.WriteHtonU64  ((uint64_t)abs(m_SenderVelocity.y*1000));
  if (m_SenderVelocity.y >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

}

uint32_t
InternetHeader::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_position.x =(double) (i.ReadNtohU64 ()/1000.0);
	m_position.y =(double) (i.ReadNtohU64 ()/1000.0);

  m_velocity.x =(double) (i.ReadNtohU64 ()/1000.0);
	uint8_t tmpsign;
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
	  m_velocity.x = - m_velocity.x;

	m_velocity.y =(double) (i.ReadNtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_velocity.y = - m_velocity.y;

  m_timestamp = MilliSeconds(i.ReadNtohU64());

  tmpsign = i.ReadU8 ();
  if (tmpsign == 0)
  {
  	m_isDtnTolerant = false;
  }else
  {
  	m_isDtnTolerant = true;
  }

	m_SenderPosition.x =(double) (i.ReadNtohU64 ()/1000.0);
	m_SenderPosition.y =(double) (i.ReadNtohU64 ()/1000.0);

	m_SenderVelocity.x =(double) (i.ReadNtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_SenderVelocity.x = - m_velocity.x;

	m_SenderVelocity.y =(double) (i.ReadNtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_SenderVelocity.y = - m_velocity.y;

	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
InternetHeader::Print (std::ostream &os) const
{
	os << "Internet Header: Position: " << m_position << " Velocity: " << m_velocity << " Time: " << m_timestamp << " DTN: " << m_isDtnTolerant << std::endl;
}

bool
InternetHeader::operator== (InternetHeader const & o) const
{
	return (m_position.x == o.m_position.x
			 && m_position.y == o.m_position.y
			 && m_velocity.x == o.m_velocity.x
			 && m_velocity.y == o.m_velocity.y
			 && m_timestamp == o.m_timestamp
			 && m_isDtnTolerant == o.m_isDtnTolerant);

}

} /* namespace thesis */
} /* namespace ns3 */
