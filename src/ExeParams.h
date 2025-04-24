#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include "pcinfo.h"

namespace exe
{
	#if ARCH_X64 or ARCH_ARM64
		using argc_t = uint64_t;
	#else
		using argc_t = uint32_t;
	#endif
	using argv_t = char**;
	using argvElem_t = char*;

	// argc = number of paramters addressed by (argv) array of char*.
	// argv = array of char* (char pointers), 
	//		where each argv[i] stores pointer to string representing 
	//		another program parameter (from running .exe from command-line or elsewhere)
	class ExeParams
	{
		public:
		ExeParams(const int argc, const char** argv)		
		{
			if (argc < 0)
			{
				throw std::invalid_argument("(argc) is less than 0. Program can not have 0 parameters (first parameter is always .exe path and name)");
			}
			m_argc = argc_t(argc);
			m_argvStrings = std::vector<std::string>(m_argc);
			for (argc_t i = 0; i < m_argc; i++)
			{
				m_argvStrings[i] = std::string(argv[i]);
			}
			// check that any parameter is not empty string
			for (argc_t i = 0; i < m_argc; i++)
			{
				if (param(i).empty())
				{
					throw std::runtime_error("parameter [" + std::to_string(i) + "] is empty string");
				}
			}
		}
		argc_t paramsNum() const { return m_argc; }
		std::string param(const argc_t index) const
		{
			if (index < m_argc-1 and m_argc-1 < index)
			{
				throw std::invalid_argument("ExeParams::getArgv(index). (index) value is out of valid range [0; u_argc-1]");
			}
			return m_argvStrings[index];
		}
		std::string pathAndName() const { return m_argvStrings[0]; }
		std::string toString() const
		{
			std::string result = "argc = " + std::to_string(m_argc) + "\n";
			result += "argv[i]: \n";
			for (argc_t i = 0; i < m_argc; i++)
			{
				result += std::to_string(i) + ": " + m_argvStrings[i] + "\n";
			}
			return result;
		}

		private:
		argc_t m_argc = 0;
		std::vector<std::string> m_argvStrings;
	};
}

