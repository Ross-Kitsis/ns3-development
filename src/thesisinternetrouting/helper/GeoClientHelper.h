/*
 * GeoClientHelper.h
 *
 *  Created on: Feb 23, 2016
 *      Author: Ross Kitsis
 */

#ifndef SRC_THESISINTERNETROUTING_HELPER_GEOCLIENTHELPER_H_
#define SRC_THESISINTERNETROUTING_HELPER_GEOCLIENT6HELPER_H_

#include <stdint.h>

#include "ns3/object-factory.h"
#include "ns3/ipv6-address.h"

#include "ns3/application-container.h"
#include "ns3/node-container.h"

#include "ns3/Db.h"

namespace ns3
{

class GeoClientHelper
{
public:

	/*
	 * \brief Constructor
	 */
	GeoClientHelper(uint32_t port);

	//Destructor
	virtual ~GeoClientHelper();

	/**
	 * \brief Set the local IPv6 address.
	 * \param ip local IPv6 address
	 */
	void SetLocal (Ipv6Address ip);

	/**
	 * \brief Set the remote IPv6 address.
	 * \param ip remote IPv6 address
	 */
	void SetRemote (Ipv6Address ip);

	/**
	 * \brief Set some attributes.
	 * \param name attribute name
	 * \param value attribute value
	 */
	void SetAttribute (std::string name, const AttributeValue& value);

	/**
	 * \brief Install the application in Nodes.
	 * \param c list of Nodes
	 * \return application container
	 */
	ApplicationContainer Install (NodeContainer c);

	/**
	 * \brief Set the out interface index.
	 * This is to send to link-local (unicast or multicast) address
	 * when a node has multiple interfaces.
	 * \param ifIndex interface index
	 */
	void SetIfIndex (uint32_t ifIndex);


	/**
	 * \brief Set routers addresses for routing type 0.
	 * \param routers routers addresses
	 */
	void SetRoutersAddress (std::vector<Ipv6Address> routers);

	/**
	 * Set RSU Database
	 */
	void SetRsuDatabase(Ptr<Db> db){m_db = db;}

private:
	/**
	 * \brief An object factory.
	 */
	ObjectFactory m_factory;

	/**
	 * \brief The local IPv6 address.
	 */
	Ipv6Address m_localIp;

	/**
	 * \brief The remote IPv6 address.
	 */
	Ipv6Address m_remoteIp;

	/**
	 * \brief Out interface index.
	 */
	uint32_t m_ifIndex;

	/**
	 * \brief Routers addresses.
	 */
	std::vector<Ipv6Address> m_routers;

	Ptr<Db> m_db;

};

} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_HELPER_GeoClientHELPER_H_ */
