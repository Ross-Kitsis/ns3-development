/*
 * vector-utils.h
 *
 *  Created on: Nov 20, 2015
 *      Author: Ross Kitsis
 */

#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

#include "ns3/buffer.h"
#include "ns3/vector.h"

namespace ns3
{
	/*
	 * Writes a vector to the passed buffer
	 */
	void WriteTo(Buffer::Iterator &i, Vector v);


	/*
	 * Reads a vector from the passed buffer
	 */
	void ReadFrom(Buffer::Iterator &i, Vector v);
}



#endif /* SRC_MCAST_MODEL_VECTOR_UTILS_H_ */
