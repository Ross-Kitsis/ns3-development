/*
 * MobiCastApp.h
 *
 *  Created on: Jan 8, 2016
 *      Author: ross
 */

#ifndef SRC_MCAST_APPLICATION_MOBICASTAPP_H_
#define SRC_MCAST_APPLICATION_MOBICASTAPP_H_

#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv6-address.h"

#include "ns3/random-variable-stream.h"

//Mcast includes
#include "ns3/mcast2.h"


namespace ns3
{

class Packet;
class Socket;

/**
 * \brief Application which starts the mobicast process
 * Routing is assumed to be the Thesis routing protocol
 *
 */
class MobiCastApp : public Application
{
public:
	static TypeId GetTypeId();

	/*
	 * \brief Constructor
	 */
	MobiCastApp();

	/*
	 * \brief Destructor
	 */
	virtual ~MobiCastApp();

	/*
	 * \brief Set local address
	 */
	void SetLocal(Ipv6Address ipv6);


	/*
	 * \brief Set interval between transmissions
	 */
	void SetAttemptInterval(Time interval);

	/*
	 * \brief Set interval between successful transmissions
	 */
	void SetSuccessInterval(Time interval);

  /**
   * \brief Set the remote peer.
   * \param ipv6 IPv6 address of the peer
   */
  void SetRemote (Ipv6Address ipv6);

  /**
   * \brief Set the out interface index.
   * This is to send to link-local (unicast or multicast) address
   * when a node has multiple interfaces.
   * \param ifIndex interface index
   */
  void SetIfIndex (uint32_t ifIndex);

protected:
  /**
   * \brief Dispose this object;
   */
  virtual void DoDispose ();
private:
  /**
   * \brief Start the application.
   */
  virtual void StartApplication ();

  /**
   * \brief Stop the application.
   */
  virtual void StopApplication ();

  /**
   * \brief Schedule sending a packet.
   * \param dt interval between packet
   */
  void ScheduleTransmit ();

  /**
   * \brief Send a packet.
   */
  void Send ();

  /**
   * \brief Receive method.
   * \param socket socket that receive a packet
   */
  void HandleRead (Ptr<Socket> socket);

  /**
   * \brief Peer IPv6 address.
   */
  Ipv6Address m_address;

  /**
   * \brief Number of "Echo request" packets that will be sent.
   */
  uint32_t m_count;

  /**
   * \brief Number of packets sent.
   */
  uint32_t m_sent;

  /**
   * \brief Size of the packet.
   */
  uint32_t m_size;

  /**
   * \brief Interval between packets sent.
   *
   *  Can this be modified as minimum time?
   *
   */
  Time m_interval;

  /**
   * \brief Schedule interval
   *
   * Time between 2 consecutive safety messages
   */
  Time m_sendSafetyMessageInterval;

  /**
   * \brief Probability to trigger and event
   */
  double m_eventProbability;

  /**
   * \brief IPv6 protocol reference
   */
  Ptr<Ipv6> m_ipv6;

  /**
   */
  Ipv6Address m_localAddress;

  /**
   * \brief Peer address.
   */
  Ipv6Address m_peerAddress;

  /**
   * \brief Local socket.
   */
  Ptr<Socket> m_socket;

  /**
   * \brief Sequence number.
   */
  uint16_t m_seq;

  /**
   * \brief Event ID.
   */
  EventId m_sendEvent;

  /**
   * \brief Event ID.
   */
  EventId m_transmitEvent;


  /**
   * \brief Out interface (i.e. for link-local communication).
   */
  uint32_t m_ifIndex;

  /**
   * \brief Routers addresses for routing type 0.
   */
  std::vector<Ipv6Address> m_routers;

  /**
   * \brief Random variable steam; used to determine if send packet
   */
  Ptr<UniformRandomVariable> m_rng;

  /**
   * \brief Pointer to the Mcast routing protocol
   */
  mcast::ThesisRoutingProtocol m_routing;

  /**
   * \brief Random transmission mode
   *
   * 0 - Single theshold used by all nodes for all transmission decisions
   * 1 - Use random variable
   *
   */

};

} /* namespace ns3 */

#endif /* SRC_MCAST_APPLICATION_MOBICASTAPP_H_ */
