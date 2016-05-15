/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#define NS_LOG_APPEND_CONTEXT                                   \
		if (GetObject<Node> ()) { std::clog << "[node " << GetObject<Node> ()->GetId () << "] "; }


#include "thesisinternetrouting2.h"
#include "ns3/ipv6-route.h"

#include <iomanip>
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"


#define VANET_TO_RSU "ff02::116"
#define RSU_TO_VANET "ff02::117"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThesisInternetRoutingProtocol2");

namespace thesis
{

class DeferredRouteOutputTag : public Tag
{
public:
	DeferredRouteOutputTag (int32_t o = -1) : Tag (), m_oif (o) {}

	static TypeId GetTypeId ()
	{
		static TypeId tid = TypeId ("ns3::thesis::DeferredRouteOutputTag")
      		.SetParent<Tag> ()
      		.SetGroupName("thesis")
      		.AddConstructor<DeferredRouteOutputTag> ()

      		;
		return tid;
	}

	TypeId  GetInstanceTypeId () const
	{
		return GetTypeId ();
	}

	int32_t GetInterface() const
	{
		return m_oif;
	}

	void SetInterface(int32_t oif)
	{
		m_oif = oif;
	}

	uint32_t GetSerializedSize () const
	{
		return sizeof(int32_t);
	}

	void  Serialize (TagBuffer i) const
	{
		i.WriteU32 (m_oif);
	}

	void  Deserialize (TagBuffer i)
	{
		m_oif = i.ReadU32 ();
	}

	void  Print (std::ostream &os) const
	{
		os << "DeferredRouteOutputTag: output interface = " << m_oif;
	}

private:
	/// Positive if output device is fixed in RouteOutput
	int32_t m_oif;
};

NS_OBJECT_ENSURE_REGISTERED (DeferredRouteOutputTag);




NS_OBJECT_ENSURE_REGISTERED(ThesisInternetRoutingProtocol2);

//Constructor
ThesisInternetRoutingProtocol2::ThesisInternetRoutingProtocol2() :
								m_hasMcast(true), m_IsRSU(false),
								m_CheckPosition(Seconds(10)), m_IsDtnTolerant(false),
								m_isStrictEffective(true),m_rWait(10),m_ThesisInternetRoutingCacheCooldown(Seconds(1)),
								m_hopCountLimit(10)
{
	m_numSourced = 0;
	m_numReceived = 0;
	m_AverageLatency = 0;
	m_receiveRate = 0;
	m_HopCountAgregatorVanetToRsu = 0;
	m_HopCountAgregatorRsuToVanet = 0;
	m_numRsuRec = 0;
	m_activeLimit = 30;
	m_GeoServerPort = 123;
	//m_ThesisInternetRoutingCacheCooldown = Seconds(5);
	//m_hopCountLimit = 10;
}

//Destructor
ThesisInternetRoutingProtocol2::~ThesisInternetRoutingProtocol2()
{

}

TypeId
ThesisInternetRoutingProtocol2::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::thesis::ThesisInternetRoutingProtocol2")
    						.SetParent<Ipv6RoutingProtocol> ()
    						.SetGroupName ("thesis")
    						.AddConstructor<ThesisInternetRoutingProtocol2> ()
    						.AddAttribute ("McastEnabled", "Determines if MCast protocol is running on the node (Default false)",
    								BooleanValue (false),
    								MakeBooleanAccessor (&ThesisInternetRoutingProtocol2::m_hasMcast),
    								MakeBooleanChecker ())
    								.AddAttribute ("StrictlyEffective", "Determines if VANET nodes follow a strictly effective forwarding pattern",
    										BooleanValue (true),
    										MakeBooleanAccessor (&ThesisInternetRoutingProtocol2::m_isStrictEffective),
    										MakeTimeChecker ())
    										.AddAttribute ("rWait", "Multiplier to wait time",
    												IntegerValue (5),
    												MakeIntegerAccessor (&ThesisInternetRoutingProtocol2::m_rWait),
    												MakeUintegerChecker<uint64_t>())
    												.AddAttribute("ThesisRoutingCacheCooldown", "Time to remove cache entry after decision has been made",
    														TimeValue(Seconds(5)),
    														MakeTimeAccessor(&ThesisInternetRoutingProtocol2::m_ThesisInternetRoutingCacheCooldown),
    														MakeTimeChecker())
    														.AddAttribute("HopCountLimit","Number of hops until a packet is dropped",
    																IntegerValue (5),
    																MakeIntegerAccessor(&ThesisInternetRoutingProtocol2::m_hopCountLimit),
    																MakeUintegerChecker<uint8_t>())
    																;
	return tid;
}

bool
ThesisInternetRoutingProtocol2::RouteInput (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION (this << p << header << header.GetSourceAddress () << header.GetDestinationAddress () << idev);

	//	std::cout << "ROUTE INPUT" << std::endl;

	if(m_IsRSU)
	{
		//std::cout << "ThesisInternet2 route input RSU" << std::endl;
		RouteInputRsu(p, header,idev,ucb,mcb,lcb,ecb);
	}else
	{
		//std::cout << "ThesisInternet2 route input VANET" << std::endl;
		RouteInputVanet(p, header,idev,ucb,mcb,lcb,ecb);
	}
	return true;
}

