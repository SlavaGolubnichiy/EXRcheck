#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <stdexcept>

#include "types.h"
#include "exrData/exrConsta.h"
#include "utils.h"

namespace exrTypes
{
	bool isValidExrAttributeNameLength(const std::string& attribName, const bool versionFieldBit10)
	{
		uint32_t attribNameSizeBytes = attribName.length() + 1;	// + '\0' byte
		if (!versionFieldBit10)
		{
			if (31 < attribNameSizeBytes)
			{
				throw std::runtime_error("exr file has versionfield.bit10 = 0 (indicating attribute names from 1 to 31 bytes), but attribute name is larger than max. size");
				return false;
			}
		}
		else
		{
			if (255 < attribNameSizeBytes)
			{
				throw std::runtime_error("exr file has versionfield.bit10 = 1 (indicating attribute names from 1 to 255 bytes), but attribute name is larger than max. size");
				return false;
			}
		}
		return true;
	}

	static std::uint32_t readUint32(const void* firstByte)
	{
		uint32_t uint = 0;
		std::memcpy(&uint, firstByte, 4);	// uint32_t is 4 bytes
		return uint;
	}

	static std::uint64_t readUint64(const void* firstByte)
	{
		uint64_t value = 0;
		std::memcpy(&value, firstByte, 8);	// uint64_t is 8 bytes
		return value;
	}

	static float readFloat32(const void* firstByte)
	{
		const uint32_t requiredNumberOfBytes = 4;
		float value = 0;
		/// how to make this check at compile-time ????
		if (sizeof(value) != requiredNumberOfBytes)
		{
			throw std::runtime_error("float value is not 4 bytes on this system! Unable to read 4-byte float value.");
		}
		std::memcpy(&value, firstByte, requiredNumberOfBytes);
		return value;
	}

	static std::string readCString(const std::vector<ui8>::const_iterator strFirst, const std::vector<ui8>::const_iterator cend)
	{
		std::vector<ui8>::const_iterator strEnd = std::find(strFirst, cend, '\0');
		if (strEnd == cend)
		{
			throw std::runtime_error("\'\0\' (c-string null-terminator) not found");
		}
		return std::string(strFirst, strEnd);
	}



	/// todo ?
	/*

	/// <summary>
	///  Class for incapsulation of firstByteIndex and lastByteIndex for each value from a file.
	/// </summary>
	/// <typeparam name="InterpretedType"></typeparam>
	template <typename InterpretedType>
	class byteseq
	{
		std::uint32_t firstByteIndex = 0;
		std::uint32_t lastByteIndex = 0;
		InterpretedType value;

		explicit byteseq(InterpretedType initValue)
		{
		}
	};

	*/

	/* document tag [EXR-ATTRIB-TYPES-01] */
	class Channel
	{
		public:
		Channel(const std::vector<ui8>& filebytes, const uint32_t channelFirstByteIndex, const bool versionFieldBit10)
			: m_channel_firstByteIndex(channelFirstByteIndex)
		{
			// find "channels\0chlist\0" byte-substring in filebytes and analyse the following bytes
			analyseChannel(filebytes, channelFirstByteIndex);
			m_channel_lastByteIndex = lastByteIndex();
		}

		void analyseChannel(const std::vector<ui8>& bytes, const uint32_t channelFirstByteIndex)
		{
			// chlist channel name
			m_name_firstByteIndex = channelFirstByteIndex;
			m_name = exrTypes::readCString(bytes.cbegin() + m_name_firstByteIndex, bytes.cend());
			m_name_lastByteIndex = m_name_firstByteIndex + m_name.length();	// no -1 because counting +1 '\0'
			// channel data type
			m_channelType_firstByteIndex = m_name_lastByteIndex + 1;
			m_channelType_lastByteIndex = m_channelType_firstByteIndex + sizeof(m_channelType)-1; // int32_t is 4 bytes => offset= 3
			m_channelType = exrTypes::readUint32((bytes.cbegin() + m_channelType_firstByteIndex)._Ptr);
			// channel pLinear
			m_pLinear_firstByteIndex = m_channelType_lastByteIndex + 1;
			m_pLinear_lastByteIndex = m_pLinear_firstByteIndex;
			m_pLinear = bytes[m_pLinear_firstByteIndex];
			// channel 3 reserved bytes
			m_reserved_firstByteIndex = m_pLinear_lastByteIndex + 1;
			m_reserved_lastByteIndex = m_reserved_firstByteIndex + m_reserved.size()-1; // 3 bytes => offset= 2
			for (uint32_t i = 0; i < m_reserved.size(); i++)
			{
				m_reserved[i] = bytes[m_reserved_firstByteIndex+i];
			}
			// channel xSampling int
			m_samplingX_firstByteIndex = m_reserved_lastByteIndex + 1;
			m_samplingX_lastByteIndex = m_samplingX_firstByteIndex + sizeof(m_samplingX)-1; // int32_t is 4 bytes => offset= 3
			m_samplingX = exrTypes::readUint32((bytes.cbegin()+m_samplingX_firstByteIndex)._Ptr);
			// channel ySampling int
			m_samplingY_firstByteIndex = m_samplingX_lastByteIndex + 1;
			m_samplingY_lastByteIndex = m_samplingY_firstByteIndex + sizeof(m_samplingY)-1; // int32_t is 4 bytes => offset= 3
			m_samplingY = exrTypes::readUint32((bytes.cbegin()+m_samplingY_firstByteIndex)._Ptr);
			m_channel_lastByteIndex = m_samplingY_lastByteIndex;
			m_isAnalysed = true;
		}

		std::string name() const { return m_name; }
		
		uint32_t type() const { return m_channelType; }

		uint32_t lastByteIndex() const
		{
			if (!m_isAnalysed) throw m_channelNotAnalysed;
			return m_channel_lastByteIndex;
		}

		uint32_t valueSizeBytes() const
		{
			if (!m_isAnalysed) throw m_channelNotAnalysed;
			return m_channel_lastByteIndex - m_channel_firstByteIndex + 1;	// channel value size in bytes
		}

		uint32_t channelSizeBytes() const
		{
			if (!m_isAnalysed) throw m_channelNotAnalysed;
			return m_channel_lastByteIndex - m_channel_firstByteIndex + 1;
		}

		void printDetailed(const uint8_t tabsNum = 0) const
		{
			if (!m_isAnalysed) throw m_channelNotAnalysed;
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			printf("%s channel [0x%4.4X ~ 0x%4.4X] name= \t\'%s\'+\'\\0\' \n", tabs.c_str(), m_name_firstByteIndex, m_name_lastByteIndex, m_name.c_str());
			std::string channelTypeStr = exr2::consta::channel::channelDataTypeName(m_channelType);
			printf("%s\t [0x%4.4X ~ 0x%4.4X] type = \t0x%8.8X (%s) \n", tabs.c_str(), m_channelType_firstByteIndex, m_channelType_lastByteIndex, m_channelType, channelTypeStr.c_str());
			printf("%s\t [0x%4.4X _ ______] pLinear = \t0x%2.2X \n", tabs.c_str(), m_pLinear_firstByteIndex, m_pLinear);
			printf("%s\t [0x%4.4X ~ 0x%4.4X] reserved = \t0x%2.2X 0x%2.2X 0x%2.2X \n", tabs.c_str(), m_reserved_firstByteIndex, m_reserved_lastByteIndex, m_reserved[0], m_reserved[1], m_reserved[2]);
			printf("%s\t [0x%4.4X ~ 0x%4.4X] xSamplimg = \t0x%8.8X = %i \n", tabs.c_str(), m_samplingX_firstByteIndex, m_samplingX_lastByteIndex, m_samplingX, m_samplingX);
			printf("%s\t [0x%4.4X ~ 0x%4.4X] ySamplimg = \t0x%8.8X = %i \n", tabs.c_str(), m_samplingY_firstByteIndex, m_samplingY_lastByteIndex, m_samplingY, m_samplingY);
		}

