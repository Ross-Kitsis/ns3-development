/*
 * mcast-utils.h
 *
 *  Created on: Dec 13, 2015
 *      Author: ross
 */

#ifndef MCAST_UTILS_H_
#define MCAST_UTILS_H_


#include "ns3/ipv6.h"
#include <math.h>
#include "ns3/vector.h"

namespace ns3
{
	namespace mcast
	{
		class McastUtils
		{
		public:

			static const double LaneWidth;
			static const double VehLength;

			McastUtils();
			~McastUtils();

			//Functions

		  double GetDistanceBetweenPoints(double x1, double y1, double x2, double y2);

		  /**
		   * \brief Generated and returns an a value for the mcast apex (Right)
		   */

		  Vector GetApexR(Vector velocity, Vector position, double dist);

		  /**
		   * \brief Generated and returns an a value for the mcast apex
		   */

		  Vector GetApexL(Vector velocity, Vector position, double dist);

		  /**
		   * \brief get A value
		   */
		  double getA(Vector velocity, Vector position);

		  /**
		   * \brief get B value
		   */
		  double GetB();

		  /**
		   * Check if node is within the ZoR
		   *
		   * Returns true if the node is within the ZoR; false otherwise
		   */
		  bool IsInZor(Vector position, Vector eventPos,double a, double b);

		};
	}
}
#endif /* SRC_MCAST_MODEL_MCAST_UTILS_H_ */