bool
ThesisInternetRoutingProtocol2::RouteInputRsu (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this << header);

	//Copy passed packet to a new packet
	Ptr<Packet> packet = p -> Copy();

	if(idev == m_lo)
	{
		//std::cout << "RSU RECV ON LOOPBACK <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		//Received on loopback (Will be done later to handle sending into the network
	}else if(idev == m_wi)
	{
		//		std::cout << "RSU RECV ON WIFI <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		//Received on wifi interface (Must be from VANET)
		//Need to send an ACK msg, add node information to a DB and forward through the internet (OR accept itself)

		//If receiving on wifi then packets should have tags

		//p ->Print(std::cout);

		//Create new typeHeader and peek
		mcast::TypeHeader theader (mcast::UNKNOWN);
		packet -> PeekHeader(theader);
		if(theader.Get() == mcast::INTERNET)
		{
			//Received type header of type internet
			packet -> RemoveHeader(theader);
			//			std::cout << "Type header with type: " << theader.Get() << std::cout;

			InternetHeader Ih;
			packet -> RemoveHeader(Ih);

			int interface = m_ipv6 -> GetInterfaceForDevice(m_wi);
			Ipv6Address currentAd = m_ipv6->GetAddress(interface,1).GetAddress();

			//Check is this is the RSU the node originally sent the msg towards
			if(!(currentAd.IsEqual(Ih.GetRsuAddress())))
			{
				//				std::cout << " CURRENT RSU ADDRESS " << currentAd << " NOT EQUAL TO HEADER ADDRESS: "<< Ih.GetRsuAddress() << std::endl;
				return true;
			}

			//			std::cout << "  Internet Header: " << Ih << std::endl;

			//Check if cache already contains an entry with the unique source,destination,sendtime tuple
			//If contained this indicates the RSU has already received this transmission
			//Do not forward any further as the packet was already forwarded
			if(m_RsuCache.ContainsEntry(header.GetSourceAddress(),header.GetDestinationAddress(), Ih.GetTimestamp()))
			{
				return true;
			}//Else fall through for further processing


			Ipv6RoutingTableEntry toHub = m_sr6 -> GetDefaultRoute();
			Ptr<Ipv6Route> route = Create<Ipv6Route> ();
			route -> SetDestination(toHub.GetDestNetwork());
			route -> SetGateway(toHub.GetGateway());
			route -> SetOutputDevice(m_ipv6 -> GetNetDevice(toHub.GetInterface()));
			route -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());

			//////////////////////////////////////////// Statistics

			//Add the difference between the maximum hop count (64) and to hops to get to the RSU
			m_HopCountAgregatorVanetToRsu += (64 - header.GetHopLimit());
			m_numRsuRec++;
			//std::cout << "CURRENT VANETTORSU Agregate" << m_HopCountAgregatorVanetToRsu <<std::endl;
			//std::cout << "GOT HERE _______________________________" << 64 - header.GetHopLimit() <<std::endl;
			////////////////////////////////////////////


			ucb (route -> GetOutputDevice(),route, packet, header);

			//Add VANET information to the RSU cache
			RsuCacheEntry *entry = new RsuCacheEntry(header.GetSourceAddress(),header.GetDestinationAddress(),
					Ih.GetSourcePosition(), Ih.GetSourceVelocity(),
					Ih.GetTimestamp(), Simulator::Now());
			m_RsuCache.AddEntry(entry);
			/*
			std::cout << " Adding entry to cache with properties: " << std::endl;
			std::cout << " Destination: " << entry -> GetDestination() << std::endl;
			std::cout << " Source: " << entry -> GetSource() << std::endl;
			std::cout << "Send Time: " << entry -> GetSendTime() << std::endl;
			std::cout << "Receive Time: " << entry -> GetReceiveTime() << std::endl;
			std::cout << "Sending node position " << entry -> GetSendingNodePosition() << std::endl;
			std::cout << "Sending node velocity " << entry -> GetSendingNodeVelocity() << std::endl;
			 */
			/*
			std::cout << " RSU Header Properties " << header.GetDestinationAddress() << std::endl;

			std::cout << " Header Destination " << header.GetDestinationAddress() << std::endl;
			std::cout << " Header  " << header.GetSourceAddress() << std::endl;

			std::cout << "  >>>>>>>>>>>>> Route to Hub: <<<<<<<<<<<<<<<<< " << std::endl;
			std::cout << " Dest " << toHub.GetDest() << std::endl;
			std::cout << " Destination " << toHub.GetDestNetwork() << std::endl;
			std::cout << " Gateway " << toHub.GetGateway() << std::endl;
			std::cout << " Interface " << toHub.GetInterface() << std::endl;
			 */


			//Create new ack packet
			//Ensure size < 4 for classification
			Ptr<Packet> ack = Create<Packet> ();

			ack -> AddHeader(Ih);

			mcast::TypeHeader ackheader (mcast::INTERNET_RSU_ACK);
			ack -> AddHeader(ackheader);

			Ptr<Ipv6Route> ackRoute= Create<Ipv6Route> ();
			//ackRoute = Lookup(Ipv6Address(VANET_TO_RSU),m_wi);
			/*
			std::cout << std::endl;
			std::cout << std::endl;
			std::cout << std::endl;
			std::cout << std::endl;
			std::cout << " >>>>>>> OUTPUT ADDRESS FOR ACK: " << m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress() << std::endl;
			 */
			Ptr<Ipv6L3Protocol> l3 = GetObject<Ipv6L3Protocol>();

			//			Ipv6Header ackHdr;
			//			ackHdr.SetSourceAddress(m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress());
			//			ackHdr.SetDestinationAddress(Ipv6Address(RSU_TO_VANET));
			//			ackHdr.SetNextHeader(1);
			//			ackHdr.SetHopLimit(2);
			//			ackHdr.SetPayloadLength(ack -> GetSize());
			//			ackHdr.SetTrafficClass(1);

			/*
						 Ipv6Header hdr;
						 1445
						 1446   hdr.SetSourceAddress (src);
						 1447   hdr.SetDestinationAddress (dst);
						 1448   hdr.SetNextHeader (protocol);
						 1449   hdr.SetPayloadLength (payloadSize);
						 1450   hdr.SetHopLimit (ttl);
						 1451   hdr.SetTrafficClass (tclass);
						 1452   return hdr;
			 */

			ackRoute -> SetDestination(Ipv6Address(VANET_TO_RSU));
			ackRoute -> SetGateway(Ipv6Address(VANET_TO_RSU));
			ackRoute -> SetOutputDevice(m_wi);
			ackRoute -> SetSource(m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress());

			//	l3 ->Send(ack,ackHdr.GetSourceAddress(), ackHdr.GetDestinationAddress(),1,ackRoute);

			//			ucb(m_wi,ackRoute,ack,ackHdr);
			ucb(m_wi,ackRoute,ack,header);

		}else if(theader.Get() == mcast::GEOQUERY_REQUEST)
		{
			//Type 8: GeoRequest, continue forwarding to other RSU
			//Create new typeHeader and peek
			mcast::TypeHeader theader (mcast::UNKNOWN);
			packet -> PeekHeader(theader);
			if(theader.Get() == mcast::GEOQUERY_REQUEST)
			{
				//Received type header of type internet
				packet -> RemoveHeader(theader);

				GeoRequestHeader Gh;
				packet -> RemoveHeader(Gh);

				int interface = m_ipv6 -> GetInterfaceForDevice(m_wi);
				Ipv6Address currentAd = m_ipv6->GetAddress(interface,1).GetAddress();

				//Check is this is the RSU the node originally sent the msg towards
				if(!(currentAd.IsEqual(Gh.GetRsuAddress())))
				{
					return true;
				}

				//Check if cache already contains an entry with the unique source,destination,sendtime tuple
				//If contained this indicates the RSU has already received this transmission
				//Do not forward any further as the packet was already forwarded
				if(m_RsuCache.ContainsEntry(header.GetSourceAddress(),header.GetDestinationAddress(), Gh.GetTimestamp()))
				{
					return true;
				}//Else fall through for further processing

				Ipv6RoutingTableEntry toHub = m_sr6 -> GetDefaultRoute();
				Ptr<Ipv6Route> route = Create<Ipv6Route> ();
				route -> SetDestination(toHub.GetDestNetwork());
				route -> SetGateway(toHub.GetGateway());
				route -> SetOutputDevice(m_ipv6 -> GetNetDevice(toHub.GetInterface()));
				route -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());

				//////////////////////////////////////////// Statistics

				//Add the difference between the maximum hop count (64) and to hops to get to the RSU
				m_HopCountAgregatorVanetToRsu += (64 - header.GetHopLimit());
				m_numRsuRec++;
				//std::cout << "CURRENT VANETTORSU Agregate" << m_HopCountAgregatorVanetToRsu <<std::endl;
				//std::cout << "GOT HERE _______________________________" << 64 - header.GetHopLimit() <<std::endl;
				////////////////////////////////////////////

				//Add headers to give info to receiving RSU
				packet -> AddHeader(Gh);
				packet -> AddHeader(theader);

				ucb (route -> GetOutputDevice(),route, packet, header);

				//Add entry into cache
				//Add VANET information to the RSU cache
				RsuCacheEntry *entry = new RsuCacheEntry(header.GetSourceAddress(),header.GetDestinationAddress(),
						Gh.GetSourcePosition(), Gh.GetSourceVelocity(),
						Gh.GetTimestamp(), Simulator::Now());
				m_RsuCache.AddEntry(entry);


				//Send ACK to VANET nodes
				Ptr<Packet> ack = Create<Packet> ();

				ack -> AddHeader(Gh);

				mcast::TypeHeader ackheader (mcast::GEOQUERY_RSU_ACK);
				ack -> AddHeader(ackheader);

				Ptr<Ipv6Route> ackRoute= Create<Ipv6Route> ();

				ackRoute -> SetDestination(Ipv6Address(VANET_TO_RSU));
				ackRoute -> SetGateway(Ipv6Address(VANET_TO_RSU));
				ackRoute -> SetOutputDevice(m_wi);
				ackRoute -> SetSource(m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress());

				//	l3 ->Send(ack,ackHdr.GetSourceAddress(), ackHdr.GetDestinationAddress(),1,ackRoute);

				//			ucb(m_wi,ackRoute,ack,ackHdr);
				ucb(m_wi,ackRoute,ack,header);
			}

		}else if(theader.Get() == mcast::INTERNET_RSU_TO_VANET)
		{
			//Type 4: RSU transmitting into VANET or VANET retransmitting
			//Do not react to this transmission if it happens
			return true;
		}else if(theader.Get() == mcast::INTERNET_VANET_ACK)
		{
			//Type 6 message - VANET node ACK recieving a packet
			//Don't need to do any processing on this end, allow entry in queue to naturally expire
			return true;
		}else if(theader.Get() == mcast::HELLO)
		{
			//Type 1 message - VANET node sending mcast hello msg
			//Don't need to react to this msg
			return true;
		}else if(theader.Get() == mcast::MCAST_CONTROL)
		{
			//Type 2 message - Rsu received mcast control packet
			//Don't need to react to this
			return true;
		}else if(theader.Get() == mcast::GEOQUERY_SENDINGREPLY)
		{
			NS_LOG_INFO("RSU RECV GEOQUERY_SENDINGREPLY ACK Msg");

			//Send an ACK into VANET
			//Remove headers from the ACK packet to add to a smaller packet
			Ptr<Packet> ack = Create<Packet> ();

			//Remove headers from sent packet and attach
			mcast::TypeHeader ackheader (mcast::GEOQUERY_SENDINGREPLY);
			packet -> RemoveHeader(ackheader);

			GeoRequestHeader Gh;
			packet -> RemoveHeader(Gh);

			//Add to ack packet
			ack -> AddHeader(Gh);
			ack -> AddHeader(ackheader);

			Ptr<Ipv6Route> ackRoute= Create<Ipv6Route> ();

			ackRoute -> SetDestination(Ipv6Address(RSU_TO_VANET));
			ackRoute -> SetGateway(Ipv6Address(RSU_TO_VANET));
			ackRoute -> SetOutputDevice(m_wi);
			ackRoute -> SetSource(m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress());

			ucb(m_wi,ackRoute,ack,header);

			return true;
		}else if(theader.Get() == mcast::GEOQUERY_REPLY)
		{
			NS_LOG_INFO("RSU RECV GEOREPLY");

			//RSU Got a geo reply packet
			/*
			 * 1. Destination in current zone: drop and do nothing
			 * 2. Destination in different zone: forward
			 *
			 */
			Ipv6Address currentAdd = m_ipv6 -> GetAddress(m_ipv6->GetInterfaceForDevice(m_wi),1).GetAddress();
			Ipv6Address destination = header.GetDestinationAddress();

			uint8_t currentAddBytes[16];
			currentAdd.GetBytes(currentAddBytes);

			uint8_t destAddBytes[16];
			destination.GetBytes(destAddBytes);

			//std::cout << "-----------------" << std::endl;
			//std::cout << "Destination: " << destination << std::endl;
			//std::cout << "Current : " << currentAdd << std::endl;

			bool currentZone = true;
			for(int i = 0; i < 8; i++)
			{
				if(currentAddBytes[i] != destAddBytes[i])
				{
					currentZone = false;
				}
			}

			if(currentZone == false)
			{
				//Current zone = false means need to forward to the RSU for that zone

				Ipv6RoutingTableEntry toHub = m_sr6 -> GetDefaultRoute();
				Ptr<Ipv6Route> route = Create<Ipv6Route> ();
				route -> SetDestination(toHub.GetDestNetwork());
				route -> SetGateway(toHub.GetGateway());
				route -> SetOutputDevice(m_ipv6 -> GetNetDevice(toHub.GetInterface()));
				route -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());

				ucb (route -> GetOutputDevice(),route, packet, header);

			}else
			{
				//Current zone = true means need to modify header and route into VANET

				//Should only happen when node retransmits a packet, dont need to do anything here
				//NS_LOG_INFO("RSU FOR QUERY ZONE RECV REPLY");

			}

			return true;
		}
		return true;
	}else if(idev == m_pp)
	{
		//		std::cout << "RSU RECV ON P2P <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		//Received on P2P interface; packet must have come from internet
		//Check header to see destination; look up destination in DB
		//Forward to other RSU if required or send out of wifi interface node is in the current area. (To be done later)

		//Create new typeHeader and peek
		mcast::TypeHeader theader (mcast::UNKNOWN);
		packet -> PeekHeader(theader);
		if(theader.Get() == mcast::INTERNET_RSU_ACK)
		{
			//Shouldn't be possible
			return true;
		}else if(theader.Get() == mcast::HELLO)
		{
			//Shouldn't happen but if it does kill the transmission here before it goes into the VANET
			return true;
		}else if(theader.Get() == mcast::INTERNET_RSU_TO_RSU_REDIRECT)
		{
			NS_LOG_INFO("Received RSU_TO_RSU_REDIRECT");
			//Got a packet from another RSU to forward onto VANET
			//Remove headers, update ITVHeader, change type header, form packet and send into VANET
			//VANET nodes should be oblivious of this redirect

			mcast::TypeHeader redirectHeader (mcast::UNKNOWN);
			packet -> RemoveHeader(redirectHeader);

			ITVHeader itvhdr(Vector(), Simulator::Now(), false, Vector(), Vector(), Vector(),0);
			packet -> RemoveHeader(itvhdr);

			Ptr<Node> theNode = GetObject<Node>();
			Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

			itvhdr.SetOriginPosition(mobility -> GetPosition());
			itvhdr.SetSenderPosition(mobility -> GetPosition());
			itvhdr.SetSenderVelocity(mobility -> GetVelocity());

			/////////////////////////////////////////////////////////////////////////////
			Ptr<Ipv6Route> route = Lookup(header.GetDestinationAddress(),m_pp);
			route -> SetGateway(Ipv6Address(RSU_TO_VANET));
			/////////////////////////////////////////////////////////////////////////////

			//Re-form packet by adding headers
			mcast::TypeHeader vanetHeader(mcast::INTERNET_RSU_TO_VANET);

			packet -> AddHeader(itvhdr);
			packet -> AddHeader(vanetHeader);

			ucb (route -> GetOutputDevice(),route, packet, header);
			return true;
		}else if(theader.Get() == mcast::GEOQUERY_REQUEST)
		{
			NS_LOG_INFO("Recv GeoQuery request of P2P link");

			//Recieved geo request on P2P link, route it into the VANET
			//No changes to the headers are the moment

			Ptr<Ipv6Route> route = Lookup(header.GetDestinationAddress(),m_wi);
			route -> SetGateway(Ipv6Address(RSU_TO_VANET));
			/////////////////////////////////////////////////////////////////////////////

			ucb (route -> GetOutputDevice(),route, packet, header);
			return true;
		}else if(theader.Get() == mcast::GEOQUERY_REPLY)
		{
			NS_LOG_INFO("Recv GeoQueryREPLY RECIEVED on CORRECT ZONE P2P link");

			mcast::TypeHeader theader (mcast::UNKNOWN);
			packet -> RemoveHeader(theader);

			GeoReplyHeader GR;
			packet -> RemoveHeader(GR);

			RsuCacheEntry entry;
			if(m_RsuCache.Lookup(header.GetDestinationAddress(), entry))
			{
				Ipv6Address destination = header.GetDestinationAddress();

				Ptr<Ipv6Route> route = Lookup(destination,m_wi);
				if(route)
				{
					//Get Mobility
					Ptr<Node> theNode = GetObject<Node>();
					Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

					//Set gateway
					route -> SetGateway(Ipv6Address(RSU_TO_VANET));

					Vector predictedPosition = GetPredictedNodePosition(header.GetDestinationAddress(), entry.GetSendingNodePosition(),
							entry.GetSendingNodeVelocity(),entry.GetSendTime());

					DbEntry t1 = m_Db -> GetEntryForCurrentPosition(predictedPosition);

					int intCheck = m_ipv6 ->GetInterfaceForAddress(t1.GetRsuAddress());

					if(intCheck != -1)
					{
						//Interface for address was found meaning this is the RSU for the zone, forward normally
						//					std::cout << "Predicted position: " << predictedPosition << std::endl;
						GeoReplyHeader GH(mobility -> GetPosition(), Simulator::Now(), false, mobility -> GetPosition(),mobility -> GetVelocity(),predictedPosition,0);
						packet -> AddHeader(GH);

						//Create type header add to packet
						mcast::TypeHeader RToVheader (mcast::GEOQUERY_REPLY);
						packet -> AddHeader(RToVheader);

						ucb (route -> GetOutputDevice(),route, packet, header);
						return true;
					}else
					{
						NS_LOG_INFO("GeoQuery Reply, node in a different zone, redirect");


//						std::cout << "GeoQuery Node predicted in another zone" << std::endl;
//						std::cout << "Destination Address: " << destination << std::endl;
//						std::cout << "Source: " << header.GetSourceAddress() << std::endl;
//						std::cout << "Predicted network: " << t1.GetRsuAddress() << std::endl;

						//Predicted position in another zone

						//Get bytes for destination address
						uint8_t addressBuf[16];
						destination.GetBytes(addressBuf);

						//Get bytes for RSU address
						uint8_t rsuAddressBuf[16];
						t1.GetRsuAddress().GetBytes(rsuAddressBuf);

						//Buffer to hold newly formed address
						uint8_t newDestinationAddressBuffer[16];

						//Set Network address bits
						for(int i = 0; i < 8; i++)
						{
							newDestinationAddressBuffer[i] = rsuAddressBuf[i];
						}

						//Set Host address bits
						for(int i = 8; i < 16; i++)
						{
							newDestinationAddressBuffer[i] = addressBuf[i];
						}

						Ipv6Address newDestinationAddress;
						newDestinationAddress.Set(newDestinationAddressBuffer);

						//Create new header with the updated location
						Ipv6Header newV6Header;
						newV6Header.SetSourceAddress(header.GetSourceAddress());
						newV6Header.SetDestinationAddress(newDestinationAddress);
						newV6Header.SetNextHeader(header.GetNextHeader());
						newV6Header.SetHopLimit(header.GetHopLimit());
						newV6Header.SetPayloadLength(header.GetPayloadLength());
						newV6Header.SetTrafficClass(newV6Header.GetTrafficClass());
						newV6Header.SetFlowLabel(header.GetFlowLabel());

						//Create a new type header
						mcast::TypeHeader redirectHeader(mcast::GEOREPLY_RSU_REDIRECT);

						//Create new ITV header
						GeoReplyHeader Gr(mobility -> GetPosition(), Simulator::Now(), false, mobility -> GetPosition(),mobility -> GetVelocity(),predictedPosition,0);

						//Get route to the new RSU
						//						Ptr<Ipv6Route> redirectRoute = Lookup(newDestinationAddress,m_pp);

						Ipv6RoutingTableEntry sr = m_sr6 -> GetDefaultRoute();
						Ptr<Ipv6Route> redirectRoute = Create<Ipv6Route> ();
						redirectRoute -> SetDestination(sr.GetDestNetwork());
						redirectRoute -> SetGateway(sr.GetGateway());
						redirectRoute -> SetOutputDevice(m_ipv6 -> GetNetDevice(sr.GetInterface()));
						redirectRoute -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());


						//Form packet
						packet -> AddHeader(Gr);
						packet -> AddHeader(redirectHeader);

						ucb (m_pp,redirectRoute, packet, newV6Header);


					}

				}
			}

			return true;
		}else if(theader.Get() == mcast::GEOREPLY_VANET_ACK)
		{
			NS_LOG_INFO("Recv GEOREPLY ACK from VANET");

			//Dont need to do anything here

			return true;
		}else if(theader.Get() == mcast::GEOREPLY_RSU_REDIRECT)
		{
			NS_LOG_INFO("RSU REDV GeoReply redirect");

			mcast::TypeHeader redirectHeader (mcast::UNKNOWN);
			packet -> RemoveHeader(redirectHeader);

			GeoReplyHeader Gh;
			packet -> RemoveHeader(Gh);

			Ptr<Node> theNode = GetObject<Node>();
			Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

			Gh.SetOriginPosition(mobility -> GetPosition());
			Gh.SetSenderPosition(mobility -> GetPosition());
			Gh.SetSenderVelocity(mobility -> GetVelocity());

			/////////////////////////////////////////////////////////////////////////////
			Ptr<Ipv6Route> route = Lookup(header.GetDestinationAddress(),m_wi);
			route -> SetGateway(Ipv6Address(RSU_TO_VANET));
			/////////////////////////////////////////////////////////////////////////////

			//Re-form packet by adding headers
			mcast::TypeHeader replyHeader(mcast::GEOQUERY_REPLY);

			packet -> AddHeader(Gh);
			packet -> AddHeader(replyHeader);

			ucb (route -> GetOutputDevice(),route, packet, header);
			return true;
		}
		else
		{
			NS_LOG_INFO("RSU attempting to reply into VANET");
//			std::cout << "GOT HERE IN GEO TRANSMIT TEST" << std::endl;

			RsuCacheEntry entry;
			if(m_RsuCache.Lookup(header.GetDestinationAddress(), entry))
			{
				//std::cout << "<<<<<<<<<<<< Found an entry in RSU cache >>>>>>>>>>>>>>>>" << std::endl;

				//Packet from a normal internet source
				Ipv6Address destination = header.GetDestinationAddress();

				Ptr<Ipv6Route> route = Lookup(destination,m_pp);
				if(route)
				{

					Ptr<Node> theNode = GetObject<Node>();
					Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

					/*
					Ptr<Ipv6Route> rtentry = Create<Ipv6Route> ();
					rtentry -> SetDestination(destination);
					rtentry -> SetGateway(Ipv6Address::GetLoopback());
					rtentry -> SetOutputDevice(m_lo);
					rtentry -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());
					 */



					//Set gateway to RSU TO VANET multicast address to avoid ARP
					route -> SetGateway(Ipv6Address(RSU_TO_VANET));

					//					std::cout << "Attempting to predict position" << std::endl;

					//Create ITV header and attach to packed
					//					std::cout << "Destination: " << header.GetDestinationAddress() << std::endl;
					//					std::cout << "Sending Node Position: " << entry.GetSendingNodePosition() << std::endl;
					//					std::cout << "Sending Node Velocity: " << entry.GetSendingNodeVelocity() << std::endl;
					//					std::cout << "Sending Node Time: " << entry.GetSendTime() << std::endl;


					Vector predictedPosition = GetPredictedNodePosition(header.GetDestinationAddress(), entry.GetSendingNodePosition(),
							entry.GetSendingNodeVelocity(),entry.GetSendTime());

					DbEntry t1 = m_Db -> GetEntryForCurrentPosition(predictedPosition);

					//if(!t1)
					//{
					//	std::cout << "COULD NOT GET ZONE FOR NODE POSITION" << std::endl;
					//	return true;
					//}


					int intCheck = m_ipv6 ->GetInterfaceForAddress(t1.GetRsuAddress());

					if(intCheck != -1)
					{
						//Interface for address was found meaning this is the RSU for the zone, forward normally
						//					std::cout << "Predicted position: " << predictedPosition << std::endl;
						ITVHeader itvh(mobility -> GetPosition(), entry.GetSendTime(), false, mobility -> GetPosition(),mobility -> GetVelocity(),predictedPosition,0);
						packet -> AddHeader(itvh);

						//Create type header add to packet
						mcast::TypeHeader RToVheader (mcast::INTERNET_RSU_TO_VANET);
						packet -> AddHeader(RToVheader);

						ucb (route -> GetOutputDevice(),route, packet, header);
						return true;
					}else
					{
						//Predicted position puts the node inside a different zone

						//Get bytes for destination address
						uint8_t addressBuf[16];
						destination.GetBytes(addressBuf);

						//Get bytes for RSU address
						uint8_t rsuAddressBuf[16];
						t1.GetRsuAddress().GetBytes(rsuAddressBuf);

						//Buffer to hold newly formed address
						uint8_t newDestinationAddressBuffer[16];

						//Set Network address bits
						for(int i = 0; i < 8; i++)
						{
							newDestinationAddressBuffer[i] = rsuAddressBuf[i];
						}

						//Set Host address bits
						for(int i = 8; i < 16; i++)
						{
							newDestinationAddressBuffer[i] = addressBuf[i];
						}

						Ipv6Address newDestinationAddress;
						newDestinationAddress.Set(newDestinationAddressBuffer);

						//Create new header with the updated location
						Ipv6Header newV6Header;
						newV6Header.SetSourceAddress(header.GetSourceAddress());
						newV6Header.SetDestinationAddress(newDestinationAddress);
						newV6Header.SetNextHeader(header.GetNextHeader());
						newV6Header.SetHopLimit(header.GetHopLimit());
						newV6Header.SetPayloadLength(header.GetPayloadLength());
						newV6Header.SetTrafficClass(newV6Header.GetTrafficClass());
						newV6Header.SetFlowLabel(header.GetFlowLabel());

						//Create a new type header
						mcast::TypeHeader redirectHeader(mcast::INTERNET_RSU_TO_RSU_REDIRECT);

						//Create new ITV header
						ITVHeader itvh(mobility -> GetPosition(), entry.GetSendTime(), false, mobility -> GetPosition(),mobility -> GetVelocity(),predictedPosition,0);

						//Get route to the new RSU
						//						Ptr<Ipv6Route> redirectRoute = Lookup(newDestinationAddress,m_pp);

						Ipv6RoutingTableEntry sr = m_sr6 -> GetDefaultRoute();
						Ptr<Ipv6Route> redirectRoute = Create<Ipv6Route> ();
						redirectRoute -> SetDestination(sr.GetDestNetwork());
						redirectRoute -> SetGateway(sr.GetGateway());
						redirectRoute -> SetOutputDevice(m_ipv6 -> GetNetDevice(sr.GetInterface()));
						redirectRoute -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());


						//Form packet
						packet -> AddHeader(itvh);
						packet -> AddHeader(redirectHeader);

						ucb (m_pp,redirectRoute, packet, newV6Header);

					}
				}
			}else
			{
				//packet -> Print(std::cout);
				//std::cout << "<<<<<<<<<<<<<< Should not have happened yet >>>>>>>>>>>>>>>>>" << std::endl;
				//std::cout << "Destination: " << header.GetDestinationAddress() << std::endl;
			}
		}
		return true;
	}

	return false;
}