		std::string toString(const uint8_t tabsNum = 0) const
		{
			if (!m_isAnalysed) throw m_channelNotAnalysed;
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			const std::string channelTypeStr = exr2::consta::channel::channelDataTypeName(m_channelType);
			return 
				tabs + " channel [0x" + utils::hex(m_name_firstByteIndex,4) + " ~ 0x" + utils::hex(m_name_lastByteIndex,4) + "] name \t= \'" + m_name + "\'+\'\\0\'\n"
				+ tabs + "\t [0x" + utils::hex(m_channelType_firstByteIndex,4) + " ~ 0x" + utils::hex(m_channelType_lastByteIndex,4) + "] type \t= 0x" + utils::hex(m_channelType,2) + " = " + channelTypeStr + "\n"
				+ tabs + "\t [0x" + utils::hex(m_pLinear_firstByteIndex,4) + " ~ ______] pLinear \t= 0x" + utils::hex(m_pLinear,2) + "\n"
				+ tabs + "\t [0x" + utils::hex(m_reserved_firstByteIndex,4) + " ~ 0x" + utils::hex(m_reserved_lastByteIndex,4) + "] reserved \t= 0x" + utils::hex(m_reserved[0], 2) + " 0x" + utils::hex(m_reserved[1], 2) + " 0x" + utils::hex(m_reserved[2], 2) + "\n"
				+ tabs + "\t [0x" + utils::hex(m_samplingX_firstByteIndex,4) + " ~ 0x" + utils::hex(m_samplingX_lastByteIndex,4) + "] xSampling \t= 0x" + utils::hex(m_samplingX,8) + " = " + std::to_string(m_samplingX) + "\n"
				+ tabs + "\t [0x" + utils::hex(m_samplingY_firstByteIndex,4) + " ~ 0x" + utils::hex(m_samplingY_lastByteIndex,4) + "] ySampling \t= 0x" + utils::hex(m_samplingY,8) + " = " + std::to_string(m_samplingY);
		}

		private:
		const std::runtime_error m_channelNotAnalysed = std::runtime_error("Channel is not yet analysed. Call analyseChannel() beforehand.");
		uint32_t m_channel_firstByteIndex = 0, m_channel_lastByteIndex = 0;
		uint32_t m_name_firstByteIndex = 0, m_name_lastByteIndex = 0;
		uint32_t m_channelType_firstByteIndex = 0, m_channelType_lastByteIndex = 0;
		uint32_t m_pLinear_firstByteIndex = 0, m_pLinear_lastByteIndex = 0;
		uint32_t m_reserved_firstByteIndex = 0, m_reserved_lastByteIndex = 0;
		uint32_t m_samplingX_firstByteIndex = 0, m_samplingX_lastByteIndex = 0;
		uint32_t m_samplingY_firstByteIndex = 0, m_samplingY_lastByteIndex = 0;

		bool m_isAnalysed = false;
		std::string m_name = "";
		uint32_t m_channelType = 0xFFFFFFFF;
		uint8_t m_pLinear = 0xFF;
		std::vector<uint8_t> m_reserved = {0xFF, 0xFF, 0xFF};		// OpenEXR: 3 reserved bytes
		int32_t m_samplingX = 0;
		int32_t m_samplingY = 0;
	};

	// ----
	
	/* document tag [EXR-ATTRIB-TYPES-01] */

	/// <summary>
	///		Provides interface (common fields & methods for all its subclasses), thus specifying the way of communicating
	///		to all and each subclass of this interface class.
	///		Creating instance of interface class is prohibited by declaring virtual functions + protected constructor.
	/// </summary>
	class exrTypeBase
	{
		/// <summary>
		///		* Warning		[CHLIST-VALUE-WARNING-1]
		///			Constructor of Chlist does not check the type of stored OpenEXR data to be of same size as its OpenEXR equivalent,
		///			because the size of chlist is not known until its value is read. exrValueInterface_v2 sets its valueLastByteIndex member
		///			within the construction call, therefore Chlist class is MADE FRIEND to exrValueInterface_v2 for Chlist to be able to set 
		///			valueLastByteIndex later in the constructor of itself after reading through the Chlist value from filebytes.
		/// </summary>
		friend class Chlist;		// allows Chlist_v2 access private members (needed for u_lastByteIndex)

		public:
		/// <summary>
		///		In derived class, implement it returns std::string of OpenEXR type name of OpenEXR data derived class stores.
		/// </summary>
		/// <returns> std::string of name of OpenEXR type of data, the derived class stores </returns>
		virtual std::string type() const = 0;
		/// <summary>
		///		In derived class, implement it returns uint32_t size (in bytes) of OpenEXR-type data derived class stores.
		/// </summary>
		/// <returns> uint32_t number of bytes of data (payload) (of OpenEXR type), the derived class stores </returns>
		virtual uint32_t sizeInBytes() const = 0;
		/// <summary>
		///		Returns index of FIRST byte of value in input .exr file bytes vector (filebytes).
		/// </summary>
		/// <returns> uint32_t index of FIRST byte of value in input .exr filebytes </returns>
		virtual uint32_t firstByteIndex() const final { return m_firstByteIndex; }
		/// <summary>
		///		Returns index of LAST byte of value in input .exr file bytes vector (filebytes).
		/// </summary>
		/// <returns> uint32_t index of LAST byte of value in input .exr filebytes </returns>
		virtual uint32_t lastByteIndex() const final { return m_lastByteIndex; }
		/// <summary>
		///		In derived class, implement it returns string containing values stored within class.
		/// </summary>
		/// <returns> std::string text with values stored by class </returns>
		virtual std::string toString(const uint8_t tabsNum = 0) const = 0;

		protected:
		/// <summary>
		///		Protected constructor to give classes ability to inherit from it, that initializes parameters:
		///			index of first byte of value = valueFirstByteIndex;
		///			index of last byte of value = valueFirstByteIndex + sizeInBytes - 1;
		/// </summary>
		/// <param name="valueFirstByteIndex"> - index of first byte of value within full .exr filebytes vector of bytes </param>
		/// <param name="sizeInBytes"> - number of bytes value of its type takes </param>
		exrTypeBase(const uint32_t valueFirstByteIndex, const uint32_t sizeInBytes)
			: 
			m_firstByteIndex(valueFirstByteIndex), 
			m_lastByteIndex(valueFirstByteIndex + sizeInBytes - 1)
		{
		}
		
		/// <summary>
		///		Call to validate that the size retrieved from .exr filebytes equals to the size required by the class, derived from the interface.
		/// </summary>
		/// <param name="requiredSizeInBytes"> - uint32_t number of bytes required according to implementation of the derived class </param>
		inline void tryValidateSizeIs(const uint32_t requiredSizeInBytes) const
		{
			// check if type of stored value and sizeInBytes() are implemented correctly
			if (sizeInBytes() != requiredSizeInBytes)
			{
				throw std::runtime_error("Stored value size in bytes is incorrect to OpenEXR required (check \'OpenEXR standard attribute value types\').");
			}
		}

		private:
		uint32_t m_firstByteIndex = 0, m_lastByteIndex = 0;
	};

