/*
 * ITVHeader.h
 *
 *  Created on: Mar 19, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_MODEL_ITVHEADER_H_
#define SRC_THESISINTERNETROUTING_MODEL_ITVHEADER_H_

#include "ns3/header.h"
#include "ns3/ipv6.h"
#include "ns3/nstime.h"
#include "ns3/vector.h"
#include "ns3/simulator.h"
#include "ns3/address-utils.h"

namespace ns3
{

namespace thesis
{

class ITVHeader : public Header
{
public:

	ITVHeader(Vector m_OriginPosition = Vector(), Time m_OriginalTimestamp = Simulator::Now(),
						bool m_isDtnTolerant = false, Vector m_SenderPosition = Vector(),
						Vector m_SenderVelocity = Vector(), Vector m_PredictedPosition = Vector());

	virtual ~ITVHeader();

	//Size of header
	static const uint32_t m_headerSize;

	//Header serialize/deserialize
	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);

	void Print(std::ostream &os) const;

	Vector GetOriginPosition() {return m_OriginPosition;}
	void SetOriginPosition(Vector position) {m_OriginPosition = position;}

	Time GetOriginalTimestamp() {return m_OriginalTimestamp;}
	void SetOriginalTimestamp(Time t) {m_OriginalTimestamp = t;}

	bool GetIsDtnTolerant() {return m_isDtnTolerant;}
	void SetIsDtnTolerant(bool b) {m_isDtnTolerant = b;}

	Vector GetSenderPosition() {return m_SenderPosition;}
	void SetSenderPosition(Vector position) {m_SenderPosition = position;}

	Vector GetSenderVelocity() {return m_SenderVelocity;}
	void SetSenderVelocity(Vector velocity) {m_SenderVelocity = velocity;}

	Vector GetPredictedPosition() {return m_PredictedPosition;}
	void SetPredictedPosition(Vector position) {m_PredictedPosition = position;}

private:

	Vector m_OriginPosition;
	Time m_OriginalTimestamp;
	bool m_isDtnTolerant;
	Vector m_SenderPosition;
	Vector m_SenderVelocity;
	Vector m_PredictedPosition;

};

} /* namespace thesis*/
} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_MODEL_ITVHEADER_H_ */