Vector
ThesisInternetRoutingProtocol2::GetPredictedNodePosition(Ipv6Address nodeAddress,Vector oldPosition, Vector reportedVelocity ,Time originalSendTime)
{

	NS_LOG_FUNCTION(this);

	Vector newPosition;

	//	std::cout << "Current time: " << Simulator::Now() << std::endl;
	//	std::cout << "Original Send Time " << originalSendTime << std::endl;

	Time diff = Simulator::Now() - originalSendTime;


	if(reportedVelocity.x == 0 && reportedVelocity.y ==0)
	{
		//Calculate averages to try to get a better reading
		Vector avgVelocity = m_RsuCache.GetAverageVelocity(nodeAddress);

		//If both reported velocity AND average velocity are both 0 only option left is to return the original position and assume
		//the node hasn't moved
		if(avgVelocity.x == 0 && avgVelocity.y == 0)
		{
			newPosition = oldPosition;
		}else
		{
			newPosition.x = oldPosition.x + (diff.GetSeconds() * avgVelocity.x);
			newPosition.y = oldPosition.y + (diff.GetSeconds() * avgVelocity.y);
		}

	}else
	{
		//Velocities were not 0; calculate position based on the reported velocity
		newPosition.x = oldPosition.x + (diff.GetSeconds() * reportedVelocity.x);
		newPosition.y = oldPosition.y + (diff.GetSeconds() * reportedVelocity.y);
	}

	return newPosition;
}


