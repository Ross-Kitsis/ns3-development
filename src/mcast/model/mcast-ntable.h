/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Stores neighbors collected through broadcasts
 *
 * Based on
 * 		AODV routing table given in NS-3 source files
 * Authors: Ross Kitsis <rkitsis@cse.yorku.ca>
 *
 */

#ifndef MCAST_NTABLE_H_
#define MCAST_NTABLE_H_

#include <stdint.h>
#include <cassert>
#include <map>
#include <sys/types.h>

#include "ns3/ipv6.h"
//#include "ns3/ipv4-route.h" //Maybe include?
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3
{
	namespace mcast
	{
		class NeighborTableEntry
		{
		public:
			//Neighbor table entry consists of a pointer to the device, the neighbor address
			//The interface address and the time when the entry was created
			NeighborTableEntry(Ptr<NetDevice> dev = 0, Ipv6Address nAddress = Ipv6Address()
					, Ipv6InterfaceAddress iface = Ipv6InterfaceAddress(), Time lifetime = Simulator::Now());

			~NeighborTableEntry();
		private:
			/*
			 * Lifetime for a neighbor relationship,
			 * once expires (and not renewed) assume no longer a neighbor
			 */
			Time m_lifeTime;

		  /// Output interface address
		  Ipv6InterfaceAddress m_iface;

		};
		class NeighborTable
		{
		public:
			NeighborTable(Time t);

			//Add a new neighbor table entry
			bool AddNeighbor(NeighborTableEntry & n);

			//Delete a neighbor table entry
			//Return true if it exists
			bool DeleteNeighbor(Ipv6Address n);

			//Clear all entries from neighbor table
			void Clear(){m_ipv6AddressEntry.clear();}

			//Print the neighbor table
			void Print(Ptr<OutputStreamWrapper> stream) const;

		private:
			std::map<Ipv6Address,NeighborTableEntry> m_ipv6AddressEntry;
		};
	}
}

#endif /* SRC_MCAST_MODEL_MCAST_NTABLE_H_ */
