#pragma once
#include <stdexcept>
#include <type_traits>

namespace exrPixeldata
{
	// test start

	/*
	
	/// <summary>
	///		RGBA name reflects how getByIndex and setByIndex work (RGBA -> 0123)
	/// </summary>
	/// <typeparam name="channelCType"></typeparam>
	template <typename channelCType>
	class RGBA
	{
		public:
		static inline uint32_t channelsNumber() { return 4; }
	
		channelCType R = 0, G = 0, B = 0, A = 0;
	
		RGBA() {}
		RGBA(const channelCType R, const channelCType G, const channelCType B, const channelCType A) : R(R), G(G), B(B), A(A) {}
		channelCType getByIndex(const uint32_t channelIndex) const
		{
			switch(channelIndex)
			{
				case 0: return R; break;
				case 1: return G; break;
				case 2: return B; break;
				case 3: return A; break;
				default: throw invalidChannelIndex;
			}
		}
		void setByIndex(const uint32_t channelIndex, channelCType newValue)
		{
			switch(channelIndex)
			{
				case 0: R = newValue; break;
				case 1: G = newValue; break;
				case 2: B = newValue; break;
				case 3: A = newValue; break;
				default: throw invalidChannelIndex;
			}
		}
		std::string toString(const std::string& delim = ", ", const bool isReversed = false) const
		{
			if (!isReversed) return std::to_string(R) + delim + std::to_string(G) + delim + std::to_string(B) + delim + std::to_string(A);
			else			 return std::to_string(A) + delim + std::to_string(B) + delim + std::to_string(G) + delim + std::to_string(R);
		}
	
		private:
		static inline const std::invalid_argument invalidChannelIndex = std::invalid_argument("channelIndex input parameter is out of valid range [0; 4]");
	};
	
	*/
	
	// test end
	
	/// Scanline reading classes
	static inline const std::invalid_argument s_c_invalidPixelRowIndex = std::invalid_argument("(pixelRowIndex) value is out of valid range [0; numberOfScanlines-1] (note: numberOfScanlines equals to number of image rows)");
	static inline const std::invalid_argument s_c_invalidPixelColumnIndex = std::invalid_argument("(pixelColumnIndex) value is out of valid range [0; numberOfPixelsInScanline-1] (note: numberOfPixelsInScanline equals to number of image columns)");
	
	// template type constraint do disallow data types that will cause error
	template<typename T>
	concept Unsigned32OrFloat = (sizeof(T) <= 4) and !std::is_same_v<T, bool> and (std::is_unsigned_v<T> or std::is_floating_point_v<T>);	// requires type of 4-byte size and not bool and (unsigned integer or float)
	
