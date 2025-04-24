#pragma once

// OS
#if defined(__APPLE__)
	#define OS_MACOS 1
#elif defined(__linux__)
	#define OS_LINUX 1
#elif defined(_WIN32)
	#define OS_WINDOWS 1
#else
	#define OS_UNKNOWN 1
#endif
// Architecture
#if defined(__x86_64__) || defined(_M_X64) || OS_WINDOWS && defined(_WIN64)
	#define ARCH_X64 1
#elif defined(__i386__) || defined(_M_IX86) || OS_WINDOWS && !defined(_WIN64)
	#define ARCH_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64)
	#define ARCH_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
	#define ARCH_ARM32 1
#elif defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || defined(__PPC__)	// old MacOS
	#define ARCH_POWERPC 1
#else
	#define ARCH_UNKNOWN 1
#endif

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

	std::string Configuration()
	{
		#if _DEBUG
			return "DEBUG";
		#else
			return "RELEASE";
		#endif
	}

	class System
	{
		public:
		System()
			: m_isBigEndian(isBigEndianPC())
		{
		}

		std::string isBigEndianToString() const
		{
			switch(m_isBigEndian)
			{
				case 1: return "big-endian (most significant byte first)";
				case 0: return "little-endian (least significant byte first)";
				case -1: return "mixed-endian / undefined";
				default: return "TEST ERROR: UNKNOWN";
			}
		}

		private:
		int m_isBigEndian = 0xFF;
	};

}
