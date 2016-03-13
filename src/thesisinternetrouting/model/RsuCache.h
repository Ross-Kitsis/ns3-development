/*
 * RsuCache.h
 *
 *  Created on: Mar 13, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_MODEL_RSUCACHE_H_
#define SRC_THESISINTERNETROUTING_MODEL_RSUCACHE_H_

namespace ns3
{
namespace thesis
{

class RsuCacheEntry
{
public:

	RsuCacheEntry(Ipv6Address m_Source = Ipv6Address(), Ipv6Address m_Destination = Ipv6Address(),
								Time m_SendTime = Simulator::Now(), Time m_ReceiveTime = Simulator::Now());

	~RsuCacheEntry();

	Ipv6Address GetSource();
	void SetSource(Ipv6Address Source);

	Ipv6Address GetDestination();
	void SetDestination(Ipv6Address Destination);

	Time GetSendTime();
	void SetSendTime(Time SendTime);


	Time GetReceiveTime();
	void SetReceieveTime();

private:
	/*
	 * Source of transmission in VANET
	 */
	Ipv6Address m_Source;
	/*
	 * Destination of transmittion
	 */
	Ipv6Address m_Destination;
	/*
	 * Timestamp of when message was sent
	 */
	Time m_SendTime;
	/*
	 * Timestamp of when message was received
	 */
	Time ReceiveTime;

};

class RsuCache
{
public:
	RsuCache();
	virtual ~RsuCache();
};

} /* namespace thesis*/
} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_MODEL_RSUCACHE_H_ */