	// read each channel of each pixel of the scan line
	/*
		source: https://openexr.com/en/latest/OpenEXRFileLayout.html#sample-file:~:text=End%20of%20scan%20line%20offset%20table
		date: 2025.04.03
	
		Scan line pixel data is stored in the following way (for RGBA .exr file)
		scanLine					0
		scanLineSize				...
		scanLine0.pixels.Rchannels	px00.R, px01.R, px02.R, px03.R
		scanLine0.pixels.Gchannels	px00.G, px01.G, px02.G, px03.G
		...
		scanLine					1
		scanLineSize				...
		scanLine1.pixels.Rchannels	px10.R, px11.R, px12.R, px13.R
		scanLine1.pixels.Gchannels	px10.G, px11.G, px12.G, px13.G
		
		* Note: inside scanline, pixel channels are organized in the same order as they're listed in chlist attribute value.
	*/
	// stores exrFile.pixelData.RegularScanline[i].channel[i].pixel[i]
	template <Unsigned32OrFloat channelCType32>
	class Channel_i_ofPixel
	{
		public:
		Channel_i_ofPixel(const std::vector<ui8>& filebytes, const uint32_t valueFirstByteIndex)
			: m_channelAndByteRange(utils::IndexedValue<channelCType32>(0,0,0))
		{
			//tryValidateChannelValueType<channelCType32>();
			uint32_t valueLastByteIndex = valueFirstByteIndex + sizeof(channelCType32)-1;
			channelCType32 value;
			//if ((channelCType4Bytes == unsigned int) or (channelCType4Bytes == uint32_t))
			if (std::is_same_v<channelCType32, unsigned int> or std::is_same_v<channelCType32, uint32_t>)
			{
				value = exrTypes::readUint32((filebytes.begin()+valueFirstByteIndex)._Ptr);
			}
			else if (std::is_same_v<channelCType32, float>)
			{
				value = exrTypes::readFloat32((filebytes.begin()+valueFirstByteIndex)._Ptr);
			}
			m_channelAndByteRange = utils::IndexedValue(valueFirstByteIndex, valueLastByteIndex, value);
		}
		Channel_i_ofPixel(const Channel_i_ofPixel<channelCType32>& other)
			: m_channelAndByteRange(other.m_channelAndByteRange)
		{
			//tryValidateChannelValueType<channelCType32>();
		}
		Channel_i_ofPixel<channelCType32>& operator=(const Channel_i_ofPixel<channelCType32>& other)
		{
			if (this != &other)
			{
				m_channelAndByteRange = other.m_channelAndByteRange;
			}
			return *this;
		}
		channelCType32 value() const { return m_channelAndByteRange.value(); }
		uint32_t firstByteIndex() const { return m_channelAndByteRange.firstByteIndex(); }
		uint32_t lastByteIndex() const { return m_channelAndByteRange.lastByteIndex(); }
		std::string toString(const uint8_t tabsNum = 0, const bool includeByteIndexes = false) const
		{
			if (!includeByteIndexes)
				return utils::tabs(tabsNum) + std::string(value());
			else 
				return utils::tabs(tabsNum) + "[0x" + utils::hex(firstByteIndex(),4) + "; 0x" + utils::hex(lastByteIndex(),4) + "] " + std::to_string(value());
		}
	
		private:
		utils::IndexedValue<channelCType32> m_channelAndByteRange;
	
	};
	
	// stores exrFile.pixelData.RegularScanline[i].channel[i]
	template <Unsigned32OrFloat channelCType32>
	class Channel_i_ofScanlinePixels
	{
		public:
		Channel_i_ofScanlinePixels() { /*tryValidateChannelValueType<channelCType32>();*/ }
		Channel_i_ofScanlinePixels(const std::vector<ui8>& filebytes, const uint32_t pixeldataFirstByteIndex, const uint32_t scanlinePixelsNum)
		{
			//tryValidateChannelValueType<channelCType32>();
			uint32_t currentByteIndex = pixeldataFirstByteIndex;
			for (uint32_t i = 0; i < scanlinePixelsNum; i++)
			{
				Channel_i_ofPixel<channelCType32> channelOfPixel_copy = Channel_i_ofPixel<channelCType32>(filebytes, currentByteIndex);
				m_channel_i_ofScanlinePixels.push_back(channelOfPixel_copy);
				currentByteIndex += 4;	// because Channel_i_ofPixel requires channelCType4Bytes, not channelCType_?_Bytes
			}
		}
		Channel_i_ofScanlinePixels(const Channel_i_ofScanlinePixels<channelCType32> &other)
			: m_channel_i_ofScanlinePixels(other.m_channel_i_ofScanlinePixels)
		{
			//tryValidateChannelValueType<channelCType32>();
		}
		void tryValidatePixelColumnIndex(const uint32_t pixelIndex) const
		{
			if (pixelIndex < 0 or (scanlinePixelsNum()-1) < pixelIndex)
			{
				throw s_c_invalidPixelColumnIndex;
			}
		}
		Channel_i_ofScanlinePixels<channelCType32>& operator=(const Channel_i_ofScanlinePixels<channelCType32>& other)
		{
			if (this != &other)
			{
				m_channel_i_ofScanlinePixels.clear();
				for (const Channel_i_ofPixel<channelCType32> channel_i_ofScanline : other.m_channel_i_ofScanlinePixels)
				{
					m_channel_i_ofScanlinePixels.push_back(channel_i_ofScanline);
				}
			}
			return *this;
		}
		Channel_i_ofPixel<channelCType32> indexedChannelByPixelIndex(const uint32_t pixelIndex) const
		{
			tryValidatePixelColumnIndex(pixelIndex);
			return m_channel_i_ofScanlinePixels[pixelIndex];
		}
		channelCType32 channelValueByPixelIndex(const uint32_t pixelIndex) const
		{
			tryValidatePixelColumnIndex(pixelIndex);
			return m_channel_i_ofScanlinePixels[pixelIndex].value();
		}
		uint32_t firstByteIndex() const { return m_channel_i_ofScanlinePixels[0].firstByteIndex(); }	// first byte index of first scanline pixel channel_i entry
		uint32_t lastByteIndex() const { return this->m_channel_i_ofScanlinePixels[scanlinePixelsNum()-1].lastByteIndex(); }	// last byte index of last scanline pixel channel_i entry
		uint32_t scanlinePixelsNum() const { return this->m_channel_i_ofScanlinePixels.size(); }
	
