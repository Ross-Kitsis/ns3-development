/*
 * MobiCastAppHelper.cc
 *
 *  Created on: Jan 14, 2016
 *      Author: ross
 */


#include "ns3/MobiCastApp.h"

#include "MobiCastAppHelper.h"

namespace ns3
{

MobiCastAppHelper::MobiCastAppHelper()
{
	// TODO Auto-generated constructor stub
	m_factory.SetTypeId(MobiCastApp::GetTypeId());

}

MobiCastAppHelper::~MobiCastAppHelper()
{
	// TODO Auto-generated destructor stub
}

void
MobiCastAppHelper::SetLocal (Ipv6Address ip)
{
  m_localIp = ip;
}

void
MobiCastAppHelper::SetInterval(Time interval)
{
	m_interval = interval;
}

void
MobiCastAppHelper::SetEventProbability(double v)
{
	m_eventProbability = v;
}

void
MobiCastAppHelper::SetSafetyInterval(Time interval)
{
	m_safetyMessageInterval = interval;
}

void
MobiCastAppHelper::SetAttribute (std::string name, const AttributeValue& value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
MobiCastAppHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<MobiCastApp> client = m_factory.Create<MobiCastApp> ();
      //client->SetLocal (m_localIp);
      client->SetAttemptInterval(m_interval);
      client->SetSuccessInterval(m_safetyMessageInterval);
      client->SetSendProbability(m_eventProbability);

      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();

      std::cout << "Attempting to set Ipv6 and routing protocol" << std::endl;

      client->SetIpV6(ipv6);

  		Ptr<Ipv6RoutingProtocol> proto = ipv6->GetRoutingProtocol ();
  		Ptr<mcast::ThesisRoutingProtocol> mcast = DynamicCast<mcast::ThesisRoutingProtocol> (proto);


      client->SetRoutingProtocol(mcast);
      //client->SetRemote (m_remoteIp);
      //client->SetIfIndex (m_ifIndex);
      //client->SetRouters (m_routers);
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}



} /* namespace ns3 */
