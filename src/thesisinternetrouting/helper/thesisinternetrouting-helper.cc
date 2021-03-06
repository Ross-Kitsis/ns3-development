/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "thesisinternetrouting-helper.h"

namespace ns3
{

ThesisInternetRoutingHelper::ThesisInternetRoutingHelper() :
		Ipv6RoutingHelper(), m_IsRSU(false)
{
	m_agentFactory.SetTypeId("ns3::thesis::ThesisInternetRoutingProtocol");
}

ThesisInternetRoutingHelper*
ThesisInternetRoutingHelper::Copy(void) const
{
	return new ThesisInternetRoutingHelper(*this);
}

Ptr<Ipv6RoutingProtocol>
ThesisInternetRoutingHelper::Create (Ptr<Node> node) const
{
	std::cout << "Creating new thesisInternetRouting protocol (new) for" << node << std::endl;
	Ptr<thesis::ThesisInternetRoutingProtocol> agent = m_agentFactory.Create<thesis::ThesisInternetRoutingProtocol> ();

	//////////////ADD DATABASE ASSIGNMENT HERE!!!
	agent ->SetRsuDatabase(m_database);
	agent ->SetIsRSU(m_IsRSU);

	node->AggregateObject (agent);
	return agent;
}

void
ThesisInternetRoutingHelper::Set (std::string name, const AttributeValue &value)
{
	m_agentFactory.Set (name, value);
}

void
ThesisInternetRoutingHelper::SetRsuDatabase(Ptr<Db> database)
{
	m_database = database;
}

void
ThesisInternetRoutingHelper::SetIsRSU(bool isRSU)
{
	m_IsRSU = isRSU;
}

/*
int64_t
ThesisInternetRoutingHelper::AssignStreams (NodeContainer c, int64_t stream)
{
	std::cout << "Assigning Streams" << std::endl;
	int64_t currentStream = stream;
	Ptr<Node> node;
	for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
	{
		node = (*i);
		Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
		NS_ASSERT_MSG (ipv6, "Ipv6 not installed on node");
		Ptr<Ipv6RoutingProtocol> proto = ipv6->GetRoutingProtocol ();
		NS_ASSERT_MSG (proto, "Ipv6 routing not installed on node");
		Ptr<thesis::ThesisInternetRoutingProtocol> thesis = DynamicCast<thesis::ThesisInternetRoutingProtocol> (proto);
		if (thesis)
		{
			currentStream += thesis->AssignStreams (currentStream);
			continue;
		}
		// Mcast may also be in a list
		Ptr<Ipv6ListRouting> list = DynamicCast<Ipv6ListRouting> (proto);
		if (list)
		{
			int16_t priority;
			Ptr<Ipv6RoutingProtocol> listProto;
			Ptr<thesis::ThesisInternetRoutingProtocol> thesis;
			for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
			{
				listProto = list->GetRoutingProtocol (i, priority);
				thesis = DynamicCast<thesis::ThesisInternetRoutingProtocol> (listProto);
				if (thesis)
				{
					currentStream += thesis->AssignStreams (currentStream);
					break;
				}
			}
		}
	}
	return (currentStream - stream);
}
*/

}

