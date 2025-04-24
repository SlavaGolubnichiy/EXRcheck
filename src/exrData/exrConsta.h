#pragma once
#include <cstdint>
#include <string>

namespace exr
{
	namespace consta
	{
		/* document tag [STD-ATTRIBUTE-01] */
		namespace StdAttribName
		{
			inline static const std::string channels = "channels";
			inline static const std::string compression = "compression";
			inline static const std::string dataWindow = "dataWindow";
			inline static const std::string displayWindow = "displayWindow";
			inline static const std::string lineOrder = "lineOrder";
			inline static const std::string pixelAspectRatio = "pixelAspectRatio";
			inline static const std::string screenWindowCenter = "screenWindowCenter";
			inline static const std::string screenWindowWidth = "screenWindowWidth";
		};

		/* document tag [STD-ATTRIBUTE-01] */
		namespace Type
		{
			inline static const std::string chlist = "chlist";
			inline static const std::string compression = "compression";
			inline static const std::string box2i = "box2i";
			inline static const std::string lineOrder = "lineOrder";
			inline static const std::string float32 = "float";
			inline static const std::string v2f = "v2f";
		};

		/* document tag [EXR-ATTRIB-TYPES-01] */
		namespace TypeValueSizeBytes
		{
			inline static const uint32_t compression= 1;
			inline static const uint32_t box2i		= 4 * sizeof(uint32_t);
			inline static const uint32_t lineOrder	= 1;
			inline static const uint32_t float32	= sizeof(float);
			inline static const uint32_t v2f		= 2 * sizeof(float);
		};

	}

}

namespace exr2
{
	namespace consta
	{
		const uint32_t magicNumber = 0x01312F76;
		const uint8_t versionNumber = 0x02;

		/* document tag [VERSIONFIELD-02] */
		namespace ValidVersionField
		{
			inline constexpr uint32_t singleScan			= 0;			// bits 9, 10, 11, 12: 0, 0, 0, 0
			inline constexpr uint32_t singleTile			= 0x00000200;	// bits 9, 10, 11, 12: 1, ~, 0, 0
			inline constexpr uint32_t multiScanOrTile		= 0x00001000;	// bits 9, 10, 11, 12: 0, ~, 0, 1
			inline constexpr uint32_t singleDeepScanOrTile	= 0x00000800;	// bits 9, 10, 11, 12: 0, ~, 1, 0
			inline constexpr uint32_t multiDeepScanOrTile	= 0x00001800;	// bits 9, 10, 11, 12: 0, ~, 1, 1
		}

		static std::string compressionName(const uint8_t compressionValue)
		{
			std::string name;
			switch(compressionValue)
			{
				case 0: name = "NO_COMPRESSION"; break;
				case 1: name = "RLE_COMPRESSION"; break;
				case 2: name = "ZIPS_COMPRESSION"; break;
				case 3: name = "ZIP_COMPRESSION"; break;
				case 4: name = "PIZ_COMPRESSION"; break;
				case 5: name = "PXR24_COMPRESSION"; break;
				case 6: name = "B44_COMPRESSION"; break;
				case 7: name = "B44A_COMPRESSION"; break;
				case 8: name = "DWAA_COMPRESSION"; break;
				case 9: name = "DWAB_COMPRESSION"; break;
				default:
				{
					throw std::invalid_argument("OpenEXR compression can not have the specified value. Check the documentation.");
					break;
				}
			}
			return name;
		}

		namespace lineOrder
		{
			typedef uint8_t ctype;

			static const enum value
			{
				INCREASING_Y = 0x00,
				DECREASING_Y = 0x01,
				RANDOM_Y	 = 0x02
			};
		}

		static std::string lineOrderName(const lineOrder::ctype lineOrderValue)
		{
			switch(lineOrderValue)
			{
				case lineOrder::value::INCREASING_Y: return "INCREASING_Y"; break;
				case lineOrder::value::DECREASING_Y: return "DECREASING_Y"; break;
				case lineOrder::value::RANDOM_Y:	 return "RANDOM_Y"; break;
				default:
				{
					throw std::invalid_argument("OpenEXR lineOrder can not have the specified value. Check the documentation.");
					break;
				}
			}
		}

	}

}
