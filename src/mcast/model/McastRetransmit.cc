/*
 * McastRetransmit.cc
 *
 *  Created on: Jan 22, 2016
 *      Author: ross
 */

#include "McastRetransmit.h"

namespace ns3
{

namespace mcast
{
McastRetransmit::McastRetransmit()
{
	// TODO Auto-generated constructor stub

}

McastRetransmit::~McastRetransmit()
{
	// TODO Auto-generated destructor stub
}

Ptr<Packet>
McastRetransmit::GetPacket()
{
	return toRetransmit;
}

void
McastRetransmit::SetPacket(Ptr<Packet> p)
{
	toRetransmit = p;
}

Timer
McastRetransmit::GetTimer()
{
	return timerToSend;
}

void
McastRetransmit::SetControlHeader(ControlHeader cHeader)
{
	ctrl = cHeader;
}

ControlHeader
McastRetransmit::GetControlHeader()
{
	return ctrl;
}

} /* namespace mcast*/
} /* namespace ns3 */