	/// <summary>
	///		Implements OpenEXR type interface to operate with data of "compression" OpenEXR type + 
	///		methods name() and value() specifically for comfortable work with data of this type.
	///		
	///		* Warning: Chlist is a friend of exrValueInterface_v2 ( search for [CHLIST-VALUE-WARNING-1] )
	/// </summary>
	class Chlist : public exrTypeBase
	{
		public:
		/// <summary>
		///		Constructor creates instance of base class and
		///			reads the value (and its components) from filebytes &
		///			initializes filebytes indexes of first and last bytes of each value component (using another container class).
		///		* Warning:		[CHLIST-VALUE-WARNING-1]
		///			Constructor of Chlist does not check the type of stored OpenEXR data to be of same size as its OpenEXR equivalent,
		///			because the size of chlist is not known until its value is read. exrValueInterface_v2 sets its valueLastByteIndex member
		///			within the construction call, therefore Chlist class is MADE FRIEND to exrValueInterface_v2 for Chlist to be able to set 
		///			valueLastByteIndex later in the constructor of itself after reading through the Chlist value from filebytes.
		///		** Since there is no constant pre-known size of value of Chlist type, calling tryValidateSizeIs has no sense, 
		///			since there is no suitable predefined requiredSize constant.
		/// </summary>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="chlistFirstByteIndex"> - index of first byte of chlist value within "filebytes" vector of input .exr file bytes </param>
		Chlist(const std::vector<ui8> filebytes, const uint32_t chlistFirstByteIndex, const bool versionFieldBit10)
			: exrTypeBase(chlistFirstByteIndex, sizeInBytes())
		{
			// tryValidateSizeIs();		// chlist size does not have predefined const value
			// extract channels from attribute byte sequence
			uint32_t currentByteIndex = chlistFirstByteIndex;
			while (filebytes[currentByteIndex] != 0x00)
			{
				Channel ch = Channel(filebytes, currentByteIndex, versionFieldBit10);
				m_channels.push_back(ch);
				currentByteIndex += ch.channelSizeBytes();	/// ch.getSizeInBytes() better ?
			}
			m_lastByteIndex = currentByteIndex;			// counts final 0x00 byte	& setting u_lastByteIndex of base class finalizes base class initialization impossible for chlist to do solely using base class constructor
			// analyse final byte that must be 0x00
			if (filebytes[m_lastByteIndex] != 0x00)
			{
				throw std::runtime_error("Chlist value last byte must be 0x00, but its not. Further analysis is impossible. Possible reasons: file is bad / .exr file layout is bad / bug during analysis.");
			}
		}

		void tryValidateChannelIndex(const uint32_t channelIndex) const
		{
			uint32_t channelIndexMin = 0;
			uint32_t channelIndexMax = m_channels.size()-1;
			if (channelIndex < channelIndexMin or channelIndexMax < channelIndex)
			{
				throw std::invalid_argument("(channelIndex) = " + std::to_string(channelIndex) + " is out of valid range [" + std::to_string(channelIndexMin) + "; " + std::to_string(channelIndexMax) + "]");
			}
		}

		/// <summary> Implements interface: returns string of name of OpenEXR-defined data type stored within this class. </summary>
		/// <returns> std::string name of OpenEXR-defined type of data stored within this class </returns>
		std::string type() const override { return exr::consta::Type::s_chlist; }		// rename to attrib_type()
		/// <summary>
		///		Implements interface: returns number of bytes that OpenEXR data stored within this class takes, 
		///		which must be equal to the size (in bytes) of value of corresponding OpenEXR-defined data type 
		///		this class is implemented to store.
		/// </summary>
		/// <returns> uint32_t number of bytes took by OpenEXR data stored within this class </returns>
		uint32_t sizeInBytes() const override
		{
			uint32_t sizeBytes = 0;
			for (const Channel ch : m_channels)
			{
				sizeBytes += ch.channelSizeBytes();
			}
			return sizeBytes;
		}
		
		std::string channelName(const uint32_t channelIndex) const
		{
			tryValidateChannelIndex(channelIndex);
			return m_channels[channelIndex].name();
		}

		std::vector<std::string> channelsNames() const
		{
			std::vector<std::string> chNames;
			for (uint32_t i = 0; i < m_channels.size(); i++)
			{
				chNames.push_back(m_channels[i].name());
			}
			return chNames;
		}

		std::string channelDataTypeName(const uint32_t channelIndex) const
		{
			tryValidateChannelIndex(channelIndex);
			return exr2::consta::channel::channelDataTypeName(m_channels[channelIndex].type());
		}

		/// <summary> Implements interface: returns string containing values stored within class. </summary>
		/// <returns> std::string text with values stored by class </returns>
		std::string toString(const uint8_t tabsNum = 0) const override
		{
			// print chlist with one tab less
			uint8_t newTabsNum = tabsNum ? tabsNum-1 : tabsNum;
			std::string tabs = "";
			for (uint8_t i = 0; i < newTabsNum; i++)
			{
				tabs += "\t";
			}
			std::string result = "";
			for (Channel ch : m_channels)
			{
				result += ch.toString(newTabsNum) + "\n";
			}
			return result + tabs + " [0x" + utils::hex(m_lastByteIndex,4) + "] attribute final byte = 0x" + utils::hex(0x00);
		}
		/// <summary>
		///		Get number of channels stored in the Chlist channel list.
		/// </summary>
		/// <returns> number of channels, this object currently is storing </returns>
		uint32_t channelsNum() const { return m_channels.size(); }

		private:
		std::vector<Channel> m_channels;

	};

	/// <summary>
	///		Implements OpenEXR type interface to operate with data of "compression" OpenEXR type + 
	///		methods name() and value() specifically for comfortable work with data of this type.
	/// </summary>
	class Compression : public exrTypeBase
	{
		public:
		/// <summary>
		///		Constructor creates instance of base class and
		///			checks the type of stored OpenEXR data to be of same size as its OpenEXR equivalent,
		///			reads the value (or components of value) from filebytes.
		/// </summary>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="compressionFirstByteIndex"> - index of first byte of compression value within "filebytes" vector of input .exr file bytes </param>
		Compression(const std::vector<ui8> filebytes, const uint32_t compressionFirstByteIndex)
			: exrTypeBase(compressionFirstByteIndex, sizeInBytes())
		{
			// check if type of stored value and sizeInBytes() are implemented correctly
			/// is it possible to do this check during compile-time ???
			tryValidateSizeIs(exr::consta::TypeValueSizeBytes::s_compression);
			// read compression byte from filebytes
			m_compression = filebytes[firstByteIndex()];
		}
		/// <summary>
		///		Get OpenEXR-defined name of compression, given the compression value obtained from .exr file (filebytes).
		/// </summary>
		/// <returns> std::string text of compression name regarding compression value in .exr file (filebytes) </returns>
		std::string name() const { return exr2::consta::compressionName(m_compression); }
		/// <summary>
		///		Get compression value obtained from .exr file (filebytes).
		/// </summary>
		/// <returns> uint8_t (byte) value of compression specified in .exr file (filebytes) and used to compress the input .exr file </returns>
		uint8_t value() const { return m_compression; }

		/// <summary> Implements interface: returns string of name of OpenEXR-defined data type stored within this class. </summary>
		/// <returns> std::string name of OpenEXR-defined type of data stored within this class </returns>
		std::string type() const override { return exr::consta::Type::s_compression; }
		/// <summary>
		///		Implements interface: returns number of bytes that OpenEXR data stored within this class takes, 
		///		which must be equal to the size (in bytes) of value of corresponding OpenEXR-defined data type 
		///		this class is implemented to store.
		/// </summary>
		/// <returns> uint32_t number of bytes took by OpenEXR data stored within this class </returns>
		uint32_t sizeInBytes() const override { return sizeof(m_compression); }
		/// <summary> Implements interface: returns string containing values stored within class. </summary>
		/// <returns> std::string text with values stored by class </returns>
		std::string toString(const uint8_t tabsNum = 0) const override
		{
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			return tabs + std::to_string(m_compression) + " = " + name();
		}

		private:
		uint8_t m_compression = 0;	// size in bytes must equal to exr::consta::exrDataType::compression.sizeInBytes
	};

