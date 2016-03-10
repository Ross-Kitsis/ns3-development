/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef THESISINTERNETROUTING_HELPER_H
#define THESISINTERNETROUTING_HELPER_H

#include "ns3/thesisinternetrouting2.h"

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv6-routing-helper.h"

#include "ns3/Db.h"

namespace ns3
{

/**
 * \ingroup thesis
 * \brief Helper class that adds thesis internet routing protocol to nodes
 */

class ThesisInternetRoutingHelper2 : public Ipv6RoutingHelper
{
public:
	ThesisInternetRoutingHelper2();

	/**
	 * \returns pointer to clone of this ThesisInternetRoutingHelper2
	 *
	 * \internal
	 * This method is mainly for internal use by the other helpers;
	 * clients are expected to free the dynamic memory allocated by this method
	 */
	ThesisInternetRoutingHelper2* Copy (void) const;

	/**
	 * \param node the node on which the routing protocol will run
	 * \returns a newly-created routing protocol
	 *
	 * This method will be called by ns3::InternetStackHelper::Install
	 *
	 * \todo support installing AODV on the subset of all available IP interfaces
	 */
	virtual Ptr<Ipv6RoutingProtocol> Create (Ptr<Node> node) const;

	/**
	 * \param name the name of the attribute to set
	 * \param value the value of the attribute to set.
	 *
	 * This method controls the attributes of ns3::ThesisInternetRouting::RoutingProtocol
	 */
	void Set (std::string name, const AttributeValue &value);

	/**
	 * Assign a fixed random variable stream number to the random variables
	 * used by this model.  Return the number of streams (possibly zero) that
	 * have been assigned.  The Install() method of the InternetStackHelper
	 * should have previously been called by the user.
	 *
	 * \param stream first stream index to use
	 * \param c NodeContainer of the set of nodes for which MCAST
	 *          should be modified to use a fixed stream
	 * \return the number of stream indices assigned by this helper
	 */
//	int64_t AssignStreams (NodeContainer c, int64_t stream);

	/*
	 * Sets a pointer to a global database object
	 * Due to address assignment occuring after routing protocol
	 * the database will be empty at assignment.
	 * Driver program should later build the database; pointer should then
	 * point to the built up database.
	 *
	 */
	void SetRsuDatabase(Ptr<Db> database);

	/**
	 * Sets RSU status (Is RSU or is Mobile node)
	 */
	void SetIsRSU(bool isRSU);

private:

	ObjectFactory m_agentFactory;
	Ptr<Db> m_database;
	bool m_IsRSU;
};

}

#endif /* THESISINTERNETROUTING_HELPER_H */