bool
ThesisInternetRoutingProtocol2::RouteInputVanet (Ptr<const Packet> p, const Ipv6Header &header, Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb, MulticastForwardCallback mcb,
		LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this << header);

	//Copy passed packet to a new packet
	Ptr<Packet> packet = p -> Copy();

	//packet -> Print(std::cout);
	//std::cout << std::endl;

	//Input device was loopback; only packets coming through loopback should be deferred
	if(idev == m_lo)
	{
		NS_LOG_INFO("Vanet node recved on loopback");

		//std::cout << "VANET RECV ON LOOPBACK <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;

		uint32_t iif = (idev ? m_ipv6->GetInterfaceForDevice (idev) : -1);
		DeferredRouteOutputTag tag(iif);
		if(packet->PeekPacketTag(tag))
		{
			//std::cout << "Peek finished; deferred tag found, remove and start IR" << std::endl;
			Ipv6Address destination = header.GetDestinationAddress();

			Transmission t;
			t.Destination = destination;
			t.SendTime = Simulator::Now();
			m_sourcedTrans.push_back(t);
			m_numSourced++;

			if(m_numSourced > 200)
			{
			//	std::cout << "Over 200" << std::endl;
			//	std::cout << "Destination: " << destination << std::endl;
			//	std::cout << "Source: " << header.GetSourceAddress() << std::endl;
			}

		}

		//Remove DR tag
		packet -> RemovePacketTag(tag);

		int32_t forCurrentNode = m_ipv6 -> GetInterfaceForAddress(header.GetDestinationAddress());
		if(!(forCurrentNode == -1))
		{
			//For current Node
			lcb (packet, header, iif);
			return true;
		}

		if(header.GetDestinationAddress().IsMulticast())
		{
			std::cout << "Got multicast destination" << std::endl;
			Ptr<NetDevice> ndev = m_wi;
			Ptr<Ipv6Route> route;
			Ipv6Address destination = header.GetDestinationAddress();
			route = Lookup(destination,ndev);
			//Found route; forward along
			if(route)
			{
				ucb (ndev,route, packet, header);
				return true;
			}
			return true;
		}


		bool isInternet = false;
		bool isGeoRequest = false;
		////////// Check if for an internet Node or VANET ////////////////////
		/*
		if(header.GetDestinationAddress() == Ipv6Address("ff02::118"))
		{
			isGeoRequest = true;
		}else if(header.GetDestinationAddress() == Ipv6Address("ff02::119"))
		{
			isGeoRequest = false;
		}else
		{
			isInternet = true;
		}*/


		uint8_t currentBytes[16];
		uint8_t destBytes[16];

		Ipv6Address currentAddress = m_ipv6->GetAddress(m_ipv6->GetInterfaceForDevice(m_wi),1).GetAddress();

		currentAddress.GetBytes(currentBytes);
		header.GetDestinationAddress().GetBytes(destBytes);

		for(int i = 0; i < 4; i++)
		{
			if(currentBytes[i] != destBytes[i])
			{
				isInternet = true;
				break;
			}
		}

		if(!isInternet)
		{
			//isGeoRequest = true;
			for(int i = 8; i < 16; i++)
			{
				if(currentBytes[i] != destBytes[i])
				{
					isGeoRequest = false;
					break;
				}
				isGeoRequest = true;
			}
		}

		//All GeoRequest destinations are assumes to have 0 filled host sections
		//A non-zero host section indicates a reply
		//bool isGeoRequest = true;
		/*
		for(int i = 8; i < 16; i++)
		{
			if(destBytes[i] != 0)
			{
				isGeoRequest = false;
				break;
			}
		}*/

		///////////////////////////////////////////////////////////////////////////////////////////

		if(isInternet)
		{

			//Create new typeHeader
			mcast::TypeHeader theader (mcast::INTERNET);

			//Get mobility model properties and extract values needed for header
			Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();

			Vector position = mobility -> GetPosition();
			Vector velocity = mobility -> GetVelocity();
			Time CurrentTime = Simulator::Now();

			//Instantiate new ThesisInternetRouting header.
			InternetHeader Ih(position,velocity,CurrentTime,m_IsDtnTolerant,position,velocity,m_RsuDestination,0);

			//		std::cout << "Sending IH position:  " << position << std::endl;
			//		std::cout << "Sending IH velocity " << velocity  << std::endl;
			//		std::cout << "Sending IH currentTime" << CurrentTime  << std::endl;

			//Add headers; IH first than type, read in reverse order on receving end
			packet -> AddHeader(Ih);

			packet -> AddHeader(theader);

			//Find actual route to send packet, send using UCB callback
			Ptr<Ipv6Route> route;
			Ipv6Address destination = header.GetDestinationAddress();
			Ipv6Address source = header.GetSourceAddress();

			Ptr<NetDevice> ndev = m_wi;

			route = Lookup(destination,ndev);

			//Found route; forward along
			if(route)
			{
				ucb (ndev,route, packet, header);
				return true;
			}
		}else
		{// Geocast transmission

			if(isGeoRequest) //GeoQuery request
			{
				NS_LOG_INFO("SENDING GEO REQUEST");
				//Create new typeHeader
				mcast::TypeHeader theader (mcast::GEOQUERY_REQUEST);

				//Get mobility model properties and extract values needed for header
				Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();

				Vector position = mobility -> GetPosition();
				Vector velocity = mobility -> GetVelocity();
				Time CurrentTime = Simulator::Now();

				//Instantiate new GeoQuery header.
				GeoRequestHeader GH(position,velocity,CurrentTime,m_IsDtnTolerant,position,velocity,m_RsuDestination,0,m_GeoQueryPosition);

				//Add headers; GB first than type, read in reverse order on receving end
				packet -> AddHeader(GH);

				packet -> AddHeader(theader);

				//Find actual route to send packet, send using UCB callback
				Ptr<Ipv6Route> route;
				Ipv6Address destination = header.GetDestinationAddress();

				//Ipv6Address destination =m_Db -> Get

				Ipv6Address source = header.GetSourceAddress();

				Ptr<NetDevice> ndev = m_wi;

				route = Lookup(destination,ndev);

				//Found route; forward along
				if(route)
				{
					ucb (ndev,route, packet, header);
					return true;
				}

			}else// GeoQuery Reply
			{
				NS_LOG_INFO("SENDING GEOREPLY");
				mcast::TypeHeader theader (mcast::GEOQUERY_REPLY);

				GeoReplyHeader GH;

				//Add reply headers
				packet -> AddHeader(GH);
				packet -> AddHeader(theader);


				//Find actual route to send packet, send using UCB callback
				Ptr<Ipv6Route> route;
				Ipv6Address destination = header.GetDestinationAddress();

				//Ipv6Address destination =m_Db -> Get

				Ipv6Address source = header.GetSourceAddress();

				Ptr<NetDevice> ndev = m_wi;

				route = Lookup(destination,ndev);

				//Found route; forward along
				if(route)
				{
					ucb (ndev,route, packet, header);
					return true;
				}

			}
		}

	}
	else
	{
		//Received packet on another interface; must have been wifi since only 2 interfaces

		//	std::cout << "Peeking at type header - Input VANET" << std::endl;
		//Create new typeHeader and peek
		mcast::TypeHeader theader (mcast::UNKNOWN);
		packet -> PeekHeader(theader);
		if(theader.Get() == mcast::INTERNET)
		{
			NS_LOG_INFO("Received Internet packet on VANET");

			//TEMPORARY
//			std::cout << "Destination: " << header.GetDestinationAddress() << std::endl;
//			std::cout << "Source: " << header.GetSourceAddress() << std::endl;
//			packet -> Print(std::cout);

			//Received type header of type internet
			//packet -> PeekHeader(theader);
			//std::cout << " BREAKING? Type header with type: " << theader.Get() << std::endl;

			/////////////////////////Remove headers; add again before placing into queue

			//Check that this isn't a packet retransmitted from another VANET node but originating from this node
			if(m_ipv6->GetInterfaceForAddress(header.GetSourceAddress()) != -1)
			{
				//std::cout << std::endl;
				//std::cout << "Retransmit from current node" << std::endl;

				return true;
			}

			NS_LOG_INFO("Vanet Route Input Packet ");
			//packet -> Print(std::cout);

			mcast::TypeHeader typeHeader (mcast::UNKNOWN);
			packet -> RemoveHeader(typeHeader);
			NS_LOG_INFO("Removed typeHeader " << typeHeader);
			//			std::cout << typeHeader <<std::endl;
			//			std::cout << std::endl;

			InternetHeader Ih;
			packet -> RemoveHeader(Ih);
			NS_LOG_INFO("Removed internetHeader " << Ih);

			//Check hop count limit exceeded, if true drop packet and return
			if(Ih.GetHopCount() > m_hopCountLimit)
			{
				return true;
			}

			//Check that the packet did not come from another zone
			//Don't allow interzone wifi routing for the moment
			Ipv6Address destinationRSU = Ih.GetRsuAddress();
			if(!destinationRSU.IsEqual(m_currentRsu.GetRsuAddress()))
			{
				//Destination address was not equal to current RSU address
				//Drop packet
				//				std::cout << "Destination RSU: " << destinationRSU << std::endl;
				//				std::cout << "Current RSU " << m_currentRsu.GetRsuAddress() << std::endl;
				return true;
			}


			//////////////////////////////////////////

			Ptr<Node> theNode = GetObject<Node>();
			Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

			//Check strictly effective and if the position satisfies effectivity parameters
			if(m_isStrictEffective)
			{
				if(!IsEffective(Ih.GetSenderPosition()))
				{
					return true;
				}
			}

			Time backoff = GetBackoffDuration(Ih.GetSenderPosition());
			Ipv6Address source = header.GetSourceAddress();
			Ipv6Address destination = header.GetDestinationAddress();
			Time timeStamp = Ih.GetTimestamp();

			bool CacheContains = m_RoutingCache.Lookup(source, destination, timeStamp);

			if(CacheContains)
			{
				//Cache contains entry with the source,destination,timestamp tuple already (Stop timer on retransmit and remove)
				ThesisInternetQueueEntry * entry = m_RoutingCache.GetRoutingEntry(source,destination,timeStamp);

				//Stop timer before removing
				entry -> m_RetransmitTimer.Cancel();

				//Remove queue entry to stop retransmit
				//				m_RoutingCache.RemoveRoutingQueueEntry(source,destination,timeStamp);

				//Change to remove with a delay
				entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveThesisRoutingCacheEntry, this);
				entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
				entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);
				entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);
			}else
			{
				//Check if number of active transmissions greater than some value
				//std::cout << "Active Cache size: " << m_RoutingCache.GetNumActive(m_ThesisInternetRoutingCacheCooldown,0.1) << std::endl;
				if(m_RoutingCache.GetNumActive(m_ThesisInternetRoutingCacheCooldown, 0.1) > m_activeLimit)
				{
					return true;
				}

				//Re-Add headers before placing into queue
				packet ->AddHeader(Ih);
				packet ->AddHeader(typeHeader);

				//Cache does not contain and entry with the tuple (Add to cache and start timer on retransmit
				ThesisInternetQueueEntry * entry = new ThesisInternetQueueEntry(packet, header, ucb, ecb, timeStamp);

				entry->m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::SendInternetRetransmit, this);
				entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
				entry->m_RetransmitTimer.Schedule(backoff);

				entry -> SetInitialDelay(backoff);

				m_RoutingCache.AddRoutingEntry(entry);

				return true;
			}
		}
		else if(theader.Get() == mcast::GEOQUERY_REQUEST)
		{
			NS_LOG_INFO("Received GeoQuery packet on VANET");

			//std::cout << std::endl;

			//packet -> Print(std::cout);

			//std::cout << std::endl;

			//Received type header of type internet
			//packet -> PeekHeader(theader);
			//std::cout << " BREAKING? Type header with type: " << theader.Get() << std::endl;

			/////////////////////////Remove headers; add again before placing into queue

			//Check that this isn't a packet retransmitted from another VANET node but originating from this node
			if(m_ipv6->GetInterfaceForAddress(header.GetSourceAddress()) != -1)
			{
				return true;
			}

			mcast::TypeHeader typeHeader (mcast::UNKNOWN);
			packet -> RemoveHeader(typeHeader);
			NS_LOG_INFO("Removed typeHeader " << typeHeader);

			GeoRequestHeader Gh;
			packet -> RemoveHeader(Gh);
			NS_LOG_INFO("Removed GeoRequestHeader " << Gh);

			DbEntry entry = m_Db -> GetEntryForCurrentPosition(Gh.GetQueryPosition());

			//Current RSU != entry RSU means forward to Gateway
			//Otherwise RSU == Current RSU means zone has been reached, start reply process
			if(!m_currentRsu.GetRsuAddress().IsEqual(entry.GetRsuAddress()))
			{

				//Check hop count limit exceeded, if true drop packet and return
				if(Gh.GetHopCount() > m_hopCountLimit)
				{
					return true;
				}

				//Check that the packet did not come from another zone
				//Don't allow interzone wifi routing for the moment
				Ipv6Address destinationRSU = Gh.GetRsuAddress();
				if(!destinationRSU.IsEqual(m_currentRsu.GetRsuAddress()))
				{
					//Destination address was not equal to current RSU address
					//Drop packet
					return true;
				}

				Ptr<Node> theNode = GetObject<Node>();
				Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

				//Check strictly effective and if the position satisfies effectivity parameters
				if(m_isStrictEffective)
				{
					if(!IsEffective(Gh.GetSenderPosition()))
					{
						return true;
					}
				}

				Time backoff = GetBackoffDuration(Gh.GetSenderPosition());
				Ipv6Address source = header.GetSourceAddress();
				Ipv6Address destination = header.GetDestinationAddress();
				Time timeStamp = Gh.GetTimestamp();

				bool CacheContains = m_GeoRoutingQueue.Lookup(source, destination, timeStamp);
				if(CacheContains)
				{
					//Cache contains entry with the source,destination,timestamp tuple already (Stop timer on retransmit and remove)
					ThesisInternetQueueEntry * entry = m_GeoRoutingQueue.GetRoutingEntry(source,destination,timeStamp);

					//Stop timer before removing
					entry -> m_RetransmitTimer.Cancel();

					//Remove queue entry to stop retransmit
					//				m_RoutingCache.RemoveRoutingQueueEntry(source,destination,timeStamp);

					//Change to remove with a delay
					entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoRoutingCacheEntry, this);
					entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
					entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);
					entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);
					return true;
				}else
				{
					//Re-Add headers before placing into queue
					packet ->AddHeader(Gh);
					packet ->AddHeader(typeHeader);

					//Cache does not contain and entry with the tuple (Add to cache and start timer on retransmit
					ThesisInternetQueueEntry * entry = new ThesisInternetQueueEntry(packet, header, ucb, ecb, timeStamp);

					entry->m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::SendGeoQueryRetransmit, this);
					entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
					entry->m_RetransmitTimer.Schedule(backoff);

					entry -> SetInitialDelay(backoff);

					m_GeoRoutingQueue.AddRoutingEntry(entry);

					return true;
				}
			}else
			{
				NS_LOG_INFO("Received GeoQuery from other zone");
				//std::cout << "VANET NODE RECEIVED GEOQUERY FROM OTHER ZONE" <<std::endl;
				Ptr<Node> theNode = GetObject<Node>();
				Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();
				Vector currentPos = mobility -> GetPosition();

				//Check current node close than RSU
				int nodeDistanceToQueryPos = utils.GetDistanceBetweenPoints(currentPos.x, currentPos.y, Gh.GetQueryPosition().x, Gh.GetQueryPosition().y);
				int rsuDistanceToQueryPos = utils.GetDistanceBetweenPoints(m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y, Gh.GetQueryPosition().x, Gh.GetQueryPosition().y);

				//Check node is close to query point than RSU
				//If not return else fall through for more processing

				//std::cout << "Node distance to query Pos: " << nodeDistanceToQueryPos << std::endl;
				//std::cout << "RSU distance to query Pos " << rsuDistanceToQueryPos << std::endl;

				if(nodeDistanceToQueryPos > rsuDistanceToQueryPos)
				{
					//return true;
				}

				double ratio = nodeDistanceToQueryPos/rsuDistanceToQueryPos;
				ratio = ceil(ratio * 100);

				//Backoff in microseconds
				Time toWait = MicroSeconds(ratio * m_rWait);
				Ipv6Address source = header.GetSourceAddress();
				Ipv6Address destination = header.GetDestinationAddress();
				Time timeStamp = Gh.GetTimestamp();

				bool CacheContains = m_GeoQueryReplyCache.Lookup(source, destination, timeStamp);
				if(CacheContains)
				{
					//Cache already contains this entry. Suspend transmission and start cancel process
					ThesisInternetQueueEntry * entry = m_GeoQueryReplyCache.GetRoutingEntry(source,destination,timeStamp);

					//Stop timer before removing
					entry -> m_RetransmitTimer.Cancel();

					//Remove queue entry to stop retransmit
					//				m_RoutingCache.RemoveRoutingQueueEntry(source,destination,timeStamp);

					//Change to remove with a delay
					entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoQueryReplyCacheEntry, this);
					entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
					entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

					entry ->SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);

					return true;
				}else
				{

					if(m_GeoQueryReplyCache.GetNumActive(m_ThesisInternetRoutingCacheCooldown,0.1) > m_activeLimit)
					{
						//Too many active elements already just cache and set for removal
						packet ->AddHeader(Gh);
						packet ->AddHeader(typeHeader);

						//Cache does not contain and entry with the tuple (Add to cache and start timer on retransmit
						ThesisInternetQueueEntry * entry = new ThesisInternetQueueEntry(packet, header, ucb, ecb, timeStamp, lcb);

						entry->m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::SendGeoQueryReply, this);
						entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
						entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

						m_GeoQueryReplyCache.AddRoutingEntry(entry);

						entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);

					}

					//Re-Add headers before placing into queue
					packet ->AddHeader(Gh);
					packet ->AddHeader(typeHeader);

					//Cache does not contain and entry with the tuple (Add to cache and start timer on retransmit
					ThesisInternetQueueEntry * entry = new ThesisInternetQueueEntry(packet, header, ucb, ecb, timeStamp, lcb);

					entry->m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::SendGeoQueryReply, this);
					entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
					entry->m_RetransmitTimer.Schedule(toWait);

					m_GeoQueryReplyCache.AddRoutingEntry(entry);

					entry -> SetInitialDelay(toWait);

					return true;
				}
				return true;
			}
		}else if(theader.Get() == mcast::INTERNET_RSU_TO_VANET)
		{
			NS_LOG_INFO("Received RSU_TO_VANET packet on VANET");

			//Type 4 is RSU to VANET (Handle later)
			Ipv6Address destination = header.GetDestinationAddress();

			//std::cout << "Internet Rsu to Vanet destination: " << destination << std::endl;

			NS_LOG_LOGIC("Destination: " << destination << " Interface for address: " << m_ipv6 -> GetInterfaceForAddress(destination));
			NS_LOG_LOGIC("Current address: " <<  m_ipv6->GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1));
			if(m_ipv6 -> GetInterfaceForAddress(destination) != -1 && CheckHostBits(destination))
			{

				//				std::cout << ">>>>>> Received packet for this node; forwarding LCB <<<<<<<"<< std::endl;

				//				std::cout << "Header Properties: "<< std::endl;
				//				std::cout << "Destination: " << header.GetDestinationAddress() << std::endl;
				//				std::cout << "Source: " << header.GetSourceAddress() << std::endl;
				//				std::cout << std::endl;


				//Remove headers before forwarding up, maybe thats causing the problem??
				mcast::TypeHeader typeHeader (mcast::UNKNOWN);
				packet -> RemoveHeader(typeHeader);

				ITVHeader itvhdr(Vector(), Simulator::Now(), false, Vector(), Vector(), Vector(),0);
				packet -> RemoveHeader(itvhdr);


				Ipv6Address source = header.GetSourceAddress();
				///Set transmission statistics for data collection
				//std::cout << std::endl;
				for(TransmissionsIt it = m_sourcedTrans.begin(); it != m_sourcedTrans.end(); it++)
				{
					//std::cout << "Cache destination: " << it -> Destination << std::endl;
					//std::cout << "Cache SendTime: " << it -> SendTime << std::endl;

					//Header properties
					//std::cout << "Source: (From RSU) " << source << std::endl;
					//std::cout << "source Timestamp: (From RSU) " << itvhdr.GetOriginalTimestamp() << std::endl;


					if(/*it -> Destination == source && */ it -> SendTime == itvhdr.GetOriginalTimestamp())
					{
						//std::cout << "GOT HERE" << std::endl;
						m_numReceived++;

						//						std::cout << "RTT: " << Simulator::Now() - itvhdr.GetOriginalTimestamp() << std::endl;

						m_RTT = m_RTT + (Simulator::Now() - itvhdr.GetOriginalTimestamp());

						//Ptr<Ipv6L3Protocol> l3 = GetObject<Ipv6L3Protocol>();
						//l3 -> GetTypeId() .Get
						//std::cout << "Hop Limit: " << (unsigned)header.GetHopLimit() << std::endl;


						//Remove 1 from hop count to offset the hop from the Hub to the RSU
						//Default hop limit set to 64
						m_HopCountAgregatorRsuToVanet = m_HopCountAgregatorRsuToVanet + (64- header.GetHopLimit()) -1;

						m_sourcedTrans.erase(it);
						break;
					}
				}
				//std::cout << std::endl;



				int32_t iif = m_ipv6->GetInterfaceForDevice (idev);
				lcb (packet, header, iif);

				//Need to send out an ACK message to surrounding node to stop their retransmission
				//Ensure sending out to a multi-cast address to avoid ARP issues
				//Ensure Packet size is 0 (Less than 4 bytes)

				Ptr<Packet> ackPacket = Create<Packet> ();

				mcast::TypeHeader ackHeader (mcast::INTERNET_VANET_ACK);

				ackPacket -> AddHeader(itvhdr);
				ackPacket -> AddHeader(ackHeader);

				Ptr<Ipv6Route> ackEntry = Create<Ipv6Route> ();
				ackEntry -> SetDestination(Ipv6Address(VANET_TO_RSU));
				ackEntry -> SetGateway(Ipv6Address(VANET_TO_RSU));
				ackEntry -> SetOutputDevice(m_wi);
				ackEntry -> SetSource(Ipv6Address(VANET_TO_RSU));

				//Send Ack Message
				ucb (m_wi,ackEntry, ackPacket, header);

				return true;
			}else
			{
				//Transmission not for me, may need to retransmit
				//Calculate a backoff timer and place into routing queue

				//Remove headers before forwarding up, maybe thats causing the problem??
				mcast::TypeHeader typeHeader (mcast::UNKNOWN);
				packet -> RemoveHeader(typeHeader);

				ITVHeader itvhdr(Vector(), Simulator::Now(), false, Vector(), Vector(), Vector());
				packet -> RemoveHeader(itvhdr);

				if(m_isStrictEffective)
				{
					//VANET routing mode is strictly effective; check effectivity of retransmitting
					if(!IsEffectiveV2VTransmission(itvhdr.GetSenderPosition(),itvhdr.GetPredictedPosition()))
					{
						return true;
					}
				}

				//Check hop count; if exceed max then drop packets
				//std::cout << "Checking hop count limit of: " << (unsigned)m_hopCountLimit << std::endl;
				if(itvhdr.GetHopCount() > m_hopCountLimit)
				{
					return true;
				}

				///////////////////////////////////////////////////////////////////////////////////////////////////

				Time backoff = GetV2VBackoffDuration(itvhdr.GetSenderPosition(), itvhdr.GetPredictedPosition());
				Ipv6Address source = header.GetSourceAddress();
				Ipv6Address destination = header.GetDestinationAddress();
				Time timeStamp = itvhdr.GetOriginalTimestamp();

				bool CacheContains = m_RoutingRtoVCache.Lookup(source, destination, timeStamp);

				if(CacheContains)
				{

					//					std::cout << "Cache Entry hit in INTERNET_RSU_TO_VANET" << std::endl;

					//Cache contains entry with the source,destination,timestamp tuple already (Stop timer on retransmit and remove)
					ThesisInternetQueueEntry * entry = m_RoutingRtoVCache.GetRoutingEntry(source,destination,timeStamp);

					//Stop timer before removing
					entry -> m_RetransmitTimer.Cancel();

					//Remove queue entry to stop retransmit
					//					m_RoutingCache.RemoveRoutingQueueEntry(source,destination,timeStamp);

					//Schedule remove with delay
					entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveThesisRoutingRtVCacheEntry, this);
					entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
					entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

					entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);

				}else
				{

					if(m_RoutingRtoVCache.GetNumActive(m_ThesisInternetRoutingCacheCooldown, 0.1) > m_activeLimit)
					{
						return true;
					}


					//Re-Add headers before placing into queue
					packet ->AddHeader(itvhdr);
					packet ->AddHeader(typeHeader);

					//Cache does not contain and entry with the tuple (Add to cache and start timer on retransmit
					ThesisInternetQueueEntry * entry = new ThesisInternetQueueEntry(packet, header, ucb, ecb, timeStamp);

					//Timer * toRetransmit = entry -> GetTimer();

					entry->m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::SendInternetRetransmitIntoVanet, this);
					entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
					entry->m_RetransmitTimer.Schedule(backoff);

					m_RoutingRtoVCache.AddRoutingEntry(entry);

					entry -> SetInitialDelay(backoff);

					return true;
				}
				///////////////////////////////////////////////////////////////////////////////////////////////////

				return true;
			}

		}else if(theader.Get() == mcast::INTERNET_RSU_ACK)
		{
			NS_LOG_INFO("Received INTERNET_RSU_ACK packet on VANET");

			//			std::cout << ">>>>>> GOT PACKET WITH TYPE 5 - RSU ACK <<<<<<<"<< std::endl;

			/////////////////////////Remove headers; packet will be discarded after this anyway
			mcast::TypeHeader typeHeader (mcast::UNKNOWN);
			packet -> RemoveHeader(typeHeader);

			InternetHeader Ih;
			packet -> RemoveHeader(Ih);
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			//Remove entry for queue (If it exists)
			//			m_RoutingCache.RemoveRoutingQueueEntry(header.GetSourceAddress(), header.GetDestinationAddress(), Ih.GetTimestamp());

			//Remove with delay
			Ipv6Address source = header.GetSourceAddress();
			Ipv6Address destination = header.GetDestinationAddress();
			Time sendTime = Ih.GetTimestamp();

			if(m_RoutingCache.Lookup(source,destination,sendTime))
			{

				ThesisInternetQueueEntry * entry = m_RoutingCache.GetRoutingEntry(source,destination,sendTime);

				if(entry -> m_RetransmitTimer.IsRunning())
				{
					entry -> m_RetransmitTimer.Cancel();
				}
				//Schedule removal of entry
				entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveThesisRoutingCacheEntry, this);
				entry -> m_RetransmitTimer.SetArguments(source,destination,sendTime);
				entry -> m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);
			}

			return true;
		}else if(theader.Get() == mcast::INTERNET_VANET_ACK)
		{
			NS_LOG_INFO("Received VANET_ACK packet on VANET");
			//Ack message by a VANET node
			//Cancel stop and cancel timer retransmission
			//Remove entry from routing cache
			mcast::TypeHeader ack(mcast::UNKNOWN);
			packet -> RemoveHeader(ack);

			ITVHeader itvhdr(Vector(), Simulator::Now(), false, Vector(), Vector(), Vector());
			packet -> RemoveHeader(itvhdr);

			//			std::cout << "Received VANET Ack, Source: " << header.GetSourceAddress() <<
			//									 " Destination: " << header.GetDestinationAddress() <<
			//									 " Original timestamp: " << itvhdr.GetOriginalTimestamp() << std::endl;

			//			m_RoutingCache.RemoveRoutingQueueEntry(header. GetSourceAddress(), header.GetDestinationAddress(), itvhdr.GetOriginalTimestamp());

			//Schedule removal with delay
			Ipv6Address source = header.GetSourceAddress();
			Ipv6Address destination = header.GetSourceAddress();
			Time sendTime = itvhdr.GetOriginalTimestamp();

			if(m_RoutingCache.Lookup(source,destination,sendTime))
			{

				ThesisInternetQueueEntry * entry = m_RoutingCache.GetRoutingEntry(source,destination,sendTime);
				//Schedule removal of entry
				entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveThesisRoutingCacheEntry, this);
				entry -> m_RetransmitTimer.SetArguments(source,destination,sendTime);
				entry -> m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);
			}

			return true;
		}else if(theader.Get() == mcast::GEOQUERY_RSU_ACK)
		{
			NS_LOG_INFO("Received GEOQUERY_RSU_ACK packet on VANET");

			//			std::cout << ">>>>>> GOT PACKET WITH TYPE 5 - RSU ACK <<<<<<<"<< std::endl;

			/////////////////////////Remove headers; packet will be discarded after this anyway
			mcast::TypeHeader typeHeader (mcast::UNKNOWN);
			packet -> RemoveHeader(typeHeader);

			GeoRequestHeader Gh;
			packet -> RemoveHeader(Gh);
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			//Remove with delay
			Ipv6Address source = header.GetSourceAddress();
			Ipv6Address destination = header.GetDestinationAddress();
			Time sendTime = Gh.GetTimestamp();

			if(m_GeoRoutingQueue.Lookup(source,destination,sendTime))
			{

				ThesisInternetQueueEntry * entry = m_GeoRoutingQueue.GetRoutingEntry(source,destination,sendTime);

				if(entry -> m_RetransmitTimer.IsRunning())
				{
					entry -> m_RetransmitTimer.Cancel();
				}
				//Schedule removal of entry
				entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoRoutingCacheEntry, this);
				entry -> m_RetransmitTimer.SetArguments(source,destination,sendTime);
				entry -> m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

				entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);

			}

			return true;
		}else if(theader.Get() == mcast::GEOQUERY_SENDINGREPLY)
		{
			NS_LOG_INFO("Received Geo Query Sending Reply");

			//			std::cout << ">>>>>> GOT PACKET WITH TYPE 5 - RSU ACK <<<<<<<"<< std::endl;

			/////////////////////////Remove headers; packet will be discarded after this anyway
			mcast::TypeHeader typeHeader (mcast::UNKNOWN);
			packet -> RemoveHeader(typeHeader);

			GeoRequestHeader Gh;
			packet -> RemoveHeader(Gh);
			////////////////////////////////////////////////////////////////////////////////////////////////////////

			//Remove entry for queue (If it exists)
			//			m_RoutingCache.RemoveRoutingQueueEntry(header.GetSourceAddress(), header.GetDestinationAddress(), Ih.GetTimestamp());

			//Remove with delay
			Ipv6Address source = header.GetSourceAddress();
			Ipv6Address destination = header.GetDestinationAddress();
			Time sendTime = Gh.GetTimestamp();
			if(m_GeoQueryReplyCache.Lookup(source,destination,sendTime))
			{

				ThesisInternetQueueEntry * entry = m_GeoQueryReplyCache.GetRoutingEntry(source,destination,sendTime);

				if(entry -> m_RetransmitTimer.IsRunning())
				{
					entry -> m_RetransmitTimer.Cancel();
				}
				//Schedule removal of entry
				entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoQueryReplyCacheEntry, this);
				entry -> m_RetransmitTimer.SetArguments(source,destination,sendTime);
				entry -> m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

				entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);

			}
			return true;
		}else if(theader.Get() == mcast::GEOQUERY_REPLY)
		{
			NS_LOG_INFO("VANET NODE RECV GEOQUERY REPLY");

			//Check zone; if incorrect zone then dont retransmit (Current implementation assuming 1 hop)
			Ipv6Address currentAdd = m_ipv6 ->GetAddress(m_ipv6->GetInterfaceForDevice(m_wi),1).GetAddress();
			Ipv6Address destination = header.GetDestinationAddress();

			uint8_t currentBytes[16];
			uint8_t destBytes[16];

			currentAdd.GetBytes(currentBytes);
			destination.GetBytes(destBytes);

			bool correctZone = true;
			for(int i = 0; i < 8; i++)
			{
				if(currentBytes[i] != destBytes[i])
				{
					correctZone = false;
					break;
				}
			}

			if(!correctZone)
			{
				//Same zone, don't do anything here
				return true;
			}else
			{
				NS_LOG_INFO("RECV GEOREPLY ON VANET NODE IN TARGET ZONE");

				//Check if current address equal to destination or if
				Ipv6Address destination = header.GetDestinationAddress();

				//std::cout << "Internet Rsu to Vanet destination: " << destination << std::endl;

				NS_LOG_LOGIC("Destination: " << destination << " Interface for address: " << m_ipv6 -> GetInterfaceForAddress(destination));
				NS_LOG_LOGIC("Current address: " <<  m_ipv6->GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1));
				//if(m_ipv6 -> GetInterfaceForAddress(destination) != -1 && CheckHostBits(destination))
				if(CheckHostBits(destination))
				{
					//Received query reply for this node, remove all headers and send upwards
					//Remove headers before forwarding up, maybe thats causing the problem??
					mcast::TypeHeader typeHeader (mcast::UNKNOWN);
					packet -> RemoveHeader(typeHeader);

					GeoReplyHeader Gh;
					packet -> RemoveHeader(Gh);

					int32_t iif = m_ipv6->GetInterfaceForDevice (idev);
					lcb (packet, header, iif);


					//Need to send am ACK packet to allow others to clear caches
					Ptr<Packet> ackPacket = Create<Packet> ();

					mcast::TypeHeader ackHeader (mcast::GEOREPLY_VANET_ACK);

					ackPacket -> AddHeader(Gh);
					ackPacket -> AddHeader(ackHeader);

					Ptr<Ipv6Route> ackEntry = Create<Ipv6Route> ();
					ackEntry -> SetDestination(Ipv6Address(VANET_TO_RSU));
					ackEntry -> SetGateway(Ipv6Address(VANET_TO_RSU));
					ackEntry -> SetOutputDevice(m_wi);
					ackEntry -> SetSource(Ipv6Address(VANET_TO_RSU));

					//Send Ack Message
					ucb (m_wi,ackEntry, ackPacket, header);

					return true;
				}else
				{
					//For other node start retransmission process

					//Remove headers before forwarding up, maybe thats causing the problem??
					mcast::TypeHeader typeHeader (mcast::UNKNOWN);
					packet -> RemoveHeader(typeHeader);

					GeoReplyHeader Gh;
					packet -> RemoveHeader(Gh);

					if(m_isStrictEffective)
					{
						//VANET routing mode is strictly effective; check effectivity of retransmitting
						if(!IsEffectiveV2VTransmission(Gh.GetSenderPosition(),Gh.GetPredictedPosition()))
						{
							return true;
						}
					}

					//Check hop count; if exceed max then drop packets
					//std::cout << "Checking hop count limit of: " << (unsigned)m_hopCountLimit << std::endl;
					if(Gh.GetHopCount() > m_hopCountLimit)
					{
						return true;
					}

					///////////////////////////////////////////////////////////////////////////////////////////////////

					Time backoff = GetV2VBackoffDuration(Gh.GetSenderPosition(), Gh.GetPredictedPosition());
					Ipv6Address source = header.GetSourceAddress();
					Ipv6Address destination = header.GetDestinationAddress();
					Time timeStamp = Gh.GetOriginalTimestamp();

					bool CacheContains = m_GeoReplyQueue.Lookup(source, destination, timeStamp);
					if(CacheContains)
					{

						//Cache contains entry with the source,destination,timestamp tuple already (Stop timer on retransmit and remove)
						ThesisInternetQueueEntry * entry = m_GeoReplyQueue.GetRoutingEntry(source,destination,timeStamp);

						//Stop timer before removing
						entry -> m_RetransmitTimer.Cancel();

						//Remove queue entry to stop retransmit
						//					m_RoutingCache.RemoveRoutingQueueEntry(source,destination,timeStamp);

						//Schedule remove with delay
						entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoReplyQueue, this);
						entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
						entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

						entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);

						return true;

					}else
					{
						//Re-Add headers before placing into queue
						packet ->AddHeader(Gh);
						packet ->AddHeader(typeHeader);

						//Cache does not contain and entry with the tuple (Add to cache and start timer on retransmit
						ThesisInternetQueueEntry * entry = new ThesisInternetQueueEntry(packet, header, ucb, ecb, timeStamp);

						//Timer * toRetransmit = entry -> GetTimer();

						//CHANGEHERE

						entry->m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::SendGeoReplyRetransmitVanet, this);
						entry->m_RetransmitTimer.SetArguments(source,destination,timeStamp);
						entry->m_RetransmitTimer.Schedule(backoff);

						m_GeoReplyQueue.AddRoutingEntry(entry);

						entry -> SetInitialDelay(backoff);

						return true;
					}


				}
			}

			return true;
		}else if(theader.Get() == mcast::GEOREPLY_VANET_ACK)
		{
			NS_LOG_INFO("RECV VANET ACK on VANET");
			//Need to clear cached retransmissions
			mcast::TypeHeader ack(mcast::UNKNOWN);
			packet -> RemoveHeader(ack);

			GeoReplyHeader Gh;
			packet -> RemoveHeader(Gh);

			//Schedule removal with delay
			Ipv6Address source = header.GetSourceAddress();
			Ipv6Address destination = header.GetSourceAddress();
			Time sendTime = Gh.GetOriginalTimestamp();


			if(m_GeoReplyQueue.Lookup(source,destination,sendTime))
			{

				ThesisInternetQueueEntry * entry = m_GeoReplyQueue.GetRoutingEntry(source,destination,sendTime);

				if(entry -> m_RetransmitTimer.IsRunning())
				{
				 entry-> m_RetransmitTimer.Cancel();
				}

				//Schedule removal of entry
				entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoReplyQueue, this);
				entry -> m_RetransmitTimer.SetArguments(source,destination,sendTime);
				entry -> m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

				entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);
			}

			return true;
		}
	}

	return false;
}

