#pragma once
#include <stdexcept>
#include <vector>

#include "exrData/exrConsta.h"
#include "types.h"
#include "utils.h"

/* document tag [VERSIONFIELD-01] */
class VersionField
{
	public:
	VersionField(const std::vector<u8>& filebytes)
	{
		std::vector<uint8_t> exr_versionFieldBytes = utils::slice((std::vector<uint8_t>)filebytes, vfFirstByteIndex, vfLastByteIndex);	// versionField: filebytes [04~07]
		std::memcpy(&versionfield, exr_versionFieldBytes.data(), exr_versionFieldBytes.size());
		exrVersionNum = versionfield & 0x000000FF;
		bit09_singlePartTiled = bool((versionfield & 0x00000200) >> 8);	// docs say mask = 0x0200 => bit 9 is bit countable or bit index ? - test!
		bit10_hasLongNames = bool((versionfield & 0x00000400) >> 8);	// docs say mask = 0x0400
		bit11_hasDeepData = bool((versionfield & 0x00000800) >> 8);		// docs say mask = 0x0800
		bit12_isMultipart = bool((versionfield & 0x00001000) >> 8);		// docs say mask = 0x1000
		if (exrVersionNum == 1)
		{
			// check for OpenEXR 1.x - implement
			throw std::runtime_error("This program does not yet support .exr versions before OpenEXR 2.0");
		}
		else if (exrVersionNum == exr2::consta::versionNumber)
		{
			// check it fetches remaining 19 bits	[VERSIOFIELD-BIT-08] [VERSIOFIELD-BIT-13-31]
			const uint32_t exr2_0_unusedFlags = versionfield & 0xFFFFE100;
			if (exr2_0_unusedFlags != 0)
			{
				throw std::runtime_error("OpenEXR 2.0 must have each reserved versionField bit (masked by 0xFFFFE100) equal 0");
			}
		}
		else
		{
			throw VersionField::ex_exrVersionUnknown;
		}
	}

	bool isValidExr2_0() const
	{
		uint32_t versionfieldAnyVersion = versionfield & 0xFFFFFF00;		// all its except OpenEXR version byte
		return (versionfieldAnyVersion == exr2::consta::ValidVersionField::singleScan or
			versionfieldAnyVersion == exr2::consta::ValidVersionField::singleTile or
			versionfieldAnyVersion == exr2::consta::ValidVersionField::multiScanOrTile or
			versionfieldAnyVersion == exr2::consta::ValidVersionField::singleDeepScanOrTile or 
			versionfieldAnyVersion == exr2::consta::ValidVersionField::multiDeepScanOrTile);
	}

	uint8_t exrVersion() const
	{
		return exrVersionNum;
	}

	bool bit09_SinglePartTiled() const
	{
		return bit09_singlePartTiled;
	}

	bool bit10_HasLongNames() const
	{
		return bit10_hasLongNames;
	}

	bool bit11_HasDeepData() const
	{
		return bit11_hasDeepData;
	}

	bool bit12_IsMultipart() const
	{
		return bit12_isMultipart;
	}

	void printData() const
	{
		printf("[%2.2X~%2.2X] (little-endian) %8.8X => bits [00~31] \n", vfFirstByteIndex, vfLastByteIndex, versionfield);
		printf("\t bits [07~00] exr version number _________________ = %u \n", exrVersionNum);
		if (isValidExr2_0())
		{
			printf("\t bit  [09] is single-tiled _______________________ = %u \n", bit09_singlePartTiled);
			printf("\t bit  [10] contains long names ___________________ = %u \n", bit10_hasLongNames);
			printf("\t bit  [11] contains non-image (deep format) parts  = %u \n", bit11_hasDeepData);
			printf("\t bit  [12] contains multiple parts _______________ = %u \n", bit12_isMultipart);
		}
		else
		{
			printf("\t WARNING: EXR file version field is unknown or invalid value. Version field analysis is skipped, but this may lead to incorrect further analysis. \n");
		}
	}

	private:
	static const uint32_t vfFirstByteIndex = 4;
	static const uint32_t vfLastByteIndex = 7;
	static const std::runtime_error ex_exrVersionUnknown;

	uint32_t versionfield = 0;
	uint8_t exrVersionNum = 0;
	bool bit09_singlePartTiled = false;
	bool bit10_hasLongNames = false;
	bool bit11_hasDeepData = false;
	bool bit12_isMultipart = false;
};
const std::runtime_error VersionField::ex_exrVersionUnknown = std::runtime_error("Input filebytes have unknown OpenEXR version or filebytes is not a .exr file.");
