/*
 * Db.cc
 *
 *  Created on: Jan 25, 2016
 *      Author: ross
 */

#include "Db.h"

#include "ns3/wifi-module.h"
namespace ns3
{
//DB ENTRY ///////////////////////////////

DbEntry::DbEntry()
{

}

DbEntry::DbEntry(Vector bottomCorner, Vector topCorner, Ipv6Address RsuAddress, Vector RsuPosition, uint32_t ZoneId, Mac48Address RsuMacAddress) :
		m_bottomLeftCorner(bottomCorner), m_topRightCorner(topCorner),
		m_RsuAddress(RsuAddress),m_RsuPosition(RsuPosition),m_ZoneId(ZoneId), m_RsuMacAddress(RsuMacAddress)
{

}

DbEntry::~DbEntry()
{

}

void
DbEntry::SetBottomCorner(Vector corner)
{
	m_bottomLeftCorner = corner;
}

Vector
DbEntry::GetBottomCorner()
{
	return m_bottomLeftCorner;
}

void
DbEntry::SetTopCorner(Vector corner)
{
	m_topRightCorner = corner;
}

Vector
DbEntry::GetTopCorner()
{
	return m_topRightCorner;
}

void
DbEntry::SetRsuAddress(Ipv6Address address)
{
	m_RsuAddress = address;
}

Ipv6Address
DbEntry::GetRsuAddress()
{
	return m_RsuAddress;
}

void
DbEntry::SetRsuPosition(Vector position)
{
	m_RsuPosition = position;
}

Vector
DbEntry::GetRsuPosition()
{
	return m_RsuPosition;
}

void
DbEntry::SetZoneId(uint32_t id)
{
	m_ZoneId = id;
}

uint32_t
DbEntry::GetZoneId()
{
	return m_ZoneId;
}

void
DbEntry::SetRsuMacAddress(Mac48Address mac)
{
	m_RsuMacAddress = mac;
}

Mac48Address
DbEntry::GetRsuMacAddress()
{
	return m_RsuMacAddress;
}

Db::Db()
{


}

Db::~Db()
{

}

void
Db::CreateDatabase(NodeContainer c, uint32_t length, uint32_t width)
{
	uint32_t zone = 0;

	for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
	{
		Ptr<Node> node = *i;
    Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();

		Vector position = ipv6->GetObject<MobilityModel>()->GetPosition();

		Vector bottomCorner(position.x - length/2,position.y - width/2,0);
		Vector topCorner(position.x + length/2,position.y + width/2,0);

		Ipv6Address RsuAddress;
		Mac48Address RsuMacAddress;

		WifiNetDevice wi;

		for(uint32_t j = 0; j < ipv6 -> GetNInterfaces(); j++)
		{
			if(ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(wi.GetTypeId().GetName()) == 0)
			{
				//Found wifiNetDevice
				RsuAddress = ipv6 ->GetAddress(j,1).GetAddress();
				Ptr<WifiNetDevice> wifi = DynamicCast<WifiNetDevice>(ipv6 ->GetNetDevice(j));
				RsuMacAddress = wifi -> GetMac() -> GetAddress();
			}
		}

		DbEntry * entry = new DbEntry(bottomCorner, topCorner, RsuAddress, position,zone, RsuMacAddress);

		zone++;
		m_db.push_back(*entry);
	}
}

int
Db::GetNumEntry()
{
	return m_db.size();
}

DbEntry
Db::GetEntry(int i)
{
	DbEntry * toReturn;
	int count = 0;
	for(std::list<DbEntry>::iterator it = m_db.begin(); it!= m_db.end(); ++it)
	{
		if(count == i)
		{
			toReturn = &*it;
			break;
		}else
		{
			count++;
		}
	}
	return *toReturn;
}

DbEntry
Db::GetEntryForCurrentPosition(Vector position)
{
	//DbEntry(Vector bottomCorner, Vector topCorner, Ipv6Address RsuAddress, Vector RsuPosition, uint32_t ZoneId);
	DbEntry * toReturn;
	for(std::list<DbEntry>::iterator it = m_db.begin(); it!= m_db.end(); ++it)
	{
		double totalArea = 0;

		Vector bottomLeft = it -> GetBottomCorner();
		Vector topRight =  it -> GetTopCorner();
		Vector topLeft(bottomLeft.x, topRight.y,0);
		Vector bottomRight(topRight.x ,bottomLeft.y,0);

		//Calculate area of triangles
		totalArea = totalArea + GetAreaOfTriangle(bottomLeft,position,topLeft);
		totalArea = totalArea + GetAreaOfTriangle(topLeft, position, topRight);
		totalArea = totalArea + GetAreaOfTriangle(topRight, position, bottomRight);
		totalArea = totalArea + GetAreaOfTriangle(bottomRight,position,bottomLeft);

		//Calculate area of the rectangle as a whole
		double rectangleArea = std::abs((bottomLeft.x - topRight.x) * (bottomLeft.y - topRight.y));

//	  std::cout << "Total area: " << totalArea << std::endl;
//	  std::cout << "Rectangle area: " << rectangleArea << std::endl;

		rectangleArea = round(rectangleArea);
		totalArea = round(totalArea);

		if(totalArea == rectangleArea)
		{
			toReturn = &*it;
			break;
		}
	}


	return *toReturn;
}

double
Db::GetAreaOfTriangle(Vector A, Vector B, Vector C)
{
	double area;

	area = std::abs((A.x *(B.y - C.y) + B.x *(C.y-A.y) + C.x * (A.y - B.y))/2);

//  std::cout << "Triangle area: " << area << std::endl;

	return area;
}

} /* namespace ns3 */
