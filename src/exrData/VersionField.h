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
	VersionField(const std::vector<ui8>& filebytes)
	{
		std::vector<uint8_t> exr_versionFieldBytes = utils::slice((std::vector<uint8_t>)filebytes, s_c_vfFirstByteIndex, s_c_vfLastByteIndex);	// versionField: filebytes [04~07]
		std::memcpy(&m_versionfield, exr_versionFieldBytes.data(), exr_versionFieldBytes.size());
		m_exrVersionNum = m_versionfield & 0x000000FF;
		m_bit09_singlePartTiled = bool((m_versionfield & 0x00000200) >> 8);	// docs say mask = 0x0200 => bit 9 is bit countable or bit index ? - test!
		m_bit10_hasLongNames = bool((m_versionfield & 0x00000400) >> 8);	// docs say mask = 0x0400
		m_bit11_hasDeepData = bool((m_versionfield & 0x00000800) >> 8);		// docs say mask = 0x0800
		m_bit12_isMultipart = bool((m_versionfield & 0x00001000) >> 8);		// docs say mask = 0x1000
		if (m_exrVersionNum == 1)
		{
			// check for OpenEXR 1.x - implement
			throw std::runtime_error("This program does not yet support .exr versions before OpenEXR 2.0");
		}
		else if (m_exrVersionNum == exr2::consta::c_versionNumber)
		{
			// check it fetches remaining 19 bits	[VERSIOFIELD-BIT-08] [VERSIOFIELD-BIT-13-31]
			const uint32_t exr2_0_unusedFlags = m_versionfield & 0xFFFFE100;
			if (exr2_0_unusedFlags != 0)
			{
				throw std::runtime_error("OpenEXR 2.0 must have each reserved versionField bit (masked by 0xFFFFE100) equal 0");
			}
		}
		else
		{
			throw VersionField::s_c_exrVersionUnknown;
		}
	}

	bool isValidExr2_0() const
	{
		uint32_t versionfieldAnyVersion = m_versionfield & 0xFFFFFF00;		// all its except OpenEXR version byte
		return (versionfieldAnyVersion == exr2::consta::ValidVersionField::c_singleScan or
			versionfieldAnyVersion == exr2::consta::ValidVersionField::c_singleTile or
			versionfieldAnyVersion == exr2::consta::ValidVersionField::c_multiScanOrTile or
			versionfieldAnyVersion == exr2::consta::ValidVersionField::c_singleDeepScanOrTile or 
			versionfieldAnyVersion == exr2::consta::ValidVersionField::c_multiDeepScanOrTile);
	}

	uint8_t exrVersion() const
	{
		return m_exrVersionNum;
	}

	bool bit09_SinglePartTiled() const
	{
		return m_bit09_singlePartTiled;
	}

	bool bit10_HasLongNames() const
	{
		return m_bit10_hasLongNames;
	}

	bool bit11_HasDeepData() const
	{
		return m_bit11_hasDeepData;
	}

	bool bit12_IsMultipart() const
	{
		return m_bit12_isMultipart;
	}

	void printData() const
	{
		printf("[%2.2X~%2.2X] (little-endian) %8.8X => bits [00~31] \n", s_c_vfFirstByteIndex, s_c_vfLastByteIndex, m_versionfield);
		printf("\t bits [07~00] exr version number _________________ = %u \n", m_exrVersionNum);
		if (isValidExr2_0())
		{
			printf("\t bit  [09] is single-tiled _______________________ = %u \n", m_bit09_singlePartTiled);
			printf("\t bit  [10] contains long names ___________________ = %u \n", m_bit10_hasLongNames);
			printf("\t bit  [11] contains non-image (deep format) parts  = %u \n", m_bit11_hasDeepData);
			printf("\t bit  [12] contains multiple parts _______________ = %u \n", m_bit12_isMultipart);
		}
		else
		{
			printf("\t WARNING: EXR file version field is unknown or invalid value. Version field analysis is skipped, but this may lead to incorrect further analysis. \n");
		}
	}

	private:
	static const uint32_t s_c_vfFirstByteIndex = 4;
	static const uint32_t s_c_vfLastByteIndex = 7;
	static inline const std::runtime_error s_c_exrVersionUnknown = std::runtime_error("Input filebytes have unknown OpenEXR version or filebytes is not a .exr file.");

	uint32_t m_versionfield = 0;
	uint8_t m_exrVersionNum = 0;
	bool m_bit09_singlePartTiled = false;
	bool m_bit10_hasLongNames = false;
	bool m_bit11_hasDeepData = false;
	bool m_bit12_isMultipart = false;
};
