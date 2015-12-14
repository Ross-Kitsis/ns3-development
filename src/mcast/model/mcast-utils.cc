/*
 * mcast-utils.cc
 *
 *  Created on: Dec 13, 2015
 *      Author: ross
 */

#include "mcast-utils.h"
#include "ns3/log.h"


namespace ns3
{
	NS_LOG_COMPONENT_DEFINE("McastControlUtities");
	namespace mcast
	{
		const double McastUtils::LaneWidth = 3.5;
		const double McastUtils::VehLength = 4.5;

		McastUtils::McastUtils()
		{

		}

		McastUtils::~McastUtils()
		{

		}

		double
		McastUtils::GetDistanceBetweenPoints(double x1, double y1, double x2, double y2)
		{
			NS_LOG_FUNCTION(this);
			double distance = 0;

			distance = sqrt(pow(x1-x2,2) + pow(y1-y2,2));

			return distance;
		}

		double
		McastUtils::getA(Vector vel, Vector pos)
		{
			double toReturn = 0;

			double dA = vel.x;
			double dB = vel.y;

			if(dA < 0)
			{
				dA = dA * -1;
			}

			if(dB < 0)
			{
				dB = dB*-1;
			}

			toReturn = sqrt(pow(dA,2) + pow(dB,2)) * 1/10 * VehLength;

			return toReturn;
		}

		double
		McastUtils::GetB()
		{
			//Assume lanes on each side of road
			return LaneWidth * 4;
		}

		Vector
		McastUtils::GetApexL(Vector velocity, Vector position, double dist)
		{
			//Return the positive answer of the equation quadratic equation
			Vector toReturn;

			/////////// Get the equation of a the line for slope and intercept
//			double x1 = position.x;
//			double y1 = position.y;

			double m;
			double yint;
			if(velocity.x == 0)
			{
				m = 1;
				yint = 0;
			}else
			{
				m = velocity.y/velocity.x;
				yint = position.y - (m * position.x);
			}

			double p = position.x;
			double q = position.y;

			//Set a,b,c values for quadratic equation

			double a = pow(m,2) + 1;
			double b = (2*yint*m) - (2*p) - (2*m*q);
			double c = pow(p,2) + pow(yint,2) - (2*yint*q) + pow(q,2) +- pow(dist, 2);

			toReturn.x = (-b + sqrt(pow(b,2) - 4*a*c))/2*a;
			toReturn.y = m * toReturn.x + b;

			return toReturn;
		}

		Vector
		McastUtils::GetApexR(Vector velocity, Vector position, double dist)
		{
			//Return the positive answer of the equation quadratic equation
			Vector toReturn;

			/////////// Get the equation of a the line for slope and intercept
//			double x1 = position.x;
//			double y1 = position.y;

			double m;
			double yint;
			if(velocity.x == 0)
			{
				m = 1;
				yint = 0;
			}else
			{
				m = velocity.y/velocity.x;
				yint = position.y - (m * position.x);
			}

			double p = position.x;
			double q = position.y;

			//Set a,b,c values for quadratic equation

			double a = pow(m,2) + 1;
			double b = (2*yint*m) - (2*p) - (2*m*q);
			double c = pow(p,2) + pow(yint,2) - (2*yint*q) + pow(q,2) +- pow(dist, 2);

			toReturn.x = (-b - sqrt(pow(b,2) - 4*a*c))/2*a;
			toReturn.y = m * toReturn.x + b;

			return toReturn;
		}

	}



}