bool
ThesisInternetRoutingProtocol2::CheckHostBits(Ipv6Address hostAddress)
{
	NS_LOG_FUNCTION(this);
	bool toReturn = true;

	Ipv6Address currentAddress = m_ipv6->GetAddress(m_ipv6->GetInterfaceForDevice(m_wi),1).GetAddress();

	uint8_t currentBytes[16];
	uint8_t hostBytes[16];

	///////////////////////////////////////////////
	//Test Section
	//Print out host and current address bytes

	//uint8_t test[16] = {};
	//hostAddress.GetBytes(test);

	/*
	std::cout << "Printing current address bytes " << currentAddress << std::endl;
	//Get bytes for current address
	currentAddress.GetBytes(currentBytes);
	for(int i = 0; i < 16; i++)
	{
		std::cout << unsigned(currentBytes[i]) << " ";
	}
	std::cout  << std::endl;

	std::cout << "Printing passed host address bytes " << hostAddress << std::endl;
	//Get bytes for passed host address
	hostAddress.GetBytes(hostBytes);
	for(int i = 0; i < 16; i++)
	{
		std::cout << unsigned(hostBytes[i]) << " ";
	}
	std::cout  << std::endl;
	 */
	////////////////////////////////////////////////

	currentAddress.GetBytes(currentBytes);
	hostAddress.GetBytes(hostBytes);


	for(int i = 8; i < 16; i++)
	{
		if(currentBytes[i] != hostBytes[i])
		{
			//			std::cout << "SETTING TORETURN TO FALSE" << std::endl;
			toReturn = false;
			break;
		}
	}

	return toReturn;
}