	/// <summary>
	///		Box2i = box 2D described with i (intergers).
	///		Implements OpenEXR type interface to operate with data of "box2i" OpenEXR type + 
	///		methods xMin(), xMax(), yMin(), yMax() specifically for comfortable work with data of this type.
	/// </summary>
	class Box2i : public exrTypeBase
	{
		public:
		/// <summary>
		///		Constructor creates instance of base class and
		///			checks the type of stored OpenEXR data to be of same size as its OpenEXR equivalent,
		///			initializes filebytes indexes of first and last bytes of each value component & 
		///			reads that value (or those components of value) from filebytes.
		/// </summary>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="box2iFirstByteIndex"> - index of first byte of box2i value within "filebytes" vector of input .exr file bytes </param>
		Box2i(const std::vector<ui8>& filebytes, const uint32_t box2iFirstByteIndex)
			: exrTypeBase(box2iFirstByteIndex, sizeInBytes())
		{
			tryValidateSizeIs(exr::consta::TypeValueSizeBytes::s_box2i);
			// read box2i from filebytes
			m_xMin_firstByteIndex = box2iFirstByteIndex;
			m_xMin_lastByteIndex = m_xMin_firstByteIndex + sizeof(m_xMin) - 1;
			m_yMin_firstByteIndex = m_xMin_lastByteIndex + 1;
			m_yMin_lastByteIndex = m_yMin_firstByteIndex + sizeof(m_yMin) - 1;
			m_xMax_firstByteIndex = m_yMin_lastByteIndex + 1;
			m_xMax_lastByteIndex = m_xMax_firstByteIndex + sizeof(m_xMax) - 1;
			m_yMax_firstByteIndex = m_xMax_lastByteIndex + 1;
			m_yMax_lastByteIndex = m_yMax_firstByteIndex + sizeof(m_yMax) - 1;
			m_xMin = exrTypes::readUint32((filebytes.begin() + m_xMin_firstByteIndex)._Ptr);
			m_yMin = exrTypes::readUint32((filebytes.begin() + m_yMin_firstByteIndex)._Ptr);
			m_xMax = exrTypes::readUint32((filebytes.begin() + m_xMax_firstByteIndex)._Ptr);
			m_yMax = exrTypes::readUint32((filebytes.begin() + m_yMax_firstByteIndex)._Ptr);
		}
		/// <summary>
		///		Get xMin component of value of box2i type.
		/// </summary>
		/// <returns> int32_t value of xMin component of value of box2i type </returns>
		int32_t xMin() const { return m_xMin; }
		/// <summary>
		///		Get xMax component of value of box2i type.
		/// </summary>
		/// <returns> int32_t value of xMax component of value of box2i type </returns>
		int32_t xMax() const { return m_xMax; }
		/// <summary>
		///		Get yMin component of value of box2i type.
		/// </summary>
		/// <returns> int32_t value of yMin component of value of box2i type </returns>
		int32_t yMin() const { return m_yMin; }
		/// <summary>
		///		Get yMax component of value of box2i type.
		/// </summary>
		/// <returns> int32_t value of yMax component of value of box2i type </returns>
		int32_t yMax() const { return m_yMax; }

		/// <summary> Implements interface: returns string of name of OpenEXR-defined data type stored within this class. </summary>
		/// <returns> std::string name of OpenEXR-defined type of data stored within this class </returns>
		std::string type() const override { return exr::consta::Type::s_box2i; }
		/// <summary>
		///		Implements interface: returns number of bytes that OpenEXR data stored within this class takes, 
		///		which must be equal to the size (in bytes) of value of corresponding OpenEXR-defined data type 
		///		this class is implemented to store.
		/// </summary>
		/// <returns> uint32_t number of bytes talen by OpenEXR data stored within this class </returns>
		uint32_t sizeInBytes() const override { return sizeof(m_xMin) + sizeof(m_xMax) + sizeof(m_yMin) + sizeof(m_yMax); }
		/// <summary> Implements interface: returns string containing values stored within class. </summary>
		/// <returns> std::string text with values stored by class </returns>
		std::string toString(const uint8_t tabsNum = 0) const override
		{
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			return 
				tabs + "[0x" + utils::hex(m_xMin_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_xMin_lastByteIndex, 4) + "] xMin = " + std::to_string(m_xMin) + "\n" + 
				tabs + "[0x" + utils::hex(m_yMin_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_yMin_lastByteIndex, 4) + "] yMin = " + std::to_string(m_yMin) + "\n" + 
				tabs + "[0x" + utils::hex(m_xMax_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_xMax_lastByteIndex, 4) + "] xMax = " + std::to_string(m_xMax) + "\n" + 
				tabs + "[0x" + utils::hex(m_yMax_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_yMax_lastByteIndex, 4) + "] yMax = " + std::to_string(m_yMax);
		}

		private:
		uint32_t m_xMin_firstByteIndex, m_xMin_lastByteIndex;
		uint32_t m_yMin_firstByteIndex, m_yMin_lastByteIndex;
		uint32_t m_xMax_firstByteIndex, m_xMax_lastByteIndex;
		uint32_t m_yMax_firstByteIndex, m_yMax_lastByteIndex;
		int32_t m_xMin = 0;
		int32_t m_yMin = 0;
		int32_t m_xMax = 0;
		int32_t m_yMax = 0;		// size in bytes must equal to exr::consta::exrDataType::box2i.sizeInBytes
	};

	/// <summary>
	///		Implements OpenEXR type interface to operate with data of "lineOrder" OpenEXR type + 
	///		methods name() and value() specifically for comfortable work with data of this type.
	/// </summary>
	class LineOrder : public exrTypeBase
	{
		public:
		/// <summary>
		///		Constructor creates instance of base class and
		///			checks the type of stored OpenEXR data to be of same size as its OpenEXR equivalent,
		///			reads that value (or those components of value) from filebytes.
		/// </summary>
		/// <param name="filebytes"> vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="box2iFirstByteIndex"> index of first byte of box2i value within "filebytes" vector of input .exr file bytes </param>
		LineOrder(const std::vector<ui8>& filebytes, const uint32_t lineOrderFirstByteIndex)
			:
			exrTypeBase(lineOrderFirstByteIndex, sizeInBytes()),
			m_lineOrder(filebytes[lineOrderFirstByteIndex])
		{
			tryValidateSizeIs(exr::consta::TypeValueSizeBytes::s_lineOrder);
		}
		/// <summary>
		///		Get the std::string name of the valid lineOrder value regarding its number value.
		/// </summary>
		/// <returns> std::string name of lineOrder value </returns>
		std::string name() const { return exr2::consta::lineOrderName(m_lineOrder); }
		/// <summary>
		///		Get lineOrder numerical value.
		/// </summary>
		/// <returns> uint8_t numerical value stored by lineOrder </returns>
		uint8_t value() const { return m_lineOrder; }

		/// <summary> Implements interface: returns string of name of OpenEXR-defined data type stored within this class. </summary>
		/// <returns> std::string name of OpenEXR-defined type of data stored within this class </returns>
		std::string type() const override { return exr::consta::Type::s_lineOrder; }
		/// <summary>
		///		Implements interface: returns number of bytes that OpenEXR data stored within this class takes, 
		///		which must be equal to the size (in bytes) of value of corresponding OpenEXR-defined data type 
		///		this class is implemented to store.
		/// </summary>
		/// <returns> uint32_t number of bytes taken by OpenEXR data stored within this class </returns>
		uint32_t sizeInBytes() const override { return sizeof(m_lineOrder); }
		/// <summary> Implements interface: returns string containing values stored within class. </summary>
		/// <returns> std::string text with values stored by class </returns>
		std::string toString(const uint8_t tabsNum = 0) const override
		{
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			return tabs + std::to_string(m_lineOrder) + " = " + name();
		}