		private:
		std::vector<Channel_i_ofPixel<channelCType32>> m_channel_i_ofScanlinePixels;
	
	};
	
	// stores exrFile.pixelData.RegularScanLine[i]		(Channel_i_ofScanlinePixels per each pixel channel = scanline)
	template <Unsigned32OrFloat channelCType32>
	class RegularScanline
	{
		public:
		static uint32_t pixelChannelsNum() { return 4; }		// number of channel_i_ofScanlinePixels = number of scanline's pixels' pixel channel groups
		RegularScanline() { /*tryValidateChannelValueType<channelCType32>();*/ }
		/// ReadMe: scanline structure
		/*
			source: https://openexr.com/en/latest/OpenEXRFileLayout.html#sample-file:~:text=End%20of%20scan%20line%20offset%20table
			date: 2025.04.03
	
			Scan line pixel data is stored in the following way (for RGBA .exr file)
			------------------------------------------------------------
			scanLine					0
			scanLineSize				...
			scanLine0.pixels.Rchannels	px00.R, px01.R, px02.R, px03.R
			scanLine0.pixels.Gchannels	px00.G, px01.G, px02.G, px03.G
			...
			scanLine					1
			scanLineSize				...
			scanLine1.pixels.Rchannels	px10.R, px11.R, px12.R, px13.R
			scanLine1.pixels.Gchannels	px10.G, px11.G, px12.G, px13.G
			------------------------------------------------------------
			* Note: inside scanline, pixel channels are organized in the same order as they're listed in chlist attribute value.
			
		*/
		/// <summary>
		/// 
		/// </summary>
		/// <param name="filebytes"></param>
		/// <param name="scanlineFirstByteIndex"></param>
		/// <param name="scanlinePixelsNum"></param>
		/// <param name="pixelChannelsNum"></param>
		RegularScanline(const std::vector<ui8>& filebytes, const uint32_t scanlineFirstByteIndex, const uint32_t scanlinePixelsNum, const uint32_t pixelChannelsNum)
			:
			// calculate scanline generic data filebytes-indexes
			m_firstByteIndex(scanlineFirstByteIndex),
			m_yFirstByteIndex(scanlineFirstByteIndex), m_yLastByteIndex(scanlineFirstByteIndex + sizeof(m_y) - 1)
		{
			m_valueSizeFirstByteIndex = m_yLastByteIndex + 1;
			m_valueSizeLastByteIndex = m_valueSizeFirstByteIndex + sizeof(m_valueSize) - 1;
			// validate template type
			//tryValidateChannelValueType<channelCType32>();
			// save scanline generic data
			m_y = int32_t(exrTypes::readUint32((filebytes.begin()+m_yFirstByteIndex)._Ptr));					/// risk: converting this uint32->int32 when uint32 is greater than 0.5*2^32, may result in wrong number
			uint32_t valueSize = exrTypes::readUint32((filebytes.begin()+m_valueSizeFirstByteIndex)._Ptr);
			m_valueSize = int32_t(valueSize);	/// risk: converting this uint32->int32 when uint32 is greater than 0.5*2^32, may result in wrong number
			
			// read scanline pixels' channel groups
			uint32_t scanlineChannelsA_firstByteIndex = m_valueSizeLastByteIndex + 1;
			m_scanlineChannelsA = Channel_i_ofScanlinePixels<channelCType32>(filebytes, scanlineChannelsA_firstByteIndex, scanlinePixelsNum);
			m_scanlineChannelsB = Channel_i_ofScanlinePixels<channelCType32>(filebytes, m_scanlineChannelsA.lastByteIndex()+1, scanlinePixelsNum);
			m_scanlineChannelsG = Channel_i_ofScanlinePixels<channelCType32>(filebytes, m_scanlineChannelsB.lastByteIndex()+1, scanlinePixelsNum);
			m_scanlineChannelsR = Channel_i_ofScanlinePixels<channelCType32>(filebytes, m_scanlineChannelsG.lastByteIndex()+1, scanlinePixelsNum);
			m_lastByteIndex = m_scanlineChannelsR.lastByteIndex();
		}

