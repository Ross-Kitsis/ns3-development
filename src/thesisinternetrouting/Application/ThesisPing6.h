/*
 * ThesisPing6.h
 *
 *  Created on: Feb 22, 2016
 *      Author: ross
 */

#ifndef SRC_THESISINTERNETROUTING_APPLICATION_THESISPING6_H_
#define SRC_THESISINTERNETROUTING_APPLICATION_THESISPING6_H_

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv6-address.h"

namespace ns3
{

class Packet;
class Socket;

/**
 * \ingroup applications
 * \defgroup ThesisPing6 ThesisPing6
 */

/**
 * \ingroup ThesisPing6
 * \class ThesisPing6
 * \brief A ping6 application designed for the grip VANEt routing protocol
 */

class ThesisPing6 : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId ();

	ThesisPing6();
	virtual ~ThesisPing6();

  /**
   * \brief Set the local address.
   * \param ipv6 the local IPv6 address
   */
  void SetLocal (Ipv6Address ipv6);

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

  /**
   * \brief Set routers for routing type 0 (loose routing).
   * \param routers routers addresses
   */
  void SetRouters (std::vector<Ipv6Address> routers);

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
  void ScheduleTransmit (Time dt);

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
   * \brief Intervall between packets sent.
   */
  Time m_interval;

  /**
   * \brief Local address.
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
   * \brief Out interface (i.e. for link-local communication).
   */
  uint32_t m_ifIndex;

  /**
   * \brief Routers addresses for routing type 0.
   */
  std::vector<Ipv6Address> m_routers;

};

} /* namespace ns3 */

#endif /* SRC_THESISINTERNETROUTING_APPLICATION_THESISPING6_H_ */
