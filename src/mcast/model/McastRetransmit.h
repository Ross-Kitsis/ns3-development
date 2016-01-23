/*
 * McastRetransmit.h
 *
 *  Created on: Jan 22, 2016
 *      Author: ross
 */

#ifndef SRC_MCAST_MODEL_MCASTRETRANSMIT_H_
#define SRC_MCAST_MODEL_MCASTRETRANSMIT_H_

#include "ns3/simulator.h"
#include "ns3/timer.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"

/**
 * A holder for packet and timer to retransmit
 */
namespace ns3
{
namespace mcast
{

class McastRetransmit
{
public:
	//Constructor
	McastRetransmit();


	virtual ~McastRetransmit();

	Ptr<Packet> GetPacket();

	Timer GetTimer();

	void SetPacket(Ptr<Packet> p);

	Timer timerToSend;

private:

	Ptr<Packet> toRetransmit;


};
} /* namespace mcast*/
} /* namespace ns3 */

#endif /* SRC_MCAST_MODEL_MCASTRETRANSMIT_H_ */