		RegularScanline<channelCType32>& operator=(const RegularScanline<channelCType32>& other)
		{
			if (this != &other)
			{
				m_firstByteIndex = other.m_firstByteIndex;
				m_lastByteIndex = other.m_lastByteIndex;
	
				m_y = other.m_y;
				m_yFirstByteIndex = other.m_yFirstByteIndex;
				m_yLastByteIndex = other.m_yLastByteIndex;
	
				m_valueSize = other.m_valueSize;
				m_valueSizeFirstByteIndex = other.m_valueSizeFirstByteIndex;
				m_valueSizeLastByteIndex = other.m_valueSizeLastByteIndex;
	
				m_scanlineChannelsA = other.m_scanlineChannelsA;
				m_scanlineChannelsB = other.m_scanlineChannelsB;
				m_scanlineChannelsG = other.m_scanlineChannelsG;
				m_scanlineChannelsR = other.m_scanlineChannelsR;
			}
			return *this;
		}
		int32_t _y() const { return m_y; }
		int32_t _valueSizeInBytes() const { return m_valueSize; }
		channelCType32 a(const uint32_t pixelColumnIndex) const { m_scanlineChannelsA.tryValidatePixelColumnIndex(pixelColumnIndex); return m_scanlineChannelsA.channelValueByPixelIndex(pixelColumnIndex); }
		channelCType32 b(const uint32_t pixelColumnIndex) const { m_scanlineChannelsB.tryValidatePixelColumnIndex(pixelColumnIndex); return m_scanlineChannelsB.channelValueByPixelIndex(pixelColumnIndex); }
		channelCType32 g(const uint32_t pixelColumnIndex) const { m_scanlineChannelsG.tryValidatePixelColumnIndex(pixelColumnIndex); return m_scanlineChannelsG.channelValueByPixelIndex(pixelColumnIndex); }
		channelCType32 r(const uint32_t pixelColumnIndex) const { m_scanlineChannelsR.tryValidatePixelColumnIndex(pixelColumnIndex); return m_scanlineChannelsR.channelValueByPixelIndex(pixelColumnIndex); }
		channelCType32 ABGRchannelByIndex(const uint32_t scanlinePixelIndex, const uint32_t pixelChannelIndex)
		{
			tryValidatePixelChannelIndex(pixelChannelIndex);
			switch(pixelChannelIndex)
			{
				case 0: return a(scanlinePixelIndex); break;
				case 1: return b(scanlinePixelIndex); break;
				case 2: return g(scanlinePixelIndex); break;
				case 3: return r(scanlinePixelIndex); break;
				default: throw s_c_invalidChannelIndex;
			}
		}
		Channel_i_ofScanlinePixels<channelCType32> scanlineChannelsByChannelIndex(const uint32_t pixelChannelIndex) const
		{
			tryValidatePixelChannelIndex(pixelChannelIndex);
			switch(pixelChannelIndex)
			{
				case 0: return Channel_i_ofScanlinePixels<channelCType32>(m_scanlineChannelsA); break;
				case 1: return Channel_i_ofScanlinePixels<channelCType32>(m_scanlineChannelsB); break;
				case 2: return Channel_i_ofScanlinePixels<channelCType32>(m_scanlineChannelsG); break;
				case 3: return Channel_i_ofScanlinePixels<channelCType32>(m_scanlineChannelsR); break;
				default: throw s_c_invalidChannelIndex;
			}
			
		}
		uint32_t firstByteIndex() const { return m_firstByteIndex; }
		uint32_t lastByteIndex() const { return m_lastByteIndex; }
		uint32_t yFirstByteIndex() const { return m_yFirstByteIndex; }
		uint32_t yLastByteIndex() const { return m_yLastByteIndex; }
		uint32_t valueSizeFirstByteIndex() const { return m_valueSizeFirstByteIndex; }
		uint32_t valueSizeLastByteIndex() const { return m_valueSizeLastByteIndex; }
		uint32_t sizeInBytes() const { return m_lastByteIndex - m_firstByteIndex + 1; }
		uint32_t pixelsNum() const { return m_scanlineChannelsA.scanlinePixelsNum(); }	// all channels are of same scanline of pixels => their size is the same
		