		private:
		uint32_t m_firstByteIndex = 0, m_lastByteIndex = 0;
		uint8_t m_lineOrder = 0xFF;
	};

	/// <summary>
	///		Implements OpenEXR type interface to operate with data of "float" (32-bit) OpenEXR type + 
	///		method value() specifically for comfortable work with data of this type.
	/// </summary>
	class Float32 : public exrTypeBase
	{
		public:
		/// <summary>
		///		Constructor creates instance of base class and
		///			checks the type of stored OpenEXR data to be of same size as its OpenEXR equivalent,
		///			reads that value (or those components of value) from filebytes.
		/// </summary>
		/// <param name="filebytes"> vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="box2iFirstByteIndex"> index of first byte of box2i value within "filebytes" vector of input .exr file bytes </param>
		Float32(const std::vector<ui8>& filebytes, const uint32_t floatFirstByteIndex)
			: exrTypeBase(floatFirstByteIndex, sizeInBytes())
		{
			tryValidateSizeIs(exr::consta::TypeValueSizeBytes::s_float32);
			m_float32 = readFloat32((void*)(filebytes.begin()+floatFirstByteIndex)._Ptr);
		}
		/// <summary>
		///		Get Float32 value.
		/// </summary>
		/// <returns> uint8_t numerical value stored by Float32 </returns>
		float value() const { return m_float32; }

		/// <summary> Implements interface: returns string of name of OpenEXR-defined data type stored within this class. </summary>
		/// <returns> std::string name of OpenEXR-defined type of data stored within this class </returns>
		std::string type() const override { return exr::consta::Type::s_float32; }
		/// <summary>
		///		Implements interface: returns number of bytes that OpenEXR data stored within this class takes, 
		///		which must be equal to the size (in bytes) of value of corresponding OpenEXR-defined data type 
		///		this class is implemented to store.
		/// </summary>
		/// <returns> uint32_t number of bytes taken by OpenEXR data stored within this class </returns>
		uint32_t sizeInBytes() const override { return sizeof(m_float32); }
		/// <summary> Implements interface: returns string containing values stored within class. </summary>
		/// <returns> std::string text with values stored by class </returns>
		std::string toString(const uint8_t tabsNum = 0) const override
		{
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			return tabs + utils::str(m_float32);
		}

		private:
		float m_float32;
	};

	/// <summary>
	///		V2f = vector of 2 floats.
	///		Implements OpenEXR type interface to operate with data of "v2f" (vector of 2 floats (32-bit)) OpenEXR type + 
	///		method byteOffset() and operator[] specifically for comfortable work with data of this type.
	/// </summary>
	class V2f : public exrTypeBase
	{
		public:
		/// <summary>
		///		Constructor creates instance of base class and
		///			checks the type of stored OpenEXR data to be of same size as its OpenEXR equivalent,
		///			initializes filebytes indexes of first and last bytes of each value component & 
		///			reads that value (or those components of value) from filebytes.
		/// </summary>
		/// <param name="filebytes"> vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="box2iFirstByteIndex"> index of first byte of box2i value within "filebytes" vector of input .exr file bytes </param>
		V2f(const std::vector<ui8>& filebytes, const uint32_t v2fFirstByteIndex)
			: exrTypeBase(v2fFirstByteIndex, sizeInBytes())
		{
			tryValidateSizeIs(exr::consta::TypeValueSizeBytes::s_v2f);
			m_v2f[0] = readFloat32((void*)(filebytes.begin()+v2fFirstByteIndex)._Ptr);
			m_v2f[1] = readFloat32((void*)(filebytes.begin()+v2fFirstByteIndex+byteOffset(1))._Ptr);
		}
		/// <summary>
		///		Get float component of value of v2f (vector of 2 floats) type.
		/// </summary>
		/// <param name="v2fFloatIndex"> - size_t index of float within the v2f array (first float has index=0) </param>
		/// <returns> floating point number of v2f array addressed by (v2fFloatIndex) index </returns>
		float operator[](const size_t v2fFloatIndex) const { return m_v2f[v2fFloatIndex]; }
		/// <summary>
		///		Get the byte offset (distance), from the start of the array, for a float value at (v2fFloatIndex) index in the array.
		/// </summary>
		/// <param name="v2fFloatIndex"> - uint32_t index of float within the v2f array (first float has index=0) </param>
		/// <returns> uint32_t number that represents byte-distance of float inside v2f array addressed by (v2fFloatIndex) index </returns>
		uint32_t byteOffset(const uint32_t v2fFloatIndex)
		{
			return v2fFloatIndex * sizeof(m_v2f[0]);
		}

		/// <summary> Implements interface: returns string of name of OpenEXR-defined data type stored within this class. </summary>
		/// <returns> std::string name of OpenEXR-defined type of data stored within this class </returns>
		std::string type() const override { return exr::consta::Type::s_v2f; }
		/// <summary>
		///		Implements interface: returns number of bytes that OpenEXR data stored within this class takes, 
		///		which must be equal to the size (in bytes) of value of corresponding OpenEXR-defined data type 
		///		this class is implemented to store.
		/// </summary>
		/// <returns> uint32_t number of bytes taken by OpenEXR data stored within this class </returns>
		uint32_t sizeInBytes() const override { return m_v2fLen * sizeof(m_v2f[0]); }
		/// <summary> Implements interface: returns string containing values stored within class. </summary>
		/// <returns> std::string text with values stored by class </returns>
		std::string toString(const uint8_t tabsNum = 0) const override
		{
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			return tabs + "[" + utils::str(m_v2f[0]) + ", " + utils::str(m_v2f[1]) + "]";
		}

		private:
		static const uint32_t m_v2fLen = 2;
		float m_v2f[m_v2fLen] = {0};
	};

	/// Extend Interface implementations: USER MANUAL - step 1.
	/// 1. User implements exrValueTypeInterface_v2 storing other data of OpenEXR/custom type, for ex. MyV4F (4-float vector)
	///		and in constructor implements reading & initializing it from .exr filebytes,
	///		+ optionally, implements methods for comfortable workfloaw specifically with this data,
	///		+ implements virtual methods of base class (common for all OpenEXR value data types).
	
