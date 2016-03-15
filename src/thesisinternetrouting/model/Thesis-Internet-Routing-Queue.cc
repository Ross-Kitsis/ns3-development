/*
 * Thesis-Internet-Routing-Queue.cc
 *
 *  Created on: Feb 17, 2016
 *      Author: ross
 */

#include "Thesis-Internet-Routing-Queue.h"

namespace ns3
{
namespace thesis
{

ThesisInternetRoutingQueue::ThesisInternetRoutingQueue()
{
	// TODO Auto-generated constructor stub

}

ThesisInternetRoutingQueue::~ThesisInternetRoutingQueue()
{
	// TODO Auto-generated destructor stub
}

void
ThesisInternetRoutingQueue::AddRoutingEntry(ThesisInternetQueueEntry * entry)
{
	m_queue.push_back(std::make_pair(entry,EventId ()));
}

void
ThesisInternetRoutingQueue::RemoveRoutingQueueEntry(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	for(RoutingQueueI it = m_queue.begin(); it != m_queue.end(); it++)
	{
		ThesisInternetQueueEntry * entry = it -> first;
		Ipv6Address entrySource = entry -> GetIpv6Header().GetSourceAddress();
		Ipv6Address entryDestination = entry -> GetIpv6Header().GetDestinationAddress();
		Time entrySendTime = entry -> GetPacketSendTime();

		//Source, destination and sendtime must be equal to be a duplicate packet
		if(entrySource.IsEqual(source) && entryDestination.IsEqual(destination) && entrySendTime == sendTime)
		{
			m_queue.erase(it);
			return;
		}
	}
}

bool
ThesisInternetRoutingQueue::Lookup(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	bool toReturn = false;

	for(RoutingQueueI it = m_queue.begin(); it != m_queue.end(); it++)
	{
		ThesisInternetQueueEntry * entry = it -> first;
		Ipv6Address entrySource = entry -> GetIpv6Header().GetSourceAddress();
		Ipv6Address entryDestination = entry -> GetIpv6Header().GetDestinationAddress();
		Time entrySendTime = entry -> GetPacketSendTime();

		//Source, destination and sendtime must be equal to be a duplicate packet
		if(entrySource.IsEqual(source) && entryDestination.IsEqual(destination) && entrySendTime == sendTime)
		{
			toReturn = true;
		}
	}
	return toReturn;
}

} /* namespace ns3 */
}