void
ThesisInternetRoutingProtocol2::SendGeoReplyRetransmitVanet(Ipv6Address source, Ipv6Address destination, Time timestamp)
{
	NS_LOG_FUNCTION(this);

	ThesisInternetQueueEntry * entry = m_GeoReplyQueue.GetRoutingEntry(source,destination,timestamp);
	UnicastForwardCallback ucb = entry -> GetUnicastForwardCallback();

	//Get mobility model properties and extract values needed for header
	Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();
	Vector position = mobility -> GetPosition();
	Vector velocity = mobility -> GetVelocity();

	Ptr<Packet> packet = entry -> GetPacket();

	//Remove headers to modify Internet header - need to change sender parameters
	mcast::TypeHeader theader (mcast::UNKNOWN);
	packet -> RemoveHeader(theader);

	GeoReplyHeader Gh;
	packet -> RemoveHeader(Gh);

	//Update sender position and velocity
	Gh.SetSenderPosition(position);
	Gh.SetSenderVelocity(velocity);

	//Update hop count
	uint8_t newHopCount = Gh.GetHopCount() + 1;
	Gh.SetHopCount(newHopCount);

	//Re-add headers
	packet -> AddHeader(Gh);
	packet -> AddHeader(theader);

	//Create route to send forward packet
	Ptr<Ipv6Route> route = Create<Ipv6Route>();

	route -> SetDestination(destination);
	route -> SetSource(source);
	route -> SetOutputDevice(m_wi);
	route -> SetGateway(Ipv6Address(RSU_TO_VANET));

	if(m_GeoReplyQueue.Lookup(source,destination,timestamp))
	{

		ThesisInternetQueueEntry * entry = m_GeoReplyQueue.GetRoutingEntry(source,destination,timestamp);
		//Schedule removal of entry
		entry-> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoReplyQueue, this);
		entry-> m_RetransmitTimer.SetArguments(source,destination,timestamp);
		entry-> m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

		entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);
	}

	//NS_LOG_INFO(""packet);

	ucb (route -> GetOutputDevice(),route, packet, entry -> GetIpv6Header());

}

void
ThesisInternetRoutingProtocol2::SendInternetRetransmitIntoVanet(Ipv6Address source, Ipv6Address destination, Time timestamp)
{
	NS_LOG_FUNCTION(this);
	//std::cout << "**************** Sending V2VInternet Retransmit ******************" << std::endl;

	ThesisInternetQueueEntry * entry = m_RoutingRtoVCache.GetRoutingEntry(source,destination,timestamp);
	UnicastForwardCallback ucb = entry -> GetUnicastForwardCallback();

	//Get mobility model properties and extract values needed for header
	Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();
	Vector position = mobility -> GetPosition();
	Vector velocity = mobility -> GetVelocity();


	Ptr<Packet> packet = entry -> GetPacket();

	//Remove headers to modify Internet header - need to change sender parameters
	mcast::TypeHeader theader (mcast::UNKNOWN);
	packet -> RemoveHeader(theader);

	ITVHeader itvhdr;
	packet -> RemoveHeader(itvhdr);

	//Update sender position and velocity
	itvhdr.SetSenderPosition(position);
	itvhdr.SetSenderVelocity(velocity);

	//Update hop count
	uint8_t newHopCount = itvhdr.GetHopCount() + 1;
	itvhdr.SetHopCount(newHopCount);

	//Re-add headers
	packet -> AddHeader(itvhdr);
	packet -> AddHeader(theader);

	//Lookup route to send forward packet
	//		Ptr<Ipv6Route> route = Lookup(destination,m_wi);

	Ptr<Ipv6Route> route = Create<Ipv6Route>();

	route -> SetDestination(destination);
	route -> SetSource(source);
	route -> SetOutputDevice(m_wi);
	route -> SetGateway(Ipv6Address(RSU_TO_VANET));

	//Remove entry from cache
	//		m_RoutingCache.RemoveRoutingQueueEntry(source,destination,timestamp);

	//Schedule removal with delay

	if(m_RoutingRtoVCache.Lookup(source,destination,timestamp))
	{

		ThesisInternetQueueEntry * entry = m_RoutingRtoVCache.GetRoutingEntry(source,destination,timestamp);
		//Schedule removal of entry
		entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveThesisRoutingRtVCacheEntry, this);
		entry->m_RetransmitTimer.SetArguments(source,destination,timestamp);
		entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);
	}

	//NS_LOG_INFO(""packet);

	ucb (route -> GetOutputDevice(),route, packet, entry -> GetIpv6Header());
}

bool
ThesisInternetRoutingProtocol2::IsEffectiveV2VTransmission(Vector senderPosition, Vector targetPosition)
{
	NS_LOG_FUNCTION(this);
	bool isEffective = false;

	Ptr<Node> theNode = GetObject<Node>();
	Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

	Vector currentPos = mobility -> GetPosition();

	double currentDistanceToTarget = utils.GetDistanceBetweenPoints(currentPos.x, currentPos.y, targetPosition.x, targetPosition.y);
	double senderDistanceToTarget = utils.GetDistanceBetweenPoints(senderPosition.x, senderPosition.y, targetPosition.x, targetPosition.y);

	//std::cout<<std::endl;
	//std::cout << " Target Position (Type 4) :" << targetPosition << std::endl;
	//std::cout << " V2V Current Distance to Target: " << currentDistanceToTarget << std::endl;
	//std::cout << " V2V Sender Distance to Target: " << senderDistanceToTarget <<std::endl;

	//Check if current node is closer to target than the sending node
	bool isCloserToTargetThanSender = false;
	if(currentDistanceToTarget < senderDistanceToTarget)
	{
		isCloserToTargetThanSender = true;
	}else
	{
		//std::cout << "FOUND INEFFECTIVE TRANSMISSION IN IsEffectiveV2VTransmission" << std::endl;
	}

	//Check if current node is closer to RSU than target point
	//If not its possible the target point was missed and no ACK sent; don't keep transmitting

	double currentDistanceToRsu = utils.GetDistanceBetweenPoints(currentPos.x, currentPos.y, m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y);
	double targetDistanceToRsu = utils.GetDistanceBetweenPoints(targetPosition.x, targetPosition.y, m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y);

	bool isCloserToRsuThanTarget = false;
	if(currentDistanceToRsu < targetDistanceToRsu)
	{
		isCloserToRsuThanTarget = true;
	}

	//True if node is both closer to target than
	isEffective = isCloserToTargetThanSender && isCloserToRsuThanTarget;

	//std::cout<<std::endl;

	return isEffective;
}


void
ThesisInternetRoutingProtocol2::SendAckMessage(Ptr<Packet> ack, UnicastForwardCallback ucb)
{
	NS_LOG_FUNCTION(this);
	//Send out a short ACK msg

	Ptr<Ipv6Route> ackRoute= Create<Ipv6Route> ();
	//ackRoute = Lookup(Ipv6Address(VANET_TO_RSU),m_wi);

	//	std::cout << std::endl;
	//	std::cout << std::endl;
	//	std::cout << std::endl;
	//	std::cout << std::endl;
	//	std::cout << " >>>>>>> OUTPUT ADDRESS FOR ACK: " << m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress() << std::endl;

	Ptr<Ipv6L3Protocol> l3 = GetObject<Ipv6L3Protocol>();

	Ipv6Header ackHdr;
	ackHdr.SetSourceAddress(m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress());
	ackHdr.SetDestinationAddress(Ipv6Address(RSU_TO_VANET));
	ackHdr.SetNextHeader(1);
	ackHdr.SetHopLimit(1);
	ackHdr.SetPayloadLength(ack -> GetSize());
	ackHdr.SetTrafficClass(1);

	/*
			 Ipv6Header hdr;
			 1445
			 1446   hdr.SetSourceAddress (src);
			 1447   hdr.SetDestinationAddress (dst);
			 1448   hdr.SetNextHeader (protocol);
			 1449   hdr.SetPayloadLength (payloadSize);
			 1450   hdr.SetHopLimit (ttl);
			 1451   hdr.SetTrafficClass (tclass);
			 1452   return hdr;
	 */

	ackRoute -> SetDestination(Ipv6Address(VANET_TO_RSU));
	//ackRoute -> SetGateway(Ipv6Address(""));
	ackRoute -> SetOutputDevice(m_wi);
	ackRoute -> SetSource(m_ipv6 -> GetAddress(m_ipv6 -> GetInterfaceForDevice(m_wi),1).GetAddress());

	//	l3 ->Send(ack,ackHdr.GetSourceAddress(), ackHdr.GetDestinationAddress(),1,ackRoute);

	ucb(m_wi,ackRoute,ack,ackHdr);
}

void
ThesisInternetRoutingProtocol2::SendInternetRetransmit(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);

	//std::cout << "**************** Sending Internet Retransmit ******************" << std::endl;

	ThesisInternetQueueEntry * entry = m_RoutingCache.GetRoutingEntry(source,destination,sendTime);
	UnicastForwardCallback ucb = entry -> GetUnicastForwardCallback();

	//Get mobility model properties and extract values needed for header
	Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();
	Vector position = mobility -> GetPosition();
	Vector velocity = mobility -> GetVelocity();


	Ptr<Packet> packet = entry -> GetPacket();

	//Remove headers to modify Internet header - need to change sender parameters
	mcast::TypeHeader theader (mcast::HELLO);
	packet -> RemoveHeader(theader);

	InternetHeader Ih;
	packet -> RemoveHeader(Ih);

	//Update sender position and velocity
	Ih.SetSenderPosition(position);
	Ih.SetSenderVelocity(velocity);

	//Update hop count
	uint8_t newHopCount = Ih.GetHopCount() + 1;
	Ih.SetHopCount(newHopCount);

	//Re-add headers
	packet -> AddHeader(Ih);
	packet -> AddHeader(theader);

	//Lookup route to send forward packet
	Ptr<Ipv6Route> route = Lookup(destination,m_wi);

	//Remove entry from cache
	//	m_RoutingCache.RemoveRoutingQueueEntry(source,destination,sendTime);

	//Schedule removal of entry
	entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveThesisRoutingCacheEntry, this);
	entry->m_RetransmitTimer.SetArguments(source,destination,sendTime);
	entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

	ucb (route -> GetOutputDevice(),route, packet, entry -> GetIpv6Header());
}

void
ThesisInternetRoutingProtocol2::SendGeoQueryReply(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);

	//std::cout << "SENDING GEO QUERY REPLY ACK AND STARTING PROCESS TO REPLY" << std::endl;

	ThesisInternetQueueEntry * entry = m_GeoQueryReplyCache.GetRoutingEntry(source,destination,sendTime);
	UnicastForwardCallback ucb = entry -> GetUnicastForwardCallback();
	LocalDeliverCallback lcb = entry -> GetLocalDeliveryCallback();

//	std::cout << source << std::endl;
//	std::cout << destination << std::endl;
//	std::cout << sendTime << std::endl;
//	std::cout << entry -> GetPacketSendTime() << std::endl;

	/*
	//Get mobility model properties and extract values needed for header
	Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();
	Vector position = mobility -> GetPosition();
	Vector velocity = mobility -> GetVelocity();
	 */

	Ptr<Packet> packet = entry -> GetPacket();

	//Remove headers to modify Internet header - need to change sender parameters
	mcast::TypeHeader theader (mcast::UNKNOWN);
	packet -> RemoveHeader(theader);

	GeoRequestHeader Gh;
	packet -> RemoveHeader(Gh);


	//Need to send an ACK


	Ptr<Packet> ackPacket = Create<Packet> ();

	mcast::TypeHeader ackHeader (mcast::GEOQUERY_SENDINGREPLY);

	ackPacket -> AddHeader(Gh);
	ackPacket -> AddHeader(ackHeader);

	Ptr<Ipv6Route> ackEntry = Create<Ipv6Route> ();
	ackEntry -> SetDestination(Ipv6Address(VANET_TO_RSU));
	ackEntry -> SetGateway(Ipv6Address(VANET_TO_RSU));
	ackEntry -> SetOutputDevice(m_wi);
	ackEntry -> SetSource(Ipv6Address(VANET_TO_RSU));

	//Send Ack Message
	ucb (m_wi,ackEntry, ackPacket, entry -> GetIpv6Header());

	//Need to deliver to server

	NS_LOG_INFO("Current Address: " << m_ipv6 ->GetAddress(1,1).GetAddress());

	Ipv6Header toSendUp;

	toSendUp.SetDestinationAddress(m_ipv6 ->GetAddress(1,1).GetAddress());
	toSendUp.SetFlowLabel(entry->GetIpv6Header().GetFlowLabel());
	toSendUp.SetHopLimit(entry->GetIpv6Header().GetHopLimit());
	toSendUp.SetNextHeader(entry->GetIpv6Header().GetNextHeader());
	toSendUp.SetPayloadLength(entry->GetIpv6Header().GetPayloadLength());
	toSendUp.SetSourceAddress(entry->GetIpv6Header().GetSourceAddress());
	toSendUp.SetTrafficClass(entry->GetIpv6Header().GetTrafficClass());

	//lcb (packet, entry->GetIpv6Header(), m_ipv6->GetInterfaceForDevice(m_wi));
	lcb (packet, toSendUp , m_ipv6->GetInterfaceForDevice(m_wi));

}

