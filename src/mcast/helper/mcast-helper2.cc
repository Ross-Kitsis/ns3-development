/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "mcast-helper2.h"
#include "ns3/mcast2.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv6-list-routing.h"
#include <iostream>

namespace ns3 {

McastHelper2::McastHelper2() :
  		Ipv6RoutingHelper ()
{
	m_agentFactory.SetTypeId ("ns3::mcast::ThesisRoutingProtocol");
}

McastHelper2*
McastHelper2::Copy (void) const
{
	return new McastHelper2 (*this);
}

Ptr<Ipv6RoutingProtocol>
McastHelper2::Create (Ptr<Node> node) const
{
	std::cout << "Creating new mcast protocol (new) for" << node << std::endl;
	Ptr<mcast::ThesisRoutingProtocol> agent = m_agentFactory.Create<mcast::ThesisRoutingProtocol> ();
	node->AggregateObject (agent);

	return agent;
}

void
McastHelper2::Set (std::string name, const AttributeValue &value)
{
	m_agentFactory.Set (name, value);
}

int64_t
McastHelper2::AssignStreams (NodeContainer c, int64_t stream)
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
		Ptr<mcast::ThesisRoutingProtocol> mcast = DynamicCast<mcast::ThesisRoutingProtocol> (proto);
		if (mcast)
		{
			currentStream += mcast->AssignStreams (currentStream);
			continue;
		}
		// Mcast may also be in a list
		Ptr<Ipv6ListRouting> list = DynamicCast<Ipv6ListRouting> (proto);
		if (list)
		{
			int16_t priority;
			Ptr<Ipv6RoutingProtocol> listProto;
			Ptr<mcast::ThesisRoutingProtocol> listMcast;
			for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
			{
				listProto = list->GetRoutingProtocol (i, priority);
				listMcast = DynamicCast<mcast::ThesisRoutingProtocol> (listProto);
				if (listMcast)
				{
					currentStream += listMcast->AssignStreams (currentStream);
					break;
				}
			}
		}
	}
	return (currentStream - stream);
}


}

