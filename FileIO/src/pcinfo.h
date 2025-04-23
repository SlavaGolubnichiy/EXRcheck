#pragma once

namespace pcinfo
{
	// works with C++ 11.0 and earlier
	int8_t isBigEndianPC()
	{
		union BigEndianIfc0First
		{
			uint32_t i;
			uint8_t c[4];
		}
		test = {0x01020304};
		return test.c[0] == 0x01;	// big-endian if most significant byte first
	}
	// requires C++ 20
	#if __cplusplus > 201703L
	#include <bit>
	int8_t isBigEndianPCcpp20()
	{
		if constexpr (std::endian::native == std::endian::big)
		{
			return 1;	// true
		}
		else if (std::endian::native == std::endian::little)
		{
			return 0;	// false
		}
		else
		{
			return -1;
		}
	}

	#endif

	class System
	{
		public:
		System()
			: u_isBigEndian(isBigEndianPC())
		{

		}
		std::string isBigEndianToString() const
		{
			switch(u_isBigEndian)
			{
				case 1: return "big-endian (most significant byte first)";
				case 0: return "little-endian (least significant byte first)";
				case -1: return "mixed-endian / undefined";
				default: return "TEST ERROR: UNKNOWN";
			}
		}

		private:
		int u_isBigEndian = 0xFF;
	};

}
