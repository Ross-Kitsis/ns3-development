/*
 * MobiCastAppHelper.h
 *
 *  Created on: Jan 14, 2016
 *      Author: ross
 */

#ifndef SRC_MCAST_HELPER_MOBICASTAPPHELPER_H_
#define SRC_MCAST_HELPER_MOBICASTAPPHELPER_H_

#include <stdint.h>

#include "ns3/object-factory.h"
#include "ns3/ipv6-address.h"

#include "ns3/application-container.h"
#include "ns3/node-container.h"

namespace ns3
{

class MobiCastAppHelper
{
public:
	/**
	 * \brief Constructor
	 */
	MobiCastAppHelper();

	/**
	 * \brief Destructor
	 */
	virtual ~MobiCastAppHelper();

  /**
   * \brief Set the local IPv6 address.
   * \param ip local IPv6 address
   */
  void SetLocal (Ipv6Address ip);

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

private:

  /**
   * \brief An object factory.
   */
  ObjectFactory m_factory;

  /**
   * \brief The local IPv6 address.
   */
  Ipv6Address m_localIp;

};

} /* namespace ns3 */

#endif /* SRC_MCAST_HELPER_MOBICASTAPPHELPER_H_ */