	/// <summary>
	///		Class that stores data and provides methods specific to all and each OpenEXR attribute
	///		(see source document at: https://openexr.com/en/latest/OpenEXRFileLayout.html#attribute-layout)
	///		+ useful methods and data to track where each attribute part is located in filebytes (and therefore, in .exr file).
	/// </summary>
	class AttribBase
	{
		// virtual	- prohibits creating instance of this class (requires to be overridden in derived class)
		// final	- means method is implemented and prohibited to be overriden by derived class(es)
		public:
		/// <summary>
		///		Get string with the specified name of OpenEXR attribute.
		///		This implementation is final and can not be overridden by derived class.
		/// </summary>
		/// <returns> std::string of the specified name of attribute </returns>
		virtual std::string name() const final { return m_name; }	// un-overridable in derived classes, Attrib.name		// rename to attrib_name()
		/// <summary>
		///		Get string with the name of OpenEXR data type of OpenEXR attribute (value data field is expected to be added in the derived class).
		///		This implementation is final and can not be overridden by derived class.
		/// </summary>
		/// <returns> std::string of the name of the data type stored by attribute value </returns>
		virtual std::string type() const final { return	m_type; }	// type of attrib = type of value	// rename to value_type()
		/// <summary>
		///		Get number of bytes that attribute value takes (value data field is expected to be added in the derived class).
		///		This implementation is final and can not be overridden by derived class.
		/// </summary>
		/// <returns> uint32_t size in bytes of the attribute value </returns>
		virtual uint32_t value_sizeInBytes() const final { return m_valueSizeBytes; }
		/// <summary>
		///		Get index of FIRST byte of attribute value within .exr file bytes.
		///		This implementation is final and can not be overridden by derived class.
		/// </summary>
		/// <returns> uint32_t index of FIRST byte of the attribute value within .exr file bytes (filebytes vector) </returns>
		virtual uint32_t value_firstByteIndex() const final { return m_value_firstByteIndex; }
		/// <summary>
		///		Get index of LAST byte of attribute value within .exr file bytes.
		///		This implementation is final and can not be overridden by derived class.
		/// </summary>
		/// <returns> uint32_t index of LAST byte of the attribute value within .exr file bytes (filebytes vector) </returns>
		virtual uint32_t value_lastByteIndex() const final { return m_value_lastByteIndex; }
		/// <summary>
		///		Get string with all data stored by this class + attribute value (value data field is expected to be added in the derived class).
		///		This implementation is final and can not be overridden by derived class.
		/// </summary>
		/// <returns> std::string text with all data of this class + attribute value declared outside (expected to be in derived class) </returns>
		virtual std::string toString(const uint8_t tabsNum, const exrTypeBase* attribValuePtr) const final
		{
			std::string tabs = "";
			for (uint8_t i = 0; i < tabsNum; i++)
			{
				tabs += "\t";
			}
			return 
				"[0x" + utils::hex(m_attrib_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_attrib_lastByteIndex, 4) + "] attribute:\n" 
				+ tabs + "\t [0x" + utils::hex(m_name_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_name_lastByteIndex, 4) + "] name = \'" + name() + "\' + \'\\0\' \n"
				+ tabs + "\t [0x" + utils::hex(m_type_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_type_lastByteIndex, 4) + "] type = \'" + type() + "\' + \'\\0\' \n"
				+ tabs + "\t [0x" + utils::hex(m_valueSize_firstByteIndex, 4) + " ~ 0x" + utils::hex(m_valueSize_lastByteIndex, 4) + "] value size = 0x" + utils::hex(m_valueSizeBytes) + " = " + std::to_string(m_valueSizeBytes) + " bytes \n" 
				+ tabs + "\t [0x" + utils::hex(value_firstByteIndex(), 4) + " ~ 0x" + utils::hex(value_lastByteIndex(), 4) + "] value: \n" 
				+ attribValuePtr->toString(2);
		}

		protected:
		/// <summary>
		///		Constructor initializes standard data members for OpenEXR attribute: name, value type, value size in bytes.
		///		It does analysis of filebytes, searching for the specified attribute name, and if found, checks that name,
		///			saves the attribute value type (std::string of name of type) and number of bytes the value takes in .exr file and in filebytes byte vector.
		///		The value itself is expected to be specified in the derived class.
		///		Constructor is protected to prohibit creating an instance of this class by user.
		/// </summary>
		/// <param name="attribNameToAnalyse"> - name of the attribute to search for ana analyse </param>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="versionFieldBit10"> - bit 10 of Version Field of input .exr file </param>
		AttribBase(const std::string& attribNameToAnalyse, const std::vector<ui8>& filebytes, const bool versionFieldBit10)
		{
			// from filebytes, unpack common attrib parts: name, value type, value size in bytes
			// read nameToAnalyse
			m_name = attribNameToAnalyse;
			std::vector<ui8> nameVec(m_name.cbegin(), m_name.cend());
			std::vector<ui8>::const_iterator attribNameBegin = std::search(filebytes.cbegin(), filebytes.cend(), nameVec.cbegin(), nameVec.cend());
			if (attribNameBegin == filebytes.cend())
			{
				throw std::runtime_error("attribute (attribute name) not found");
			}
			m_name_firstByteIndex = std::distance(filebytes.cbegin(), attribNameBegin);
			m_attrib_firstByteIndex = m_name_firstByteIndex;
			// analyse attribute name found
			// attribute name already saved (before looking for it in filebytes), checking the final '\0' left.
			uint32_t attribNameLen = m_name.length();
			exrTypes::isValidExrAttributeNameLength(m_name, versionFieldBit10);
			if (filebytes[m_name_firstByteIndex + attribNameLen] != '\0')	// if attribNameString is not null-terminated
			{
				throw std::runtime_error("attribute name string is not null-terminated. Unable to continue analysis.");
			}
			m_name_lastByteIndex = m_name_firstByteIndex + attribNameLen;	// already considers '\0'
			// read attribute type string
			m_type_firstByteIndex = m_name_lastByteIndex + 1; // +1 considers '\0'
			m_type = exrTypes::readCString(filebytes.cbegin()+m_type_firstByteIndex, filebytes.cend());
			m_type_lastByteIndex = m_type_firstByteIndex + m_type.length();
			// read attribute value size (in bytes)
			m_valueSize_firstByteIndex = m_type_lastByteIndex + 1;
			m_valueSize_lastByteIndex = m_valueSize_firstByteIndex + 3; // int32_t is 4 bytes => offset= 3
			m_valueSizeBytes = exrTypes::readUint32((filebytes.cbegin()+m_valueSize_firstByteIndex)._Ptr);
			// define value byte index area
			m_value_firstByteIndex = m_valueSize_lastByteIndex + 1;
			m_value_lastByteIndex = m_value_firstByteIndex + m_valueSizeBytes-1;
			m_attrib_lastByteIndex = m_value_lastByteIndex;	// not true for chlist, because it ends with 0x00 terminator (+1 byte)
		}
		/// <summary>
		///		Call to validate that the type string retrieved from .exr filebytes is the same as type required by your class (which is derived from interface).
		/// </summary>
		/// <param name="requiredType"> - string of name of the attribute value type required by your derived class </param>
		inline void tryValidateTypeIs(const std::string& requiredType) const
		{
			if (strcmp(type().c_str(), requiredType.c_str()))	// if strcmp returns non-zero => strings are different
			{
				throw std::runtime_error("undefined or software-specific (defined by 3rd-party software) attribute type");
			}
		}

		protected:
		uint32_t m_attrib_firstByteIndex = 0, m_attrib_lastByteIndex = 0;
		uint32_t m_name_firstByteIndex = 0, m_name_lastByteIndex = 0;
		uint32_t m_type_firstByteIndex = 0, m_type_lastByteIndex = 0;
		uint32_t m_valueSize_firstByteIndex = 0, m_valueSize_lastByteIndex = 0;
		uint32_t m_value_firstByteIndex = 0, m_value_lastByteIndex = 0;
		std::string m_name;
		std::string m_type;
		uint32_t m_valueSizeBytes = 0;

	};

	/// <summary>
	///		Provides methods and stores data for comfortable workflow with OpenEXR .exr file attribute.
	///		Inherits methods and data from interface that are common for attributes no matter their value type
	///		+ implements those inherited methods that need implementation.
	/// </summary>
	class AttribChlist : public AttribBase
	{
		public:
		/// <summary>
		///		Initialize each attribute common data members by calling base class constructor,
		///		analyse input .exr filebytes and initialize the attribute value of implemented type,
		///		and check the type of attribute found in .exr filebytes is the same as the required value type of this attribute.
		/// </summary>
		/// <param name="attribName"> - name of the attribute to search for ana analyse </param>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="versionFieldBit10"> - bit 10 of Version Field of input .exr file </param>
		AttribChlist(const std::string& attribName, const std::vector<ui8>& filebytes, const bool versionFieldBit10)
			: 
			AttribBase(attribName, filebytes, versionFieldBit10),
			// set attrib value from filebytes using attribValue_FirstByteIndex from base class, set base.attribValue_LastByteIndex
			m_chlist(Chlist(filebytes, value_firstByteIndex(), versionFieldBit10))
		{
			tryValidateTypeIs(exr::consta::Type::s_chlist);
		}
		
		std::string channelName(const uint32_t const channelIndex)
		{
			m_chlist.tryValidateChannelIndex(channelIndex);
			return m_chlist.channelName(channelIndex);
		}
		
