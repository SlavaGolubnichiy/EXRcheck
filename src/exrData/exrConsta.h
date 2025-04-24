#pragma once
#include <cstdint>
#include <string>
#include "utils.h"

namespace exr
{
	namespace consta
	{
		/* document tag [STD-ATTRIBUTE-01] */
		namespace StdAttribName
		{
			inline static const std::string s_channels = "channels";
			inline static const std::string s_compression = "compression";
			inline static const std::string s_dataWindow = "dataWindow";
			inline static const std::string s_displayWindow = "displayWindow";
			inline static const std::string s_lineOrder = "lineOrder";
			inline static const std::string s_pixelAspectRatio = "pixelAspectRatio";
			inline static const std::string s_screenWindowCenter = "screenWindowCenter";
			inline static const std::string s_screenWindowWidth = "screenWindowWidth";
		};

		/* document tag [STD-ATTRIBUTE-01] */
		namespace Type
		{
			inline static const std::string s_chlist = "chlist";
			inline static const std::string s_compression = "compression";
			inline static const std::string s_box2i = "box2i";
			inline static const std::string s_lineOrder = "lineOrder";
			inline static const std::string s_float32 = "float";
			inline static const std::string s_v2f = "v2f";
		};

		/* document tag [EXR-ATTRIB-TYPES-01] */
		namespace TypeValueSizeBytes
		{
			inline static const uint32_t s_compression= 1;
			inline static const uint32_t s_box2i		= 4 * sizeof(uint32_t);
			inline static const uint32_t s_lineOrder	= 1;
			inline static const uint32_t s_float32	= sizeof(float);
			inline static const uint32_t s_v2f		= 2 * sizeof(float);
		};

	}

}

namespace exr2
{
	namespace consta
	{
		const uint32_t c_magicNumber = 0x01312F76; const uint32_t c_magicNumber_firstByteIndex = 0, c_magicNumber_lastByteIndex = 3;
		const uint8_t c_versionNumber = 0x02;

		/* document tag [VERSIONFIELD-02] */
		namespace ValidVersionField
		{
			inline constexpr uint32_t c_singleScan			= 0;			// bits 9, 10, 11, 12: 0, 0, 0, 0
			inline constexpr uint32_t c_singleTile			= 0x00000200;	// bits 9, 10, 11, 12: 1, ~, 0, 0
			inline constexpr uint32_t c_multiScanOrTile		= 0x00001000;	// bits 9, 10, 11, 12: 0, ~, 0, 1
			inline constexpr uint32_t c_singleDeepScanOrTile= 0x00000800;	// bits 9, 10, 11, 12: 0, ~, 1, 0
			inline constexpr uint32_t c_multiDeepScanOrTile	= 0x00001800;	// bits 9, 10, 11, 12: 0, ~, 1, 1
		}

		namespace channel
		{
			const enum datatype
			{
				min		= 0x00,
				UINT	= 0x00,
				HALF	= 0x01,
				FLOAT	= 0x02,
				max		= 0x02
			};

			std::string channelDataTypeName(const uint32_t channelDataTypeByteValue)
			{
				if (channelDataTypeByteValue < datatype::min or datatype::max < channelDataTypeByteValue)
				{
					throw 
						std::logic_error
						(
							"Channel data type byte value = " + utils::hex(channelDataTypeByteValue, 2*sizeof(channelDataTypeByteValue)) + 
							"is out of valid range [" + utils::hex(datatype::min) + "; " + utils::hex(datatype::max) + "]"
						);
				}
				return (channelDataTypeByteValue == exr2::consta::channel::datatype::UINT) ? "UINT" : 
					(channelDataTypeByteValue == exr2::consta::channel::datatype::HALF) ? "HALF" : 
					(channelDataTypeByteValue == exr2::consta::channel::datatype::FLOAT) ? "FLOAT" : "EXR2_INVALID_VALUE_OF_CHANNEL_DATA_TYPE_BYTE";
			}
		}

		namespace s_compression
		{
			static const enum value
			{
				NO		= 0x00,
				RLE		= 0x01,
				ZIPS	= 0x02,
				ZIP		= 0x03,
				PIZ		= 0x04,
				PXR24	= 0x05,
				B44		= 0x06,
				B44A	= 0x07,
				DWAA	= 0x08,
				DWAB	= 0x09
			};
		}

		static std::string compressionName(const uint8_t compressionValue)
		{
			std::string name;
			switch(compressionValue)
			{
				case s_compression::value::NO:		name = "NO_COMPRESSION"; break;
				case s_compression::value::RLE:		name = "RLE_COMPRESSION"; break;
				case s_compression::value::ZIPS:	name = "ZIPS_COMPRESSION"; break;
				case s_compression::value::ZIP:		name = "ZIP_COMPRESSION"; break;
				case s_compression::value::PIZ:		name = "PIZ_COMPRESSION"; break;
				case s_compression::value::PXR24:	name = "PXR24_COMPRESSION"; break;
				case s_compression::value::B44:		name = "B44_COMPRESSION"; break;
				case s_compression::value::B44A:	name = "B44A_COMPRESSION"; break;
				case s_compression::value::DWAA:	name = "DWAA_COMPRESSION"; break;
				case s_compression::value::DWAB:	name = "DWAB_COMPRESSION"; break;
				default:
				{
					throw std::invalid_argument("OpenEXR compression can not have the specified value. Check the documentation.");
					break;
				}
			}
			return name;
		}

		namespace s_lineOrder
		{
			typedef uint8_t ctype;

			static const enum value
			{
				INCREASING_Y = 0x00,
				DECREASING_Y = 0x01,
				RANDOM_Y	 = 0x02
			};
		}

		static std::string lineOrderName(const s_lineOrder::ctype lineOrderValue)
		{
			switch(lineOrderValue)
			{
				case s_lineOrder::value::INCREASING_Y: return "INCREASING_Y"; break;
				case s_lineOrder::value::DECREASING_Y: return "DECREASING_Y"; break;
				case s_lineOrder::value::RANDOM_Y:	 return "RANDOM_Y"; break;
				default:
				{
					throw std::invalid_argument("OpenEXR lineOrder can not have the specified value. Check the documentation.");
					break;
				}
			}
		}

	}

}