		private:
		uint32_t m_firstByteIndex = 0, m_lastByteIndex = 0;
		int32_t m_y = 0;			uint32_t m_yFirstByteIndex = 0, m_yLastByteIndex = 0;
		int32_t m_valueSize = 0;	uint32_t m_valueSizeFirstByteIndex = 0, m_valueSizeLastByteIndex = 0;
		Channel_i_ofScanlinePixels<channelCType32> m_scanlineChannelsA;	// already provides its .firstByteIndex() and .lastByteIndex()
		Channel_i_ofScanlinePixels<channelCType32> m_scanlineChannelsB;
		Channel_i_ofScanlinePixels<channelCType32> m_scanlineChannelsG;
		Channel_i_ofScanlinePixels<channelCType32> m_scanlineChannelsR;
		
		static inline const std::invalid_argument s_c_invalidChannelIndex = std::invalid_argument("(pixelChannelIndex) value is out of valid range [0; Scanline_v2::pixelChannelsNum() - 1]");
		void tryValidatePixelChannelIndex(const uint32_t pixelChannelIndex) const
		{
			if (pixelChannelIndex < 0 or (pixelChannelsNum() - 1) < pixelChannelIndex)
			{
				throw s_c_invalidChannelIndex;
			}
		}
	
	};
	
	// stores exrFile.pixelData		(RegularScanline per each image row (image line) = pixelData)
	template <Unsigned32OrFloat channelCType32>
	class PixelData
	{
		public:
		PixelData(const std::vector<ui8>& filebytes, const uint32_t pixeldataFirstByteIndex, const uint32_t imageRowsNum, const uint32_t imageColumnsNum, const uint32_t pixelChannelsNum, const exr2::consta::s_lineOrder::ctype lineOrderValue)
		{
			//tryValidateChannelValueType<channelCType32>();
			// from .exr, read each scanline, providing (currentScanlineFirstByteIndex) which starts from (pixeldataFirstByteIndex) value and steps (+ scanline.byteSize()),
			uint32_t currentScanlineFirstByteIndex = pixeldataFirstByteIndex;
			for (uint32_t scanlineIndex = 0; scanlineIndex < imageRowsNum; scanlineIndex++)
			{
				// read next scanline
				RegularScanline<channelCType32> scanline_copy = RegularScanline<channelCType32>(filebytes, currentScanlineFirstByteIndex, imageColumnsNum, pixelChannelsNum);
				m_scanlines.push_back(scanline_copy);
				currentScanlineFirstByteIndex += m_scanlines[scanlineIndex].sizeInBytes();
				// validate scanline.y(), scanline row index
				//		if (lineOrder == INCREASING_Y)		=> u_y must be the same as (scanlineIndex) value,
				//		else if (lineOrder == DECREASING_Y) => u_y must be the same as (imageRowsNumber-1 - scanlineIndex),
				//		else if (lineOrder == RANDOM_Y)		=> u_y must be in range [0; imageRowsNumber-1].
				//		else => lineOrder value is unknown.
				int32_t scanlineY = m_scanlines[scanlineIndex]._y();
				if (lineOrderValue == exr2::consta::s_lineOrder::value::INCREASING_Y)
				{
					if (scanlineY != scanlineIndex)
						throw std::runtime_error("(y) row value of scanline from .exr is different from (rowIndex) or (scanlineIndex). Check .exr lineOrder value to know actual order in your .exr file.");
				}
				else if (lineOrderValue == exr2::consta::s_lineOrder::value::DECREASING_Y)
				{
					if (scanlineY != imageRowsNum-1-scanlineIndex)
						throw std::runtime_error("(y) row value of scanline from .exr is different from (imageRows-1-rowIndex) or (imageRows-1-scanlineIndex). Check .exr lineOrder value to know actual order in your .exr file.");
				}
				else if (lineOrderValue == exr2::consta::s_lineOrder::value::RANDOM_Y)
				{
					if ((scanlineY < 0) or ((imageRowsNum-1) < scanlineY))
						throw std::runtime_error("(y) row value of scanline from .exr is out of valid range [0; imageRowsNum-1]. Check .exr lineOrder value to know actual order in your .exr file.");
				}
				else throw std::runtime_error("(lineOrder) attribute of your .exr file has invalid value. Check OpenEXR documentation on valid lineOrder values.");
	
			}
		}
		PixelData<channelCType32>& operator=(const PixelData<channelCType32>& other)
		{
			if (this != &other)
			{
				m_scanlines.clear();
				for (const RegularScanline<channelCType32> scanline : other.m_scanlines)
				{
					m_scanlines.push_back(scanline);
				}
			}
			return *this;
		}
		uint32_t firstByteIndex() const { return m_scanlines[0].firstByteIndex(); }						// firstByteIndex of first scanline
		uint32_t lastByteIndex() const { return m_scanlines[m_scanlines.size()-1].lastByteIndex(); }	// lastByteIndex of last scanline