		std::vector<std::string> channelsNames() const { return m_chlist.channelsNames(); }
		
		uint32_t channelsNum() const { return m_chlist.channelsNum(); }

		/// <summary>
		///		Warning: 
		///		this does not override the toString() method from the base class, instead this method calls it specifying
		///		the value to fetch data from to return the whole string containing data from both, base class and this derived class.
		/// </summary>
		/// <param name="tabsNum"> - the indent of each line inside the result string, use to indent the result </param>
		/// <returns> std::string text containing data names and their values of OpenEXR attribute stored </returns>
		std::string toString(const uint8_t tabsNum = 0) const
		{
			return AttribBase::toString(tabsNum, &m_chlist);
		}

		std::string channelDataTypeName(const uint32_t channelIndex) const
		{
			uint32_t channelIndexMin = 0;
			uint32_t channelIndexMax = m_chlist.channelsNum()-1;
			if (channelIndex < channelIndexMin or channelIndexMax < channelIndex)
			{
				throw std::invalid_argument
				(
					"(channelIndex) = " + std::to_string(channelIndex) + " is out of valid "
					"range [" + std::to_string(channelIndexMin) + "; " + std::to_string(channelIndexMax) + "]");
			}
			return m_chlist.channelDataTypeName(channelIndex);
		}

		private:
		Chlist m_chlist;
	};

	/// <summary>
	///		Provides methods and stores data for comfortable workflow with OpenEXR .exr file attribute.
	///		Inherits methods and data from interface that are common for attributes no matter their value type
	///		+ implements those inherited methods that need implementation.
	/// </summary>
	class AttribCompression : public AttribBase
	{
		public:
		/// <summary>
		///		Initialize each attribute common data members by calling base class constructor,
		///		analyse input .exr filebytes and initialize the attribute value of implemented type,
		///		and check the type of attribute found in .exr filebytes is the same as the required value type of this attribute.
		/// </summary>
		/// <param name="attribName"> - name of the attribute to search for ana analyse </param>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="versionFieldBit10"> - bit 10 of Version Field of input .exr file </param>
		AttribCompression(const std::string& attribName, const std::vector<ui8>& filebytes, const bool versionFieldBit10)
			: 
			AttribBase(attribName, filebytes, versionFieldBit10),
			// set attrib value from filebytes using attribValue_FirstByteIndex from base class, set base.attribValue_LastByteIndex
			m_compression(Compression(filebytes, value_firstByteIndex()))
		{
			tryValidateTypeIs(exr::consta::Type::s_compression);
		}

		std::string compressionName() const { return m_compression.name(); }

		/// <summary>
		///		Get the compression value byte stored in the .exr file.
		///		See OpenEXR technicla documents to identify the compression algorithm used in this .exr file.
		/// </summary>
		/// <returns> uint6_t - byte value stored in the .exr file that represents compression algorithm .exr file is using </returns>
		uint8_t value() const
		{
			return m_compression.value();
		}
		/// <summary>
		///		Warning: 
		///		this does not override the toString() method from the base class, instead this method calls it specifying
		///		the value to fetch data from to return the whole string containing data from both, base class and this derived class.
		/// </summary>
		/// <param name="tabsNum"> - the indent of each line inside the result string, use to indent the result </param>
		/// <returns> std::string text containing data names and their values of OpenEXR attribute stored </returns>
		std::string toString(const uint8_t tabsNum = 0) const
		{
			return AttribBase::toString(tabsNum, &m_compression);
		}

		private:
		Compression m_compression;
	};

	/// <summary>
	///		Provides methods and stores data for comfortable workflow with OpenEXR .exr file attribute.
	///		Inherits methods and data from interface that are common for attributes no matter their value type
	///		+ implements those inherited methods that need implementation.
	/// </summary>
	class AttribBox2i : public AttribBase
	{
		public:
		/// <summary>
		///		Initialize each attribute common data members by calling base class constructor,
		///		analyse input .exr filebytes and initialize the attribute value of implemented type,
		///		and check the type of attribute found in .exr filebytes is the same as the required value type of this attribute.
		/// </summary>
		/// <param name="attribName"> - name of the attribute to search for ana analyse </param>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="versionFieldBit10"> - bit 10 of Version Field of input .exr file </param>
		AttribBox2i(const std::string& attribName, const std::vector<ui8>& filebytes, const bool versionFieldBit10)
			: 
			AttribBase(attribName, filebytes, versionFieldBit10),
			// set attrib value from filebytes using attribValue_FirstByteIndex from base class, set base.attribValue_LastByteIndex
			m_box2i(Box2i(filebytes, value_firstByteIndex()))
		{
			tryValidateTypeIs(exr::consta::Type::s_box2i);
		}
		
		Box2i value() const { return m_box2i; }
		/// <summary>
		///		Warning: 
		///		this does not override the toString() method from the base class, instead this method calls it specifying
		///		the value to fetch data from to return the whole string containing data from both, base class and this derived class.
		/// </summary>
		/// <param name="tabsNum"> - the indent of each line inside the result string, use to indent the result </param>
		/// <returns> std::string text containing data names and their values of OpenEXR attribute stored </returns>
		std::string toString(const uint8_t tabsNum = 0) const
		{
			return AttribBase::toString(tabsNum, &m_box2i);
		}

		private:
		Box2i m_box2i;
	};

	/// <summary>
	///		Provides methods and stores data for comfortable workflow with OpenEXR .exr file attribute.
	///		Inherits methods and data from interface that are common for attributes no matter their value type
	///		+ implements those inherited methods that need implementation.
	/// </summary>
	class AttribLineorder : public AttribBase
	{
		public:
		/// <summary>
		///		Initialize each attribute common data members by calling base class constructor,
		///		analyse input .exr filebytes and initialize the attribute value of implemented type,
		///		and check the type of attribute found in .exr filebytes is the same as the required value type of this attribute.
		/// </summary>
		/// <param name="attribName"> - name of the attribute to search for ana analyse </param>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="versionFieldBit10"> - bit 10 of Version Field of input .exr file </param>
		AttribLineorder(const std::string& attribName, const std::vector<ui8>& filebytes, const bool versionFieldBit10)
			: 
			AttribBase(attribName, filebytes, versionFieldBit10),
			m_lineOrder(LineOrder(filebytes, value_firstByteIndex()))
		{
			tryValidateTypeIs(exr::consta::Type::s_lineOrder);
		}
		
		uint8_t value() const { return m_lineOrder.value(); }
		
		/// <summary>
		///		Warning: 
		///		this does not override the toString() method from the base class, instead this method calls it specifying
		///		the value to fetch data from to return the whole string containing data from both, base class and this derived class.
		/// </summary>
		/// <param name="tabsNum"> - the indent of each line inside the result string, use to indent the result </param>
		/// <returns> std::string text containing data names and their values of OpenEXR attribute stored </returns>
		std::string toString(const uint8_t tabsNum = 0) const
		{
			return AttribBase::toString(tabsNum, &m_lineOrder);
		}

		private:
		LineOrder m_lineOrder;
	};

	/// <summary>
	///		Provides methods and stores data for comfortable workflow with OpenEXR .exr file attribute.
	///		Inherits methods and data from interface that are common for attributes no matter their value type
	///		+ implements those inherited methods that need implementation.
	/// </summary>
	class AttribFloat32 : public AttribBase
	{
		public:
		/// <summary>
		///		Initialize each attribute common data members by calling base class constructor,
		///		analyse input .exr filebytes and initialize the attribute value of implemented type,
		///		and check the type of attribute found in .exr filebytes is the same as the required value type of this attribute.
		/// </summary>
		/// <param name="attribName"> - name of the attribute to search for ana analyse </param>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="versionFieldBit10"> - bit 10 of Version Field of input .exr file </param>
		AttribFloat32(const std::string& attribName, const std::vector<ui8>& filebytes, const bool versionFieldBit10)
			: 
			AttribBase(attribName, filebytes, versionFieldBit10),
			m_float32(Float32(filebytes, value_firstByteIndex()))
		{
			tryValidateTypeIs(exr::consta::Type::s_float32);
		}

