/*
 * Db.h
 *
 *  Created on: Jan 25, 2016
 *      Author: ross
 */

#ifndef SCRATCH_SUBDIR_DB_H_
#define SCRATCH_SUBDIR_DB_H_

#include "ns3/vector.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/mobility-model.h"
#include "ns3/ipv6.h"
#include <iostream>
#include <cmath>

namespace ns3
{

/**
 * Class defining database entry
 */
class DbEntry
{
public:

	DbEntry(Vector bottomCorner, Vector topCorner, Ipv6Address RsuAddress, Vector RsuPosition, uint32_t ZoneId);

	virtual ~DbEntry();

	/**
	 * Set bottom left corner
	 */
	Vector GetBottomCorner();

	/**
	 * Get top right corner
	 */
	void SetBottomCorner(Vector corner);

	/**
	 * Get top right corner
	 */
	Vector GetTopCorner();

	/**
	 * Set tope right corner
	 */
	void SetTopCorner(Vector corner);

	/**
	 * Get IP address of the RSU in the zone
	 */
	Ipv6Address GetRsuAddress();

	/**
	 * Set IP address of the RSU in the zone
	 */
	void SetRsuAddress(Ipv6Address address);

	/**
	 * Get Position of the RSU in the zone
	 */
	Vector GetRsuPosition();

	/**
	 * Set Position of the RSU in the zone
	 */
	void SetRsuPosition(Vector position);

	/**
	 * Get zone id
	 */
	uint32_t GetZoneId();

	/**
	 * Set zone Id
	 */
	void SetZoneId(uint32_t id);

private:

	//Position of left corner of zone
	Vector m_bottomLeftCorner;

	//Position of right corner of zone
	Vector m_topRightCorner;

	//IPv6 address of RSU
	Ipv6Address m_RsuAddress;

	//Vector containing position of the RSU
	Vector m_RsuPosition;

	//Zone Id, used to generate IPv6 Addresses for nodes as well as RSU
	uint32_t m_ZoneId;

};

/**
 * Database class to hold database entries
 */
class Db
{
public:

	Db();

	virtual ~Db();

	void CreateDatabase(NodeContainer c, uint32_t length, uint32_t width);

	DbEntry GetEntryForCurrentPosition(Vector position);

private:

	double GetAreaOfTriangle(Vector A, Vector B, Vector C);

	std::list<DbEntry> m_db;

};

} /* namespace ns3 */

#endif /* SCRATCH_SUBDIR_DB_H_ */
