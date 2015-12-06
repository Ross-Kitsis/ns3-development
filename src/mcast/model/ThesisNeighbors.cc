/*
 * ThesisNeighbors.cc
 *
 *  Created on: Dec 5, 2015
 *      Author: ross
 */

#include "ThesisNeighbors.h"
#include "ns3/log.h"

#include <algorithm>
#include <vector>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("ThesisNeighbors");

namespace mcast
{


Neighbor::Neighbor(Ipv6Address ip, Time delay, Vector vel, Vector pos) :
	m_neighborAddress(ip), m_delay(delay),  m_velocity(vel), m_position(pos)
	{

	}

Neighbor::~Neighbor()
{
	// TODO Auto-generated destructor stub
}

//Return IP
Ipv6Address
Neighbor::GetIp()
{
	return m_neighborAddress;
}

Vector
Neighbor::GetPosition()
{
	return m_position;
}

Vector
Neighbor::GetVelocity()
{
	return m_velocity;
}
//////////////////////////////////////////////////////////////////////////
ThesisNeighbors::ThesisNeighbors(Time delay)
{
	m_delay = delay;
}

void
ThesisNeighbors::Update(Ipv6Address ip, Vector pos, Vector vel)
{
	//Search neighbor entries, update if find a previous entry for this neighbor
	for(std::vector<Neighbor>::iterator it = m_nb.begin(); it!= m_nb.end(); ++it)
	{
		if(it->GetIp().IsEqual(ip))
		{
			//Found previous entry for this neighbor
			it->m_expire.Cancel();
			it->m_expire.SetFunction(&ThesisNeighbors::Purge,this);
			it->m_expire.SetArguments(ip);
			it->m_expire.Schedule(m_delay);
		}
	}
}

void
ThesisNeighbors::Purge(Ipv6Address toRemove)
{

}

}

}