		float value() const { return m_float32.value(); }

		/// <summary>
		///		Warning: 
		///		this does not override the toString() method from the base class, instead this method calls it specifying
		///		the value to fetch data from to return the whole string containing data from both, base class and this derived class.
		/// </summary>
		/// <param name="tabsNum"> - the indent of each line inside the result string, use to indent the result </param>
		/// <returns> std::string text containing data names and their values of OpenEXR attribute stored </returns>
		std::string toString(const uint8_t tabsNum = 0) const
		{
			return AttribBase::toString(tabsNum, &m_float32);
		}

		private:
		Float32 m_float32;
	};

	/// <summary>
	///		Provides methods and stores data for comfortable workflow with OpenEXR .exr file attribute.
	///		Inherits methods and data from interface that are common for attributes no matter their value type
	///		+ implements those inherited methods that need implementation.
	/// </summary>
	class AttribV2f : public AttribBase
	{
		public:
		/// <summary>
		///		Initialize each attribute common data members by calling base class constructor,
		///		analyse input .exr filebytes and initialize the attribute value of implemented type,
		///		and check the type of attribute found in .exr filebytes is the same as the required value type of this attribute.
		/// </summary>
		/// <param name="attribName"> - name of the attribute to search for ana analyse </param>
		/// <param name="filebytes"> - vector of bytes (uin8_t / unsigned char) of full input .exr file </param>
		/// <param name="versionFieldBit10"> - bit 10 of Version Field of input .exr file </param>
		AttribV2f(const std::string& attribName, const std::vector<ui8>& filebytes, const bool versionFieldBit10)
			: 
			AttribBase(attribName, filebytes, versionFieldBit10),
			m_v2f(V2f(filebytes, value_firstByteIndex()))
		{
			tryValidateTypeIs(exr::consta::Type::s_v2f);
		}
		/// <summary>
		///		Warning: 
		///		this does not override the toString() method from the base class, instead this method calls it specifying
		///		the value to fetch data from to return the whole string containing data from both, base class and this derived class.
		/// </summary>
		/// <param name="tabsNum"> - the indent of each line inside the result string, use to indent the result </param>
		/// <returns> std::string text containing data names and their values of OpenEXR attribute stored </returns>
		std::string toString(const uint8_t tabsNum = 0) const
		{
			return AttribBase::toString(tabsNum, &m_v2f);
		}

		private:
		V2f m_v2f;
	};

	/// Extend Interface implementations: USER MANUAL - steps 2, 3.
	/// 2. User implements AttribInterface_v2 storing the attribute value object of previously implemented class (MyV4F), 
	///		and in constructor, initialize the base class, initialize the attribute value object (for ex., by creating your data type object, u_value(MyV4F(...input-params...)) )
	///		and implement your toString() method as follows: ... toString(...) ... { return AttribInterface_v2::toString(u_value, ...) }
	///	3. Your custom OpenEXR attribute containing custom data type is DONE ! Use it to read, analyse and store its data from .exr file, like this
	///		1| main()
	///		2| {
	///		3|		exrfile = exrRead("myfile.exr");
	///		4|		exrTypes::AttribMyV4F myV4f = exrTypes::AttribMyV4F("myV4f", exrfile.filebytes, exrfile.versionfield.bit10);
	///		5|		printf("%s \n", myV4f.toString().c_str());
	///		6|		return 0;
	///		7| }
	///
	


	// ------------------------------

	// not enough clear documentation, see https://openexr.com/en/latest/OpenEXRFileLayout.html#component-four-offset-tables (last access 2025.04.02)
	// therefore, implementation of a class is not comprehensive (does not work with non-RegularScanLine image)
	class OffsetTable
	{
		public:
		// oneOffsetSizeInBytes value is based on OpenEXR sample file explanation (https://openexr.com/en/latest/OpenEXRFileLayout.html#sample-file) 
		// and my .exr files observation in binary (byte) form.
		static const uint8_t offsetValueSizeInBytes = 8;	// therefore, offsetTable stores uint64_t (8-byte values)

		public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="filebytes"></param>
		/// <param name="offsetTableFirstByteIndex"></param>
		/// <param name="dataWindowYMax"></param>
		/// <param name="versionFieldBit12_isMultipart"></param>
		/// <param name="doesExrHas_chunkCount_Attribute"></param>
		OffsetTable(const std::vector<ui8>& filebytes, const uint32_t offsetTableFirstByteIndex, const uint32_t dataWindowYMax, const bool versionFieldBit12_isMultipart = 0, const bool doesExrHas_chunkCount_Attribute = 0)
		{
			/// ReadMe!
			// I didnt find explanation on how to calculate number or size of offsetTable 
			// (what does it mean "computed based on dataWindow, tileDesc and compression"? - see OpenEXR document)
			// so, I'm just trying & guessing the number of offsetTable entries.
			// Nevertheless, OpenEXR documentation at "document tag [EXR-SCANLINES-01]" kinda gives a hint that (offsetTableLen = exrImage.rows).
			// To set (offsetTableLen = exrImage.rows), you can use dataWindow attribute -> yMax value like this: offsetTableLen = dataWindow.yMax()+1;
			/// For now, (offsetTableLen) equals number of image rows.
			/// This works for "Regular scan line image block layout" + NO_COMPRESSION, but may be not fully correct calculation
			const uint32_t offsetTableLen = dataWindowYMax+1;	// dataWindowYMax+1 = number of rows of exr image

			// not yet implemented ------ v v v
			if (!versionFieldBit12_isMultipart)
			{
				// if chunkCount is not present => number of entries = compute(dataWindow, tileDesc, compression);
				if (!doesExrHas_chunkCount_Attribute)
				{
					// offsetTableLen = 
				}
			}
			else
			{
				// header must contain a chunkCount attribute, which indicates the size of the table and the number of chunks.
			}

			// read and save offset table
			for (uint32_t i = offsetTableFirstByteIndex, offsetIndex = 0; offsetIndex < offsetTableLen; i += sizeof(uint64_t), offsetIndex++)
			{
				uint64_t offset = exrTypes::readUint64(filebytes.data()+i);
				if(offset < 0 or filebytes.size()-1 < offset)	// if offset is out of valid range [0; filebytes.size-1]
				{
					throw std::logic_error("Invalid (offset) value received: (offset=" + std::to_string(offset) + ") is out of valid range [0; filebytes.size()-1]");
				}
				m_offsetTable.push_back(utils::IndexedValue(i, i+OffsetTable::offsetValueSizeInBytes-1, offset));
			}
		}
		uint32_t length() const { return m_offsetTable.size(); }
		uint32_t lastByteIndex() const { return m_offsetTable[length()-1].lastByteIndex(); }	// last byte of last offset in the table
		std::string toString(const uint32_t tableEntryIndex) const 
		{
			if (length()-1 < tableEntryIndex)
			{
				throw std::invalid_argument("(tableEntryIndex) is out of valid range [0; tableEntryLength - 1]");
			}
			// print out the offset table & update offsetIndex
			utils::IndexedValue entry = m_offsetTable[tableEntryIndex];
			return 
				"i= " + std::to_string(tableEntryIndex) + 
				": [0x" + utils::hex(entry.firstByteIndex(),4) + " ~ 0x" + utils::hex(entry.lastByteIndex(),4) + 
				"] offset= " + std::to_string(entry.value()) + " = 0x" + utils::hex64(entry.value());
		}

		private:
		std::vector<utils::IndexedValue<uint64_t>> m_offsetTable;
	};

}
