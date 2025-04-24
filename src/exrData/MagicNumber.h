#pragma once
#include <cstdint>
#include <stdexcept>
#include <vector>
#include "utils.h"

namespace exr
{
	static int32_t magicNumber(const std::vector<ui8>& filebytes)
	{
		if (filebytes.size() < 4)
		{
			throw std::invalid_argument("(filebytes) contains less than 4 bytes. Unable to extract OpenEXR Magic Number.");
		}
		/*
			source: https://openexr.com/en/latest/OpenEXRFileLayout.html#magic-number
			date: 2025.03.11
			
			The magic number, of type int, is always 20000630 (decimal). 
			It allows file readers to distinguish OpenEXR files from other files, 
			since the first four bytes of an OpenEXR file are always 0x76, 0x2f, 0x31 and 0x01.
		*/
		std::vector<uint8_t> exr_magicBytes = utils::slice((std::vector<uint8_t>)filebytes, 0, 3);
		int32_t exr_magicNum = 0;
		// pack 4 uint8_t into uint32_t - method 1: tested on little-endian, untested on big-endian systems
		std::memcpy(&exr_magicNum, exr_magicBytes.data(), exr_magicBytes.size());	// safe & portable
		// pack 4 uint8_t into uint32_t - method 2: use on big endian systems if "memcpy" method gives incorrect result
		/*
		if (!isBigEndian)	// if little-endian => reverse order of bytes
		{
			std::reverse(exr_magicBytes.begin(), exr_magicBytes.end());
		}
		if (exr_magicBytes.size() != 4)
		{
			throw std::runtime_error("magicBytes must have 4 bytes to be castable to int.");
		}
		if (sizeof(exr_magicNum) < 4)
		{
			throw std::invalid_argument("exr_magicNum does not have enough bytes to store 4-byte number");
		}
		const uint8_t bitStep = 8;
		for(unsigned int i = 0; i < exr_magicBytes.size(); i++)
		{
			exr_magicNum = exr_magicNum << bitStep;	// 0x000000?? => 0x0000??00
			exr_magicNum |= exr_magicBytes[i];		// magicNum | 0x?? => 0x000000??
		}
		*/
		return exr_magicNum;
	}

}
