/*
 * ThesisNeighbors.h
 *
 *  Created on: Dec 5, 2015
 *      Author: ross
 */

#ifndef MODEL_THESISNEIGHBORS
#define MODEL_THESISNEIGHBORS

#include "ns3/simulator.h"
#include "ns3/timer.h"
#include "ns3/ipv6-address.h"
#include "ns3/vector.h"

//Thesis includes
#include "mcast-packet.h"
#include "mcast-utils.h"


//Exclude mac pieces for the moment

namespace ns3
{
namespace mcast
{

class Neighbor
{
public:

	Neighbor();

	Neighbor (Ipv6Address ip, Time delay, Vector vel, Vector pos);

	~Neighbor();


	/*
	 * \brief Returns the IP address of the neighbor
	 */
	Ipv6Address GetIp(void);

	/*
	 * \brief Return the neighbor position
	 */
	Vector GetPosition(void);

	/*
	 * \brief Set the neighbor position
	 */
	void SetPosition(Vector pos);

	/*
	 * \brief Returns the neighbor velocity
	 */
	Vector GetVelocity(void);

	/*
	 * \brief Set the neighbor velocity
	 */
	void SetVelocity(Vector v);

	/*
	 * Timer counting down expiring of neighbor relationship
	 */
	Timer m_expire;

private:

	//Exclude MAC issues here for now
	Ipv6Address m_neighborAddress;
	Time m_delay;
	Vector m_velocity;
	Vector m_position;
};

class ThesisNeighbors
{
public:

	//Constructor
	ThesisNeighbors(Time delay);

	/*
	 * \brief Function called when neighbor timer expires
	 *
	 * \param The ip address of the node to remove
	 *
	 */
	void Purge(Ipv6Address toRemove);

	/*
	 * \brief Update list of neighbors
	 *
	 * If the neighbor exists reset the timer, update position and velocity
	 * If the neighbor does not exist add it to the list
	 *
	 */
	void Update(Ipv6Address address, Vector pos, Vector vel);

	/*
	 * \brief Checks if a neighbor relationship exists between the node and the passed address
	 */
	bool IsNeighbor(Ipv6Address toFind);

	/**
	 * \brief Returns neighbor position with the given address
	 *
	 * Assumes the neighbor exists
	 *
	 */
	Vector GetNeighborPosition(Ipv6Address toGet);

	/**
	 * \brief Return the number of neighbors currently in the nieghbor list
	 */
	uint32_t GetNeighborTableSize(void) {return (uint32_t)m_nb.size();}

	/**
	 *	\brief Checks all neighbors in the neighbor table determine if have one
	 *	close to to the passed point than the passed position. Returns true if
	 *	there exists a close neighbor; otherwise false;
	 */
	bool HaveCloserNeighbor(Vector PosToCheck, double currentDistance);

	/**
	 * \brief Finds and returns the closest neighbor to the apex (Furthest distance from current node)
	 *
	 */
	double getDistanceClosestNeighborToApex(Vector PosToCheck, double currentDistance);

private:

	/*
	 * \brief Return the list of neighbors
	 *
	 */
	std::list<Neighbor> GetNeighbors(void);


	//Attributes
	std::list<Neighbor> m_nb;
	Time m_delay;
	McastUtils m_mutils;

};

} //mcast
} //ns3
#endif /* SRC_MCAST_MODEL_THESISNEIGHBORS_H_ */
