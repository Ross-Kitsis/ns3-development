/*
 * GeoClientHelper.cc
 *
 *  Created on: Feb 23, 2016
 *      Author: Ross Kitsis
 */

#include "GeoClientHelper.h"
#include "ns3/uinteger.h"
#include "ns3/GeoQueryClient.h"

namespace ns3
{

GeoClientHelper::GeoClientHelper()
	: m_ifIndex(0)
{
  m_factory.SetTypeId (GeoQueryClient::GetTypeId ());
}

GeoClientHelper::~GeoClientHelper()
{
	// TODO Auto-generated destructor stub
}

void
GeoClientHelper::SetLocal (Ipv6Address ip)
{
  m_localIp = ip;
}

void
GeoClientHelper::SetRemote (Ipv6Address ip)
{
  m_remoteIp = ip;
}

void
GeoClientHelper::SetAttribute (std::string name, const AttributeValue& value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
GeoClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<GeoQueryClient> client = m_factory.Create<GeoQueryClient> ();
      client -> SetRsuDatabase(m_db);

      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

void
GeoClientHelper::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void
GeoClientHelper::SetRoutersAddress (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}


} /* namespace ns3 */
