/*
 * GeoQueryClient.h
 *
 *  Created on: Apr 30, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_APPLICATION_GEOQUERYCLIENT_H_
#define SRC_THESISINTERNETROUTING_APPLICATION_GEOQUERYCLIENT_H_

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"

#include <cstdlib>
#include <ctime>

#include "ns3/Db.h"

namespace ns3
{

class Socket;
class Packet;

class GeoQueryClient : public Application
{
public:
	static TypeId GetTypeId (void);
	GeoQueryClient();
	virtual ~GeoQueryClient();

	void SetRsuDatabase(Ptr<Db> db);

protected:
	virtual void DoDispose (void);

private:

	virtual void StartApplication (void);
	virtual void StopApplication (void);

	/**
	 * \brief Schedule the next packet transmission
	 * \param dt time interval between packets.
	 */
	void ScheduleTransmit (Time dt);
	/**
	 * \brief Send a packet
	 */
	void Send (void);

	/**
	 * \brief Handle a packet reception.
	 *
	 * This function is called by lower layers.
	 *
	 * \param socket the socket the packet was received to.
	 */
	void HandleRead (Ptr<Socket> socket);

	uint32_t m_count; //!< Maximum number of packets the application will send
	Time m_interval; //!< Packet inter-send time
	uint32_t m_size; //!< Size of the sent packet


	uint32_t m_sent; //!< Counter for sent packets
	Ptr<Socket> m_socket; //!< Socket
	Address m_peerAddress; //!< Remote peer address
	uint16_t m_peerPort; //!< Remote peer port
	EventId m_sendEvent; //!< Event to send the next packet

	/// Callbacks for tracing the packet Tx events
	TracedCallback<Ptr<const Packet> > m_txTrace;

	//Source address used to bind socket
	Ipv6Address src;

	Ptr<Db> m_db;


	typedef struct Transmission
	{
		Ipv6Address Destination;
		Time SendTime;
	}Transmission;

	/// Container for the network routes - pair RipNgRoutingTableEntry *, EventId (update event)
	typedef std::list<Transmission> Transmissions;

	//Iterator for routes
	typedef std::list<Transmission>::iterator TransmissionsIt;

	/**
	 * List of transmissions used to keep track of receieved packets
	 */
	Transmissions m_sourcedTrans;

	int m_numReceived;

	Time m_RTT;

	int strLength;

};

} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_APPLICATION_GEOQUERYCLIENT_H_ */
