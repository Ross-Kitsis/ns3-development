/*
 * RsuCache.cc
 *
 *  Created on: Mar 13, 2016
 *      Author: ross
 */

#include "RsuCache.h"

namespace ns3
{

namespace thesis
{

RsuCacheEntry::RsuCacheEntry(Ipv6Address Source, Ipv6Address Destination, Vector SendingNodePosition, Vector SendingNodeVelocity, Time SendTime, Time ReceiveTime) :
		m_Source(Source), m_Destination(Destination), m_SendingNodePosition(SendingNodePosition), m_SendingNodeVelocity(SendingNodeVelocity),
		m_SendTime(SendTime), m_ReceiveTime(ReceiveTime)
{

}

RsuCacheEntry::~RsuCacheEntry()
{
}

Ipv6Address
RsuCacheEntry::GetSource()
{
	return m_Source;
}

void
RsuCacheEntry::SetSource(Ipv6Address Source)
{
	m_Source = Source;
}

Ipv6Address
RsuCacheEntry::GetDestination()
{
	return m_Destination;
}

void
RsuCacheEntry::SetDestination(Ipv6Address Destination)
{
	m_Destination = Destination;
}

Vector
RsuCacheEntry::GetSendingNodePosition()
{
	return m_SendingNodePosition;
}

void
RsuCacheEntry::SetSendingNodePosition(Vector Position)
{
	m_SendingNodePosition = Position;
}

Vector
RsuCacheEntry::GetSendingNodeVelocity()
{
	return m_SendingNodeVelocity;
}

void
RsuCacheEntry::SetSendingNodeVelocity(Vector Velocity)
{
	m_SendingNodeVelocity = Velocity;
}

Time
RsuCacheEntry::GetSendTime()
{
	return m_SendTime;
}

void
RsuCacheEntry::SetSendTime(Time SendTime)
{
	m_SendTime = SendTime;
}

Time
RsuCacheEntry::GetReceiveTime()
{
	return m_ReceiveTime;
}

void
RsuCacheEntry::SetReceiveTime(Time ReceiveTime)
{
	m_ReceiveTime = ReceiveTime;
}

bool
RsuCacheEntry::operator== (RsuCacheEntry const & o) const
{
	return (m_Destination == o.m_Destination
			&&  m_Source == o.m_Source
			&&  m_ReceiveTime == o.m_ReceiveTime
			&&  m_SendTime == o.m_SendTime);

}


RsuCache::RsuCache()
{
	// TODO Auto-generated constructor stub
	m_MaxCacheTime = Seconds(30);
}

RsuCache::~RsuCache()
{
	// TODO Auto-generated destructor stub
}

void
RsuCache::AddEntry(	RsuCacheEntry * entry)
{
	CleanCache();
	m_cache.push_back(std::make_pair (entry, EventId ()));
}

void
RsuCache::CleanCache()
{
	RSUCacheIC it = m_cache.begin ();
	while(it != m_cache.end ())
	{
		RsuCacheEntry *entry = it -> first;
		if(Simulator::Now() - entry ->GetReceiveTime() > m_MaxCacheTime)
		{
			it = m_cache.erase(it);
		}else
		{
			it++;
		}
	}
}

void
RsuCache::RemoveEntry(Ipv6Address toRemove)
{
	CleanCache();
	RSUCacheIC it = m_cache.begin ();
	while(it != m_cache.end ())
	{
		RsuCacheEntry *entry = it -> first;
		if(entry -> GetSource().IsEqual(toRemove))
		{
			it = m_cache.erase(it);
		}else
		{
			it++;
		}
	}
}

bool
RsuCache::Lookup(Ipv6Address toFind, RsuCacheEntry  &entry)
{
	CleanCache();
	bool hasEntry = false;
	Time min = Days(1);
	Time currentTime = Simulator::Now();

	for (RSUCacheIC it = m_cache.begin (); it != m_cache.end(); it++)
	{
		RsuCacheEntry * toCheck = it -> first;

		if(toCheck -> GetSource().IsEqual(toFind))
		{

			if(currentTime - toCheck -> GetSendTime() < min)
			{
				min = currentTime - toCheck -> GetSendTime();

				std::cout << "ToCheck found Min entry with address: " << toCheck -> GetSource() << std::endl;
				hasEntry = true;
				entry = *toCheck;
			}
		}
	}

	return hasEntry;
}

Vector
RsuCache::GetAverageVelocity(Ipv6Address toFind)
{
	Vector avgVelocity;
	int div = 0;
	for (RSUCacheIC it = m_cache.begin (); it != m_cache.end(); it++)
	{
		RsuCacheEntry * toCheck = it -> first;
		if(toCheck -> GetSource().IsEqual(toFind))
		{
			//Found a previous entry
			avgVelocity.x = toCheck -> GetSendingNodeVelocity().x;
			avgVelocity.y = toCheck -> GetSendingNodeVelocity().y;
			div++;
		}
	}

	if(div > 0)
	{
		avgVelocity.x = avgVelocity.x/div;
		avgVelocity.y = avgVelocity.y/div;
	}

	return avgVelocity;
}

bool
RsuCache::ContainsEntry(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	CleanCache();
	bool contains = false;
	for (RSUCacheIC it = m_cache.begin (); it != m_cache.end(); it++)
	{
		RsuCacheEntry * toCheck = it -> first;
		if(toCheck -> GetSource().IsEqual(source) && toCheck -> GetDestination().IsEqual(destination) &&toCheck -> GetSendTime() == sendTime)
		{
			contains = true;
			break;
		}
	}
	return contains;
}


}
} /* namespace ns3 */
