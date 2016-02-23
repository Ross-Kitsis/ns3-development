/*
 * ThesisPing6Helper.h
 *
 *  Created on: Feb 23, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_HELPER_THESISPING6HELPER_H_
#define SRC_THESISINTERNETROUTING_HELPER_THESISPING6HELPER_H_

#include <stdint.h>

#include "ns3/object-factory.h"
#include "ns3/ipv6-address.h"

#include "ns3/application-container.h"
#include "ns3/node-container.h"

namespace ns3
{

class ThesisPing6Helper
{
public:

	/*
	 * \brief Constructor
	 */
	ThesisPing6Helper();

	//Destructor
	virtual ~ThesisPing6Helper();

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

};

} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_HELPER_THESISPING6HELPER_H_ */