void
ThesisInternetRoutingProtocol2::SendGeoQueryRetransmit(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);

	//std::cout << "**************** Sending Internet Retransmit ******************" << std::endl;

	ThesisInternetQueueEntry * entry = m_GeoRoutingQueue.GetRoutingEntry(source,destination,sendTime);
	UnicastForwardCallback ucb = entry -> GetUnicastForwardCallback();

	//Get mobility model properties and extract values needed for header
	Ptr<MobilityModel> mobility = m_ipv6 -> GetObject<MobilityModel>();
	Vector position = mobility -> GetPosition();
	Vector velocity = mobility -> GetVelocity();

	Ptr<Packet> packet = entry -> GetPacket();

	//Remove headers to modify Internet header - need to change sender parameters
	mcast::TypeHeader theader (mcast::UNKNOWN);
	packet -> RemoveHeader(theader);

	GeoRequestHeader Gh;
	packet -> RemoveHeader(Gh);

	//Update sender position and velocity
	Gh.SetSenderPosition(position);
	Gh.SetSenderVelocity(velocity);

	//Update hop count
	uint8_t newHopCount = Gh.GetHopCount() + 1;
	Gh.SetHopCount(newHopCount);

	//Re-add headers
	packet -> AddHeader(Gh);
	packet -> AddHeader(theader);

	//Lookup route to send forward packet
	Ptr<Ipv6Route> route = Lookup(destination,m_wi);

	//Remove entry from cache
	//	m_RoutingCache.RemoveRoutingQueueEntry(source,destination,sendTime);

	//Schedule removal of entry
	entry -> m_RetransmitTimer.SetFunction(&ThesisInternetRoutingProtocol2::RemoveGeoRoutingCacheEntry, this);
	entry->m_RetransmitTimer.SetArguments(source,destination,sendTime);
	entry->m_RetransmitTimer.Schedule(m_ThesisInternetRoutingCacheCooldown);

	ucb (route -> GetOutputDevice(),route, packet, entry -> GetIpv6Header());
	entry -> SetInitialDelay(m_ThesisInternetRoutingCacheCooldown);
}

void
ThesisInternetRoutingProtocol2::RemoveThesisRoutingCacheEntry(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);
	m_RoutingCache.RemoveRoutingQueueEntry(source,destination,sendTime);
}

void
ThesisInternetRoutingProtocol2::RemoveGeoRoutingCacheEntry(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);
	m_GeoRoutingQueue.RemoveRoutingQueueEntry(source,destination,sendTime);
}

void
ThesisInternetRoutingProtocol2::RemoveGeoQueryReplyCacheEntry(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);
	m_GeoQueryReplyCache.RemoveRoutingQueueEntry(source,destination,sendTime);
}

void
ThesisInternetRoutingProtocol2::RemoveThesisRoutingRtVCacheEntry(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);
	m_RoutingRtoVCache.RemoveRoutingQueueEntry(source,destination,sendTime);
}

void
ThesisInternetRoutingProtocol2::RemoveGeoReplyQueue(Ipv6Address source, Ipv6Address destination, Time sendTime)
{
	NS_LOG_FUNCTION(this);
	m_GeoReplyQueue.RemoveRoutingQueueEntry(source,destination,sendTime);
}

Ptr<Ipv6Route>
ThesisInternetRoutingProtocol2::RouteOutput (Ptr<Packet> p, const Ipv6Header &header, Ptr<NetDevice> oif,
		Socket::SocketErrno &sockerr)
{

	//std::cout << "ThesisInternet2 route output" << std::endl;

	NS_LOG_FUNCTION("   " << this << header << oif);

	Ipv6Address destination = header.GetDestinationAddress();
	Ipv6Address source = header.GetSourceAddress();

	//	std::cout << "Route output: Destination: " << destination << " Source: " << source << " Is RSU? " << m_IsRSU << std::endl;

	sockerr = Socket::ERROR_NOTERROR;
	Ptr<Ipv6Route> route;

	route = Lookup(destination,oif);

	//Found a route, don't send yet though
	//Send to loopback interface to allow packet to correctly form
	//Recieve packet from loopback interface in RouteInput and then add headers or
	//make routing decisions

	if(route)
	{
		//Found a valid route (Send to loopback for further processing)
		uint32_t iif = m_ipv6->GetInterfaceForAddress(m_ipv6 -> GetAddress(1,1).GetAddress());
		DeferredRouteOutputTag tag(iif);
		NS_LOG_LOGIC("Route found - adding deferred tag to allow processing at input");
		if(!p->PeekPacketTag(tag))
		{
			//			std::cout << "Peek finished - No tag" << std::endl;
			p -> AddPacketTag(tag);
		}

		//		std::cout << "Added packet tag" << std::endl;

		//return Lookup(Ipv6Address::GetLoopback(),oif);
		/*
		std::cout << Simulator::Now() << " >>>>>>>>>> ROUTE OUTPUT RETURNING LOOPBACK ROUTE WITH FOLLOWING PROPERTIES <<<<<<<<<<<<<<<<<<" << std::endl;

		std::cout << "Output Interface (OIF): " << m_ipv6->GetInterfaceForDevice(oif) << std::endl;
		std::cout << "Original destination: " << destination << std::endl;
		std::cout << "Original source     : " << source << std::endl;
		 */
		Ptr<Ipv6Route> rtentry = Create<Ipv6Route> ();
		rtentry -> SetDestination(destination);
		rtentry -> SetGateway(Ipv6Address::GetLoopback());
		rtentry -> SetOutputDevice(m_lo);
		rtentry -> SetSource(m_ipv6 -> GetAddress(1,1).GetAddress());
		/*
		std::cout << "Destination:  " << rtentry ->GetDestination() << std::endl;
		std::cout << "Gateway       " << rtentry-> GetGateway() << std::endl;
		std::cout << "Source        " << rtentry -> GetSource() << std::endl;
		std::cout << "Output Device " << rtentry -> GetOutputDevice() -> GetIfIndex() << std::endl;

		std::cout << ">>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
		 */
		/*
		Transmission t;
		if(destination.IsMulticast())
		{
			std::cout << "RouteInput sending to multicast (Shouldn't happen)" << std::endl;
		}

		t.Destination = destination;
		t.SendTime = Simulator::Now();
		m_sourcedTrans.push_back(t);
		m_numSourced++;
		 */
		return rtentry;
	}else
	{
		//No valid route route - drop
		sockerr = Socket::ERROR_NOROUTETOHOST;
		return route;
	}
}

Ptr<Ipv6Route>
ThesisInternetRoutingProtocol2::Lookup(Ipv6Address destination, Ptr<NetDevice> interface)
{
	NS_LOG_FUNCTION (this << destination << interface);

	Ptr<Ipv6Route> rtentry = 0;
	uint16_t longestMask = 0;

	for(RoutesI it = m_routes.begin(); it != m_routes.end(); it++)
	{
		ThesisInternetRoutingTableEntry2* j = it -> first;

		Ipv6Prefix mask = j->GetDestNetworkPrefix ();
		uint16_t maskLen = mask.GetPrefixLength ();
		Ipv6Address entry = j->GetDestNetwork ();

		NS_LOG_LOGIC ("Searching for route to " << destination << ", mask length " << maskLen);

		if(mask.IsMatch(destination,entry))
		{
			NS_LOG_LOGIC ("Found global network route " << j << ", mask length " << maskLen);

			/* if interface is given, check the route will output on this interface */
			if (!interface || interface == m_ipv6->GetNetDevice (j->GetInterface ()))
			{
				if (maskLen < longestMask)
				{
					NS_LOG_LOGIC ("Previous match longer, skipping");
					continue;
				}
			}
			longestMask = maskLen;

			m_RsuDestination = j->GetRsuAddress();

			Ipv6RoutingTableEntry* route = j;
			uint32_t interfaceIdx = route->GetInterface ();
			rtentry = Create<Ipv6Route> ();

			rtentry->SetSource(m_ipv6->SourceAddressSelection (interfaceIdx, route->GetDest ()));
			rtentry->SetDestination (destination);
			rtentry->SetGateway (route->GetGateway ());
			rtentry->SetOutputDevice (m_ipv6->GetNetDevice (interfaceIdx));
		}
	}

	if (rtentry)
	{
		/*
		//NS_LOG_LOGIC ("Matching route to " << rtentry->GetDestination () << " (through " << rtentry->GetGateway () << ") at the end");
		std::cout << "Route found, printing route properties: " << std::endl;
		std::cout << "Destination:  " << rtentry->GetDestination() << std::endl;
		std::cout << "Gateway       " << rtentry-> GetGateway() << std::endl;
		std::cout << "Source        " << rtentry-> GetSource() << std::endl;
		std::cout << "Reference     " << rtentry->GetReferenceCount() << std::endl;
		std::cout << "Output Device " << rtentry-> GetOutputDevice() -> GetIfIndex() << std::endl;
		 */
	}
	return rtentry;
}
/*
 ************NEEDS TO BE UPDATED******************
 */
void
ThesisInternetRoutingProtocol2::NotifyInterfaceUp (uint32_t interface)
{
	NS_LOG_FUNCTION(this << interface << "  RUNNING");
	for(uint32_t j=0; j < m_ipv6->GetNAddresses(interface); j++)
	{
		Ipv6InterfaceAddress address = m_ipv6->GetAddress(interface,j);
		Ipv6Prefix networkMask = address.GetPrefix ();
		Ipv6Address networkAddress = address.GetAddress ().CombinePrefix (networkMask);
		if (address != Ipv6Address () && networkMask != Ipv6Prefix ())
		{
			AddNetworkRouteTo (networkAddress, networkMask, interface);
		}
	}
}

void
ThesisInternetRoutingProtocol2::NotifyInterfaceDown (uint32_t interface)
{
	NS_LOG_FUNCTION(this << interface << "Int Down");


	RoutesI it = m_routes.begin ();
	while (it != m_routes.end ())
	{
		if(it -> first ->GetInterface() == interface)
		{
			it = m_routes.erase(it);
		}else
		{
			it++;
		}
	}

}

void
ThesisInternetRoutingProtocol2::DeleteRoute(ThesisInternetRoutingTableEntry2 *route)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyAddAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyRemoveAddress (uint32_t interface, Ipv6InterfaceAddress address)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyAddRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
		uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::NotifyRemoveRoute (Ipv6Address dst, Ipv6Prefix mask, Ipv6Address nextHop,
		uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION(this);
}

void
ThesisInternetRoutingProtocol2::SetIpv6 (Ptr<Ipv6> ipv6)
{
	//	std::cout << "Setting IPv6 in routing protocol \n";

	NS_LOG_FUNCTION (this << ipv6);
	NS_ASSERT (m_ipv6 == 0 && ipv6 != 0);
	uint32_t i = 0;
	m_ipv6 = ipv6;

	for (i = 0; i < m_ipv6->GetNInterfaces (); i++)
	{
		if (m_ipv6->IsUp (i))
		{
			NS_LOG_LOGIC("IPv6 interface up, notify interface up");
			NotifyInterfaceUp (i);
		}
		else
		{
			NS_LOG_LOGIC("IPv6 interface down, notify interface down");
			NotifyInterfaceDown (i);
		}
	}

	//Set pointer to loopback netdevice
	m_lo = m_ipv6 -> GetNetDevice(m_ipv6 -> GetInterfaceForAddress(Ipv6Address::GetLoopback()));

	//Create loopback route
	AddNetworkRouteTo (Ipv6Address::GetLoopback(), Ipv6Prefix::GetOnes(), m_ipv6 -> GetInterfaceForAddress(Ipv6Address::GetLoopback()));

}

void
ThesisInternetRoutingProtocol2::SetRsuDatabase(Ptr<Db> db)
{
	NS_LOG_FUNCTION(this);
	m_Db = db;
}

void
ThesisInternetRoutingProtocol2::SetIsRSU(bool isRSU)
{
	m_IsRSU = isRSU;
}

void
ThesisInternetRoutingProtocol2::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
{
	NS_LOG_FUNCTION (this << stream);
	std::ostream* os = stream->GetStream ();

	*os << "Node: " << m_ipv6->GetObject<Node> ()->GetId ()
	      				<< " Time: " << Simulator::Now ().GetSeconds () << "s "
	      				<< "Ipv6 Thesis table" << std::endl;

	if (!m_routes.empty ())
	{
		*os << "Destination                    Next Hop                   Flag Met Ref Use If" << std::endl;
		for (RoutesIC it = m_routes.begin (); it != m_routes.end (); it++)
		{
			ThesisInternetRoutingTableEntry2* route = it->first;
			//RipNgRoutingTableEntry::Status_e status = route->GetRouteStatus();

			if (true)
			{
				std::ostringstream dest, gw, mask;

				dest << route->GetDest () << "/" << int(route->GetDestNetworkPrefix ().GetPrefixLength ());
				*os << std::setiosflags (std::ios::left) << std::setw (31) << dest.str ();
				gw << route->GetGateway ();
				*os << std::setiosflags (std::ios::left) << std::setw (27) << gw.str ();
				*os << std::setiosflags (std::ios::left) << std::setw (4) << int(route->GetRouteMetric ());
				// Ref ct not implemented
				*os << "-" << "   ";
				// Use not implemented
				*os << "-" << "   ";
				*os << route->GetInterface ();
				*os << std::endl;
			}
		}
	}
}

void
ThesisInternetRoutingProtocol2::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);

	//	ThesisInternetRoutingTableEntry* route = new ThesisInternetRoutingTableEntry (network, networkPrefix, interface);
	ThesisInternetRoutingTableEntry2* route = new ThesisInternetRoutingTableEntry2(network, networkPrefix,nextHop,interface,prefixToUse);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry2::ROUTE_VALID);
	route->SetRouteChanged (true);

	m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisInternetRoutingProtocol2::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);
	ThesisInternetRoutingTableEntry2* route = new ThesisInternetRoutingTableEntry2 (network, networkPrefix, interface);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry2::ROUTE_VALID);
	route->SetRouteChanged (true);

	m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisInternetRoutingProtocol2::AddNetworkRouteTo(Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse, Ipv6Address RsuAddress)
{
	NS_LOG_FUNCTION (this << network << networkPrefix << interface);

	//	ThesisInternetRoutingTableEntry* route = new ThesisInternetRoutingTableEntry (network, networkPrefix, interface);
	ThesisInternetRoutingTableEntry2* route = new ThesisInternetRoutingTableEntry2(network, networkPrefix,nextHop,interface,prefixToUse);
	route->SetRouteMetric (1);
	route->SetRouteStatus (ThesisInternetRoutingTableEntry2::ROUTE_VALID);
	route->SetRouteChanged (true);
	route->SetRsuAddress(RsuAddress);

	m_routes.push_back (std::make_pair (route, EventId ()));
}

void
ThesisInternetRoutingProtocol2::DoDispose()
{
	//Dispose; update later?
}

void
ThesisInternetRoutingProtocol2::DoInitialize()
{
	//std::cout << "rWait: " << m_rWait << std::endl;
	if(!m_IsRSU)
	{
		SetIpToZone();

		m_wi = m_ipv6 -> GetNetDevice(GetWirelessInterface());


		///Geo Server
		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		Ptr<Node> node = GetObject<Node>();

		m_GeoServerSocket = Socket::CreateSocket (node, tid);

		Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_GeoServerPort);
		m_GeoServerSocket->Bind (local6);
		m_GeoServerSocket->SetRecvCallback (MakeCallback (&ThesisInternetRoutingProtocol2::HandleGeoRead, this));
		///////
	}else if (m_IsRSU)
	{
		SetInterfacePointers();

	}

	Ipv6RoutingProtocol::DoInitialize ();

	if(m_IsRSU)
	{
		SetStaticRoutePointer();

		//Create a route to RSU_TO_VANET address
		AddNetworkRouteTo(Ipv6Address(RSU_TO_VANET),Ipv6Prefix::GetOnes(),Ipv6Address(RSU_TO_VANET),m_ipv6->GetInterfaceForDevice(m_wi),Ipv6Address ("::"));

	}

}

