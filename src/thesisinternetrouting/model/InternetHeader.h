/*
 * InternetHeader.h
 *
 *  Created on: Feb 19, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_MODEL_INTERNETHEADER_H_
#define SRC_THESISINTERNETROUTING_MODEL_INTERNETHEADER_H_

#include "ns3/vector.h"
#include "ns3/nstime.h"
#include "ns3/header.h"
#include "ns3/ipv6-address.h"

namespace ns3
{

namespace thesis
{

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 *
 * 64 Bit x source position
 * 64 bit y source position
 * (128)
 *
 * 64 bit x source velocity
 * 8 bit positive/negetive sender x velocity
 * (72)
 *
 * 64 bit y source velocity
 * 8 but positive/negetive sender y velocity
 * (72)
 *
 * 64 bit time
 *
 * 8 bit DTN
 *
 * 64 bit x sender position
 * 64 bit y sender position
 * (128)
 *
 * 64 bit x sender velocity
 * 8 bit positive/negetive sender x velocity
 * (72)
 *
 * 64 bit y source velocity
 * 8 but positive/negetive sender y velocity
 * (72)
 *
 * 616
 *
 */


class InternetHeader : public Header
{
public:
	InternetHeader(Vector m_position = Vector(), Vector m_velocity = Vector(), Time m_timestamp = Time(),
								 bool m_isDtnTolerant = false, Vector m_SenderPosition = Vector(), Vector m_SenderVelocity = Vector());

	virtual ~InternetHeader();

	//Size of header
	static const uint32_t m_headerSize;

	//Header serialize/deserialize
	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);

	void Print(std::ostream &os) const;

	//Fields
	void SetSourceVelocity(Vector v) {m_velocity = v;}
	Vector GetSourceVelocity() {return m_velocity;}

	void SetSourcePosition(Vector p) {m_position = p;}
	Vector GetSourcePosition() {return m_position;}

	void SetTimestamp(Time t) {m_timestamp = t;}
	Time GetTimestamp() {return m_timestamp;}

	void SetDtnTolerance(bool b) {m_isDtnTolerant = b;}
	bool GetDtnTolerance() {return m_isDtnTolerant;}

	void SetSenderVelocity(Vector v) {m_SenderVelocity = v;}
	Vector GetSenderVelocity() {return m_SenderVelocity;}

	void SetSenderPosition(Vector p) {m_SenderPosition = p;}
	Vector GetSenderPosition() {return m_SenderPosition;}

	bool operator== (InternetHeader const & o) const;

private:

	/*
	 * Position of source node
	 */
	Vector m_position;

	/*
	 * Velocity of source node
	 */
	Vector m_velocity;

	/*
	 * Timestamp of when the initial message was sent
	 */
	Time m_timestamp;

	/*
	 * Boolean determining if packet can be buffered if DTN allowed
	 */
	bool m_isDtnTolerant;

	///////////////////////////////////////////////////

	/*
	 * Position of sending node
	 */
	Vector m_SenderPosition;

	/*
	 * Velocity of sending node
	 */
	Vector m_SenderVelocity;

};
} /*namesaoce thesisinternet*/
} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_MODEL_INTERNETHEADER_H_ */
