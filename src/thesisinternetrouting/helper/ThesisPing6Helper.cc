/*
 * ThesisPing6Helper.cc
 *
 *  Created on: Feb 23, 2016
 *      Author: ross
 */

#include "ThesisPing6Helper.h"
#include "ns3/uinteger.h"
#include "ns3/ThesisPing6.h"

namespace ns3
{

ThesisPing6Helper::ThesisPing6Helper()
	: m_ifIndex(0)
{
  m_factory.SetTypeId (ThesisPing6::GetTypeId ());
}

ThesisPing6Helper::~ThesisPing6Helper()
{
	// TODO Auto-generated destructor stub
}

void
ThesisPing6Helper::SetLocal (Ipv6Address ip)
{
  m_localIp = ip;
}

void
ThesisPing6Helper::SetRemote (Ipv6Address ip)
{
  m_remoteIp = ip;
}

void
ThesisPing6Helper::SetAttribute (std::string name, const AttributeValue& value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
ThesisPing6Helper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<ThesisPing6> client = m_factory.Create<ThesisPing6> ();
      client->SetLocal (m_localIp);
      client->SetRemote (m_remoteIp);
      client->SetIfIndex (m_ifIndex);
      client->SetRouters (m_routers);
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

void
ThesisPing6Helper::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void
ThesisPing6Helper::SetRoutersAddress (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}


} /* namespace ns3 */
