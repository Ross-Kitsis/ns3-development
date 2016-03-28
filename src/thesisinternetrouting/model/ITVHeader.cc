/*
 * ITVHeader.cc
 *
 *  Created on: Mar 19, 2016
 *      Author: ross
 */

#include "ITVHeader.h"

namespace ns3
{
namespace thesis
{

ITVHeader::ITVHeader(Vector OriginPosition, Time OriginalTimestamp,
		bool isDtnTolerant, Vector SenderPosition,
		Vector SenderVelocity, Vector PredictedPosition, uint8_t hopCount) :
		m_OriginPosition(OriginPosition), m_OriginalTimestamp(OriginalTimestamp), m_isDtnTolerant(isDtnTolerant),
		m_SenderPosition(SenderPosition), m_SenderVelocity(SenderVelocity), m_PredictedPosition(PredictedPosition),
		m_hopCount(hopCount)
{
	// TODO Auto-generated constructor stub
}

ITVHeader::~ITVHeader()
{
	// TODO Auto-generated destructor stub
}

TypeId
ITVHeader::GetTypeId()
{
	static TypeId tid = TypeId ("ns3::thesis::ITVHeader")
    		.SetParent<Header> ()
    		.SetGroupName("thesis")
    		.AddConstructor<ITVHeader> ()
    		;
	return tid;
}

TypeId
ITVHeader::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t
ITVHeader::GetSerializedSize() const
{
	//Size of header in BYTES!!!!!
	return 76;
}

void
ITVHeader::Serialize(Buffer::Iterator i) const
{
	//Serialize Origin Position
	i.WriteHtolsbU64((uint64_t) abs (m_OriginPosition.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_OriginPosition.y * 1000));

  //Serialize timestamp (Milliseconds) and write to buffer
  i.WriteHtolsbU64(m_OriginalTimestamp.ToInteger(Time::MS));

  //Serialize DTN tolerance
  if (m_isDtnTolerant == false)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

	//Serialize Sender Position
	i.WriteHtolsbU64((uint64_t) abs (m_SenderPosition.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_SenderPosition.y * 1000));


	//Serialize velocity X
  i.WriteHtolsbU64  ((uint64_t)abs(m_SenderVelocity.x*1000));
  if (m_SenderVelocity.x >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

  //Serialize velocity Y
  i.WriteHtolsbU64  ((uint64_t)abs(m_SenderVelocity.y*1000));
  if (m_SenderVelocity.y >=0)
  {
	  i.WriteU8 (0);
  }
  else
  {
	  i.WriteU8 (1);
  }

	//Serialize Predicted VANET Position
	i.WriteHtolsbU64((uint64_t) abs (m_PredictedPosition.x * 1000));
	i.WriteHtolsbU64((uint64_t) abs (m_PredictedPosition.y * 1000));

	//Serialize hop count
	i.WriteU8(m_hopCount);
}

uint32_t
ITVHeader::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	//Read Origin position
	m_OriginPosition.x =(double) (i.ReadLsbtohU64 ()/1000.0);
	m_OriginPosition.y =(double) (i.ReadLsbtohU64 ()/1000.0);

	//Read timestamp
  m_OriginalTimestamp = MilliSeconds(i.ReadLsbtohU64());

  //Read DTN Tolerance
  int tmpsign = i.ReadU8 ();
  if (tmpsign == 0)
  {
  	m_isDtnTolerant = false;
  }else
  {
  	m_isDtnTolerant = true;
  }

  //Read sender position
	m_SenderPosition.x =(double) (i.ReadLsbtohU64 ()/1000.0);
	m_SenderPosition.y =(double) (i.ReadLsbtohU64 ()/1000.0);

	//Read sender velocity
	m_SenderVelocity.x =(double) (i.ReadLsbtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_SenderVelocity.x = -1 * m_SenderVelocity.x;

	m_SenderVelocity.y =(double) (i.ReadLsbtohU64 ()/1000.0);
  tmpsign = i.ReadU8 ();
  if (tmpsign != 0)
  	  m_SenderVelocity.y = - m_SenderVelocity.y;

  //Read predicted velocity
	m_PredictedPosition.x =(double) (i.ReadLsbtohU64 ()/1000.0);
	m_PredictedPosition.y =(double) (i.ReadLsbtohU64 ()/1000.0);

	//Deserialize hop count
	m_hopCount = (uint8_t) (i.ReadU8());

	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
ITVHeader::Print (std::ostream &os) const
{
	os << "ITV Header: Original Position: " << m_OriginPosition <<
												" Time: " << m_OriginalTimestamp <<
												" DTN: " << m_isDtnTolerant <<
												" Sender Position: " << m_SenderPosition <<
												" Sender Velocity: " << m_SenderVelocity <<
												" Predicted Position: " << m_PredictedPosition <<
												std::endl;
}

} /* namespace thesis*/
} /* namespace ns3 */