		/// <summary>
		/// 
		/// </summary>
		/// <how-to-use>
		///		int main()
		///		{
		///			auto rgba = pixelStructRGBA(2, 5);	std::cout << "pixelRGBA[2,5]= " << rgba;
		///			// or 
		///			auto [r,g,b,a] = pixelStructRGBA(2, 5);		printf("pixelRGBA[2,5]= (%u, %u, %u, %u)", r, g, b, a);
		///		}
		/// </how-to-use>
		/// <param name="pixelRowIndex"></param>
		/// <param name="pixelColumnIndex"></param>
		/// <returns></returns>
		auto pixelStructRGBA(const uint32_t pixelRowIndex, const uint32_t pixelColumnIndex) const
		{
			tryValidatePixelRowIndex(pixelRowIndex);
			tryValidatePixelColumnIndex(pixelColumnIndex);
			channelCType32 a = m_scanlines[pixelRowIndex].a(pixelColumnIndex);
			channelCType32 b = m_scanlines[pixelRowIndex].b(pixelColumnIndex);
			channelCType32 g = m_scanlines[pixelRowIndex].g(pixelColumnIndex);
			channelCType32 r = m_scanlines[pixelRowIndex].r(pixelColumnIndex);
			return {r, g, b, a};
		}
		std::string pixelRGBAString(const uint32_t pixelRowIndex, const uint32_t pixelColumnIndex, const bool hex = false, const uint8_t fltPrecis = 6) const
		{
			tryValidatePixelRowIndex(pixelRowIndex);
			tryValidatePixelColumnIndex(pixelColumnIndex);
			//std::string scanlinePixelsValues = "";
			//for (uint32_t pixelRowIndex = 0; pixelRowIndex < u_scanlines.size(); pixelRowIndex++)
			//{
			//	const Scanline_v2<channelCType> scanline = u_scanlines[pixelRowIndex];
			//	for (uint32_t pixelColumnIndex = 0; pixelColumnIndex < scanline.pixelsNum(); pixelColumnIndex++)
			//	{
			//		channelCType a = scanline.a(pixelColumnIndex);
			//		channelCType b = scanline.b(pixelColumnIndex);
			//		channelCType g = scanline.g(pixelColumnIndex);
			//		channelCType r = scanline.r(pixelColumnIndex);
			//		scanlinePixelsValues += 
			//			std::to_string(pixelRowIndex) + ", " + std::to_string(pixelColumnIndex) + ": " + 
			//			std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + std::to_string(a) + "\n";
			//	}
			//}
			std::string rgbaStr = "<empty>";
			const RegularScanline<channelCType32> scanline = m_scanlines[pixelRowIndex];
			channelCType32 a = scanline.a(pixelColumnIndex);
			channelCType32 b = scanline.b(pixelColumnIndex);
			channelCType32 g = scanline.g(pixelColumnIndex);
			channelCType32 r = scanline.r(pixelColumnIndex);
			// compatible with numeric C-types
			if (!hex)
			{
				if (std::is_same_v<channelCType32, float>)
				{
					rgbaStr = utils::str(r, fltPrecis) + ", " + utils::str(g, fltPrecis) + ", " + utils::str(b, fltPrecis) + ", " + utils::str(a, fltPrecis);
				}
				else
				{
					rgbaStr = std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + std::to_string(a);
				}
			}
			// compatible only with integer C-types (+ requires 0 to 255 values)
			else
			{
				if (!(utils::doesFitByte(r) && utils::doesFitByte(g) && utils::doesFitByte(b) && utils::doesFitByte(a)))
				{
					throw std::invalid_argument("(r), (g), (b) and (a) values must fit into byte (be in [0; 255] range)");
				}
				rgbaStr = "0x" + utils::hex(r, 2) + utils::hex(g, 2) + utils::hex(b, 2) + utils::hex(a, 2);
			}
			return rgbaStr;
		}
		std::string toStringAsRGBAPixels(const bool hex = false, const uint8_t fltPrecis = 6) const
		{
			std::string str = "";
			for (uint32_t row = 0; row < m_scanlines.size(); row++)
			{
				str += "row [" + std::to_string(row) + "]: \n";
				RegularScanline<channelCType32> scanline = m_scanlines[row];
				for (uint32_t col = 0; col < scanline.pixelsNum(); col++)
				{
					str += "\t pixel[" + std::to_string(row) + ", " + std::to_string(col) + "].RGBA = " + pixelRGBAString(row, col, hex, fltPrecis) + "\n";
				}
			}
			return str;
		}
		std::string toStringAsExrPixeldata(std::vector<std::string>& channelNamesOrderedAsInChlist, const uint8_t tabsNum = 0) const
		{
			// note: order of channel names in Chlist is the same as the order of channels in PixelData
			std::string pixeldataStr;
			for (uint32_t i = 0; i < m_scanlines.size(); i++)	// scanline index = row
			{
				RegularScanline<channelCType32> scanline = m_scanlines[i];
				pixeldataStr += utils::tabs(tabsNum) + "[0x" + utils::hex(scanline.yFirstByteIndex(),4) + "; 0x" + utils::hex(scanline.yLastByteIndex(),4) + "] scanline.y = " + std::to_string(scanline._y()) + " \n";
				pixeldataStr += utils::tabs(tabsNum) + "[0x" + utils::hex(scanline.valueSizeFirstByteIndex(),4) + "; 0x" + utils::hex(scanline.valueSizeLastByteIndex(),4) + "] dataSizeInBytes = " + std::to_string(scanline._valueSizeInBytes()) + "\n";
				pixeldataStr += utils::tabs(tabsNum) + "entries: \n";
				for (uint32_t j = 0; j < scanline.pixelChannelsNum(); j++)		// pixel channel
				{
					Channel_i_ofScanlinePixels<channelCType32> scanlinePixelsChannels = scanline.scanlineChannelsByChannelIndex(j);
					for (uint32_t k = 0; k < scanline.pixelsNum(); k++)			// scanline pixel index = column
					{
						Channel_i_ofPixel<channelCType32> channel = scanlinePixelsChannels.indexedChannelByPixelIndex(k);
						// output: [0x001; 0x006] channelValue	\t = px[row, col] channel channelName
						pixeldataStr += 
							utils::tabs(tabsNum+1) + "[0x" + utils::hex(channel.firstByteIndex(),4) + "; 0x" + utils::hex(channel.lastByteIndex(),4) + "] " + utils::str(channel.value(), 9)
							+ "\t = px[" + std::to_string(i) + ", " + std::to_string(k) + "] channel " + channelNamesOrderedAsInChlist[j] + "\n";
					}
					if (pixeldataStr[pixeldataStr.size()-1] == '\n') pixeldataStr.erase(pixeldataStr.end());	// remove last '\n'
				}
			}
			return pixeldataStr;
		}
	
		private:
		std::vector<RegularScanline<channelCType32>> m_scanlines;

		void tryValidatePixelRowIndex(const uint32_t pixelRowIndex) const
		{
			if (pixelRowIndex < 0 or (m_scanlines.size() - 1) < pixelRowIndex)
			{
				throw s_c_invalidPixelRowIndex;
			}
		}
		void tryValidatePixelColumnIndex(const uint32_t pixelColumnIndex) const
		{
			if (pixelColumnIndex < 0 or (m_scanlines[0].pixelsNum() - 1) < pixelColumnIndex)
			{
				throw s_c_invalidPixelColumnIndex;
			}
		}
	
	};

}
