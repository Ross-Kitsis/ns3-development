/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 *			Based on AODV neighbor implementation provided in the NS-3 simulation models
 *
 * Authors: Ross Kitsis
 */

#ifndef MCASTNEIGHBOR_H
#define MCASTNEIGHBOR_H

#include "ns3/simulator.h"
#include "ns3/timer.h"
#include "ns3/ipv6-address.h"
#include "ns3/callback.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/ndisc-cache.h"
//#include <vector>
#include "ns3/vector.h"

namespace ns3
{
namespace mcast
{
//class RoutingProtocol;
/**
 * \ingroup mcast
 * \brief maintain list of active neighbors
 * Each neighbor entry contains the neighbor IP address, Mac Address, expire time of the nieghbor
 * close??? - always false, nieghbor velocity, neighbor position
 */



class Neighbors
{
public:
  /// c-tor
  Neighbors (Time delay);
  /// Neighbor description
  struct Neighbor
  {
    Ipv6Address m_neighborAddress;
    Mac48Address m_hardwareAddress;
    Time m_expireTime;
    bool close;
    Vector m_velocity;
    Vector m_position;


    Neighbor (Ipv6Address ip, Mac48Address mac, Time t, Vector v, Vector p) :
      m_neighborAddress (ip), m_hardwareAddress (mac), m_expireTime (t),
      close (false), m_velocity(v), m_position(p)
    {
    }


  };
  /// Return expire time for neighbor node with address addr, if exists, else return 0.
  Time GetExpireTime (Ipv6Address addr);
  /// Check that node with address addr  is neighbor
  bool IsNeighbor (Ipv6Address addr);
  /// Update expire time for entry with address addr, if it exists, else add new entry
  void Update (Ipv6Address addr, Time expire, Vector velocity, Vector position);

  /// Remove all expired entries
  void Purge ();
  /// Schedule m_ntimer.
  void ScheduleTimer ();
  /// Remove all entries
  void Clear () { m_nb.clear (); }

  /// Add ARP cache to be used to allow layer 2 notifications processing
  void AddArpCache (Ptr<NdiscCache>);
  /// Don't use given ARP cache any more (interface is down)
  void DelArpCache (Ptr<NdiscCache>);
  /// Get callback to ProcessTxError
  Callback<void, WifiMacHeader const &> GetTxErrorCallback () const { return m_txErrorCallback; }

  /// Handle link failure callback
  void SetCallback (Callback<void, Ipv6Address> cb) { m_handleLinkFailure = cb; }
  /// Handle link failure callback
  Callback<void, Ipv6Address> GetCallback () const { return m_handleLinkFailure; }

  /**
   * \brief Called when timer on a neighbor relationship expires
   * Should only be called when a neighbor relationship has not been renewed
   */
  void NeighborTimerExpire(void);

private:
  /// link failure callback
  Callback<void, Ipv6Address> m_handleLinkFailure;
  /// TX error callback
  Callback<void, WifiMacHeader const &> m_txErrorCallback;
  /// Timer for neighbor's list. Schedule Purge().
  Timer m_ntimer;
  /// vector of entries
  std::vector<Neighbor> m_nb;
  /// list of ARP cached to be used for layer 2 notifications processing
  std::vector<Ptr<NdiscCache> > m_arp;

  /// Find MAC address by IP using list of ARP caches
  Mac48Address LookupMacAddress (Ipv6Address);
  /// Process layer 2 TX error notification
  void ProcessTxError (WifiMacHeader const &);
};

} //MCAST
} //NS3


#endif /* MCASTNEIGHBOR_H */
