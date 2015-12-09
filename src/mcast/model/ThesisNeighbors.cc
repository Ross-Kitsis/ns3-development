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
	return;
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

void
Neighbor::SetVelocity(Vector v)
{
	m_velocity = v;
}

void
Neighbor::SetPosition(Vector p)
{
	m_position = p;
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
	for(std::list<Neighbor>::iterator it = m_nb.begin(); it != m_nb.end(); ++it)
	{


		if(it->GetIp().IsEqual(ip))
		{
			//Found previous entry for this neighbor
			it->m_expire.Cancel();
			it->m_expire.SetFunction(&ThesisNeighbors::Purge,this);
			it->m_expire.SetArguments(ip);
			it->m_expire.Schedule(m_delay);
			it->SetVelocity(vel);
			it->SetPosition(pos);

			std::cout<< "	<<<<<	Updating neighbor velocity and position" << std::endl;

			return;
		}
	}

	//Unable to find neighbor; must be new, create new neighbor object and add to list
	Neighbor * neighbor = new Neighbor (ip, m_delay, vel, pos);
	std::cout<< "		Adding new neighbor >>>>" << m_delay.GetSeconds() << " IP: " << neighbor->GetIp() <<  std::endl;

	if(neighbor->m_expire.IsRunning())
	{
		std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>> Already running <<<<<<<<<<<<<<<<";
	}else
	{

		neighbor->m_expire.SetFunction(&ThesisNeighbors::Purge,this);
		neighbor->m_expire.SetArguments(neighbor->GetIp());

		neighbor->m_expire.Schedule(Time(Seconds(15)));
	}
	m_nb.push_back(*neighbor);



	//m_nb.
	return;
}



/**
 * Node has not received a hello message and must clear expired neighbor from table
 */
void
ThesisNeighbors::Purge(Ipv6Address toRemove)
{
	for(std::list<Neighbor>::iterator it = m_nb.begin(); it!= m_nb.end(); ++it)
	{
		if(it->GetIp().IsEqual(toRemove))
		{
			std::cout << "Removing IP " << toRemove << " from list" << std::endl;
			m_nb.erase(it);
			return;
		}
	}
}

}

}