void
ThesisInternetRoutingProtocol2::HandleGeoRead(Ptr<Socket> socket)
{
	NS_LOG_FUNCTION(this);

	Ptr<Packet> packet;
	Address from;


	while ((packet = socket->RecvFrom (from)))
	{
		Ipv6Address toSend = Inet6SocketAddress::ConvertFrom(from).GetIpv6();
		//std::cout << "1 GeoQuery Server received packet from: " << toSend << std::endl;
		//std::cout << "2 GeoQuery Server received packet from: " << toSend << std::endl;
		//std::cout << "3 GeoQuery Server received packet from: " << toSend << std::endl;




		if (InetSocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
					InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
					InetSocketAddress::ConvertFrom (from).GetPort ());
		}
		else if (Inet6SocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
					Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
					Inet6SocketAddress::ConvertFrom (from).GetPort ());
		}

		packet->RemoveAllPacketTags ();
		packet->RemoveAllByteTags ();

		NS_LOG_LOGIC ("Echoing packet");
		//int sendResult = socket->SendTo (packet, 0, from);
		socket->SendTo (packet, 0, from);

		if (InetSocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
					InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
					InetSocketAddress::ConvertFrom (from).GetPort ());
		}
		else if (Inet6SocketAddress::IsMatchingType (from))
		{
			NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
					Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
					Inet6SocketAddress::ConvertFrom (from).GetPort ());
		}

		//std::cout << "Server attempted to send reply to " << from  << " ToSend: " << toSend << std::endl;
		//std::cout << "Socket send result: " << sendResult << std::endl;
		//std::cout << "" << std::endl;
	}


}

void
ThesisInternetRoutingProtocol2::SetIpToZone()
{
	NS_LOG_FUNCTION(this);

	Ptr<Node> theNode = GetObject<Node> ();
	Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();
	Vector position = mobility -> GetPosition();

	//	std::cout << "Node position: " << position << std::endl;
	DbEntry t1 = m_Db -> GetEntryForCurrentPosition(position);

	//Set pointer to current RSU DB entry (For convenience)
	m_currentRsu = t1;

	Ipv6Address network = t1.GetRsuAddress().CombinePrefix(Ipv6Prefix(64));

	//	std::cout << "Nearest RSU position: " << t1.GetRsuPosition() << std::endl;
	//	std::cout << "Vanet node network based on position: " << network << std::endl;


	Ptr<Ipv6L3Protocol> l3 = theNode ->GetObject<Ipv6L3Protocol>();

	//Try to use Ipv6L3Protocol instead of ipv6 to reset interface addresses

	for(uint32_t i = 0; i < m_ipv6 ->GetNInterfaces();i++)
	{
		for(uint32_t j = 0; j < m_ipv6 ->GetNAddresses(i);j++)
		{
			Ipv6Address currentAdd = m_ipv6 -> GetAddress(i,j).GetAddress();
			if(!currentAdd.IsLinkLocal() && !currentAdd.IsLocalhost())
			{
				if(!(currentAdd.CombinePrefix(Ipv6Prefix(64)).IsEqual(network)))
				{
					Ptr<WifiNetDevice> wifi = DynamicCast<WifiNetDevice>(m_ipv6 ->GetNetDevice(i));
					Mac48Address mac = wifi ->GetMac() ->GetAddress();

					Ipv6Address newAddress;
					newAddress = newAddress.MakeAutoconfiguredAddress(mac,network);
					//					std::cout << "New Address: " << newAddress << std::endl;
					//					std::cout << "" << std::endl;

					//wifi -> GetMac() -> Get

					/*
					 * 0. Remove all routes to this interface
					 * 1. Get interface index for device
					 * 2. Remove old address
					 * 3. Add new address to interface
					 * 4. Set interface to up
					 * 5. Add default route
					 * 6. Set loopback to up
					 * 7. Add internet routes to this interface
					 */

					NotifyInterfaceDown(i);

					//					std::cout << "Remove Address" << " interface = " << i << " index= " << j << std::endl;
					m_ipv6 ->RemoveAddress(i,j);

					//					std::cout << "Adding new address: " << newAddress << std::endl;
					m_ipv6 -> AddAddress(i,newAddress);

					//					std::cout << "Setting interface " << i << " to up" << std::endl;
					m_ipv6 -> SetUp(i);

					Ipv6Address RsuAddress = t1.GetRsuAddress();
					//std::cout << "Adding default route, next hop:  " << nextHop << std::endl;
					AddNetworkRouteTo(Ipv6Address("::"),Ipv6Prefix::GetZero(),Ipv6Address(VANET_TO_RSU),i,Ipv6Address ("::"),RsuAddress);

					//					std::cout << "Setting loopback to up "<< std::endl;
					m_ipv6 -> SetUp(0);

					NotifyInterfaceUp(i);

					Ptr<Icmpv6L4Protocol> icmpv6 = l3 ->GetIcmpv6();
					Ptr<NdiscCache> nCache = icmpv6 -> FindCache(m_ipv6->GetNetDevice(i));

					NdiscCache::Entry* nEntry = nCache -> Lookup(RsuAddress);

					if(nEntry)
					{
						//		      	std::cout << "Neighbor Entry FOUND!!!!" << std::endl;
					}else
					{
						//gateway neighbor not in cache; manually add to cache
						//		      	std::cout << "Neighbor Entry NOT FOUND!!!!" << std::endl;

						nEntry = nCache -> Add(RsuAddress);

						nEntry -> SetRouter(true);
						nEntry -> SetMacAddress(t1.GetRsuMacAddress());
						nEntry -> MarkReachable();

					}

					//					std::cout << "Address refreshed to current zone" << std::endl;

					//GeoServer Socket Management
					//Only perform management if its been initialized already
					if(m_GeoServerSocket != 0)
					{
						Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_GeoServerPort);
						m_GeoServerSocket->Bind (local6);
						m_GeoServerSocket->SetRecvCallback (MakeCallback (&ThesisInternetRoutingProtocol2::HandleGeoRead, this));
					}
					break;
				}
			}
		}
	}




	m_CheckPositionTimer.SetFunction(&ThesisInternetRoutingProtocol2::SetIpToZone,this);
	m_CheckPositionTimer.Schedule(m_CheckPosition);
}

void
ThesisInternetRoutingProtocol2::SetInterfacePointers()
{
	NS_LOG_FUNCTION(this);

	int32_t interface = -1;

	interface = GetP2pInterface();
	NS_ASSERT(interface > -1);
	m_pp = m_ipv6 ->GetNetDevice(interface);


	//	std::cout << "First wifi interface " << interface << std::endl;

	interface = -1;
	interface = GetWirelessInterface();
	NS_ASSERT(interface > -1);
	m_wi = m_ipv6 -> GetNetDevice(interface);

	//	std::cout << "First physical interface " << interface << std::endl;

}

int32_t
ThesisInternetRoutingProtocol2::GetP2pInterface()
{
	uint32_t interface = -1;

	PointToPointNetDevice p2pd;

	for(uint32_t j = 0; j < m_ipv6 -> GetNInterfaces(); j++)
	{
		if(m_ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(p2pd.GetTypeId().GetName()) == 0)
		{
			interface = j;
			break;
		}
	}
	return interface;
}

int32_t
ThesisInternetRoutingProtocol2::GetWirelessInterface()
{
	uint32_t interface = -1;
	WifiNetDevice wifi;

	for(uint32_t j = 0; j < m_ipv6 -> GetNInterfaces(); j++)
	{
		if(m_ipv6->GetNetDevice(j)->GetInstanceTypeId().GetName().compare(wifi.GetTypeId().GetName()) == 0)
		{
			interface = j;
			break;
		}
	}
	return interface;
}

void
ThesisInternetRoutingProtocol2::SetStaticRoutePointer()
{
	Ptr<Node> theNode = GetObject<Node> ();
	Ptr<Ipv6ListRouting> lr = DynamicCast<Ipv6ListRouting>(m_ipv6 -> GetRoutingProtocol());
	Ipv6StaticRouting s;

	int16_t interface;
	for(uint32_t i = 0; i < lr ->GetNRoutingProtocols(); i++)
	{
		if(lr -> GetRoutingProtocol(i,interface) -> GetInstanceTypeId().GetName().compare(s.GetTypeId().GetName()) == 0)
		{
			m_sr6 = DynamicCast<Ipv6StaticRouting>(lr -> GetRoutingProtocol(i,interface));
			break;
		}
	}

}

void
ThesisInternetRoutingProtocol2::RemoveDefaultRoute()
{
	NS_LOG_FUNCTION(this);

	for(RoutesI it = m_routes.begin(); it != m_routes.end(); it++)
	{
		Ipv6Address destination = it -> first -> GetDestNetwork();

		Ipv6Address defaultRouteAddress("::");

		//		std::cout << " >>>>>>>>>>>>>>>> Attempting to remove route with destination: <<<<<<<<<<<<<" << destination << std::endl;

		if(destination.IsEqual(defaultRouteAddress))
		{
			//			std::cout << " >>>>>>>>>>>>>>>> Attempting to remove route with destination: <<<<<<<<<<<<<" << destination << std::endl;
			it = m_routes.erase(it);
			return;
		}

	}

}

Time
ThesisInternetRoutingProtocol2::GetBackoffDuration(Vector SenderPosition)
{
	Time toWait;

	Ptr<Node> theNode = GetObject<Node>();
	Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

	Vector currentPos = mobility -> GetPosition();

	double currentDistanceToRsu = utils.GetDistanceBetweenPoints(currentPos.x, currentPos.y, m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y);
	double senderDistanceToRsu = utils.GetDistanceBetweenPoints(SenderPosition.x, SenderPosition.y, m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y);


	double ratio = currentDistanceToRsu/senderDistanceToRsu;
	ratio = ceil(ratio * 100);

	//Backoff in microseconds
	toWait = MicroSeconds(ratio * m_rWait);

	//std::cout << "Waiting time in microseconds (Internet retransmit): " << toWait.GetMicroSeconds() << std::endl;

	return toWait;
}

Time
ThesisInternetRoutingProtocol2::GetV2VBackoffDuration(Vector senderPosition, Vector targetPosition)
{
	Time toWait;

	Ptr<Node> theNode = GetObject<Node>();
	Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

	Vector currentPos = mobility -> GetPosition();

	double currentDistanceToTarget = utils.GetDistanceBetweenPoints(currentPos.x, currentPos.y, targetPosition.x, targetPosition.y);
	double senderDistanceToTarget = utils.GetDistanceBetweenPoints(senderPosition.x, senderPosition.y, targetPosition.x, targetPosition.y);

	double ratio = currentDistanceToTarget/senderDistanceToTarget;
	ratio = ceil(ratio * 100);

	//Backoff in microseconds
	toWait = MicroSeconds(ratio * m_rWait);

	return toWait;
}

bool
ThesisInternetRoutingProtocol2::IsEffective(Vector SenderPosition)
{
	bool isEffective = false;

	Ptr<Node> theNode = GetObject<Node>();
	Ptr<MobilityModel> mobility = theNode -> GetObject<MobilityModel>();

	Vector currentPos = mobility -> GetPosition();

	double currentDistanceToRsu = utils.GetDistanceBetweenPoints(currentPos.x, currentPos.y, m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y);
	double senderDistanceToRsu = utils.GetDistanceBetweenPoints(SenderPosition.x, SenderPosition.y, m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y);

	//	std::cout << "Current Distance to RSU: " << currentDistanceToRsu << std::endl;
	//	std::cout << "Sender Distance to RSU: " << senderDistanceToRsu <<std::endl;

	if(currentDistanceToRsu < senderDistanceToRsu)
	{
		isEffective = true;
	}else
	{
		//		std::cout << "Ineffective transmission detected; current distance further from RSU than sending node" << std::endl;
	}

/*
	double SenderDistanceToRsu = utils.GetDistanceBetweenPoints(SenderPosition.x, SenderPosition.y, m_currentRsu.GetRsuPosition().x, m_currentRsu.GetRsuPosition().y);
	double SenderDistanceToCurrent = utils.GetDistanceBetweenPoints(SenderPosition.x, SenderPosition.y, currentPos.x, currentPos.y);

	bool isCloserThanSenderCorrectSide = false;
	if(SenderDistanceToRsu < SenderDistanceToCurrent)
	{
		isCloserThanSenderCorrectSide = true;
	}
*/

	return isEffective /*&& isCloserThanSenderCorrectSide*/;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 ()
: m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{

}

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse)
: Ipv6RoutingTableEntry ( ThesisInternetRoutingTableEntry2::CreateNetworkRouteTo (network, networkPrefix, nextHop, interface, prefixToUse) ),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, uint32_t interface)
: Ipv6RoutingTableEntry ( Ipv6RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, interface) ),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false)
{
}

ThesisInternetRoutingTableEntry2::ThesisInternetRoutingTableEntry2 (Ipv6Address network, Ipv6Prefix networkPrefix, Ipv6Address nextHop, uint32_t interface, Ipv6Address prefixToUse, Ipv6Address RsuAddress)
: Ipv6RoutingTableEntry ( Ipv6RoutingTableEntry::CreateNetworkRouteTo (network, networkPrefix, interface)),
  m_tag (0), m_metric (16), m_status (ROUTE_INVALID), m_changed (false),m_RsuAddress(RsuAddress)
{
}

ThesisInternetRoutingTableEntry2::~ThesisInternetRoutingTableEntry2 ()
{
}

void
ThesisInternetRoutingTableEntry2::SetRouteTag (uint16_t routeTag)
{
	m_tag = routeTag;
}

void
ThesisInternetRoutingTableEntry2::SetRouteStatus(Status_e status)
{
	m_status = status;
}

void
ThesisInternetRoutingTableEntry2::SetRouteChanged (bool changed)
{
	m_changed = changed;
}

bool
ThesisInternetRoutingTableEntry2::IsRouteChanged (void) const
{
	return m_changed;
}

void
ThesisInternetRoutingTableEntry2::SetRouteMetric (uint8_t routeMetric)
{
	m_metric = routeMetric;
}

uint8_t
ThesisInternetRoutingTableEntry2::GetRouteMetric (void) const
{
	return m_metric;
}

void
ThesisInternetRoutingTableEntry2::SetRsuAddress (Ipv6Address RsuAddress)
{
	m_RsuAddress = RsuAddress;
}

Ipv6Address
ThesisInternetRoutingTableEntry2::GetRsuAddress (void) const
{
	return m_RsuAddress;
}



}
}

