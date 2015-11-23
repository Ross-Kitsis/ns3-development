#include "vector-utils.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VectorUtils");

void WriteTo(Buffer::Iterator &i, Vector v)
{
	NS_LOG_FUNCTION(&i << &v);

	double x = v.x;
	double y = v.y;
	double z = v.z;

	unsigned char const * p = reinterpret_cast<unsigned char const *>(&x);
	i.Write(p,sizeof(double));

	unsigned char const * q  = reinterpret_cast<unsigned char const *>(&y);
	i.Write(q,sizeof(double));

	unsigned char const * r = reinterpret_cast<unsigned char const *>(&z);
	i.Write(r,sizeof(double));

/*
 * float f = 0.5f;

unsigned char const * p = reinterpret_cast<unsigned char const *>(&f);

for (std::size_t i = 0; i != sizeof(float); ++i)
{
    std::printf("The byte #%zu is 0x%02X\n", i, p[i]);
}
 */
}

/*
void WriteTo (Buffer::Iterator &i, Ipv6Address ad)
{
  NS_LOG_FUNCTION (&i << &ad);
  uint8_t buf[16];
  ad.GetBytes (buf);
  i.Write (buf, 16);
}*/

void ReadFrom(Buffer::Iterator &i, Vector v)
{
	NS_LOG_FUNCTION(&i << &v);
	uint8_t vel[sizeof(double)];

	i.Read(vel, sizeof(double));
	double x;
	memcpy(&x, vel, sizeof(double));
	v.x = x;

	i.Read(vel, sizeof(double));
	double y;
	memcpy(&y, vel, sizeof(double));
	v.y = y;

	i.Read(vel, sizeof(double));
	double z;
	memcpy(&z, vel, sizeof(double));
	v.z = z;
}

/*
 * void ReadFrom (Buffer::Iterator &i, Ipv6Address &ad)
{
  NS_LOG_FUNCTION (&i << &ad);
  uint8_t ipv6[16];
  i.Read (ipv6, 16);
  ad.Set (ipv6);
}
 */


}
