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
 * 32 Bit x position
 * 32 bit y position
 * 32 bit x velocity
 * 8 bit positive/negetive x velocity
 * 32 bit y velocity
 * 8 but positive/negetive y velocity
 * 64 bit time
 * 8 bit DTN
 *
 */


class InternetHeader : public Header
{
public:
	InternetHeader(Vector m_position = Vector(), Vector m_velocity = Vector(), Time m_timestamp = Time(), bool m_isDtnTolerant = false);
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
	void SetVelocity(Vector v) {m_velocity = v;}
	Vector GetVelocity() {return m_velocity;}

	void SetPosition(Vector p) {m_position = p;}
	Vector GetPosition() {return m_position;}

	void SetTimestamp(Time t) {m_timestamp = t;}
	Time GetTimestamp() {return m_timestamp;}

	void SetDtnTolerance(bool b) {m_isDtnTolerant = b;}
	bool GetDtnTolerance() {return m_isDtnTolerant;}

	bool operator== (InternetHeader const & o) const;

private:

	/*
	 * Position of sending node
	 */
	Vector m_position;

	/*
	 * Velocity of sending node
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

};
} /*namesaoce thesisinternet*/
} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_MODEL_INTERNETHEADER_H_ */
