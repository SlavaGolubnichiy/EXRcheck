#pragma once
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <type_traits>	// required by C++20 concept (type constraints)
#include <vector>

#include "types.h"
#include "TemplateConcepts.h"

namespace utils
{
	// classes

	// info on C++20: concept -> see TemplateConcepts.h
	template <templateConcept::numerical::StdNumeric NumberT>
	class Range
	{
		public:
		Range() {}
		Range(const NumberT first, const NumberT last)
			: u_first(first), u_last(last)
		{
			/*
				// Add step parameter ?
				if ((0 < step) && !(first < last)) throw std::invalid_argument("if (step) > 0 then must be (first) < (last). Something is not valid.");
				if ((step < 0) && !(last < first)) throw std::invalid_argument("if (step) < 0 then must be (last) < (first). Something is not valid.");
				if ((step == 0) && !(first == last)) throw std::invalid_argument("if (step) = 0 then must be (last) = (first). Something is not valid.");
			*/
		}
		Range(const Range& other) : u_first(other.u_first), u_last(other.u_last) {}
		Range& operator=(const Range& other)
		{
			if (this != &other)		// protect against self-initialization
			{
				u_first = other.u_first;
				u_last = other.u_last;
			}
			return *this;
		}
		NumberT first() const { return u_first; }
		NumberT last() const { return u_last; }

		private:
		NumberT u_first = 0, u_last = 0;

	};

	template <templateConcept::functional::CopyAssignable ValueType>
	class IndexedValue
	{
		public:
		IndexedValue(const uint32_t valueFirstByteIndex, const uint32_t valueLastByteIndex, ValueType value)
			: u_valueByteRange(utils::Range<uint32_t>(valueFirstByteIndex, valueLastByteIndex)), u_value(value)
		{
		}
		IndexedValue(const IndexedValue<ValueType>& other)
			: u_valueByteRange(other.u_valueByteRange), u_value(other.u_value)
		{
		}
		IndexedValue& operator=(const IndexedValue& other)
		{
			if (this != &other)
			{
				u_valueByteRange = other.u_valueByteRange;
				u_value = other.u_value;
			}
			return *this;
		}
		ValueType value() const { return u_value; }
		uint32_t firstByteIndex() const { return u_valueByteRange.first(); }
		uint32_t lastByteIndex() const { return u_valueByteRange.last(); }

		private:
		ValueType u_value;
		utils::Range<uint32_t> u_valueByteRange;
	};

	// static functions

	static std::string tabs(const uint32_t tabsNum)
	{
		std::string tabs = "";
		for (uint32_t i = 0; i < tabsNum; i++)
		{
			tabs += "\t";
		}
		return tabs;
	}

	static bool doesFitByte(const uint64_t value) { return (0 < value) && (value <= 255); }

	

	static std::string hex64(const uint64_t uint64, const uint8_t outputWidth = 16)	// outputWidth= 2*sizeof(uint64_t)
	{
		std::stringstream result;
		result 
			<< std::setfill('0')		// set fill character to '0': " B" -> "0B"
			<< std::setw(outputWidth)	// set output width to 2*number of bytes: "B" -> " B"
			<< std::uppercase			// 
			<< std::hex << +uint64;		// promote uint8/char to int to output value, not ASCII character.
		return result.str();
	}

	static std::string hex(const uint32_t uint, const uint8_t outputWidth = 8)	// outputWidth = 2*sizeof(uint)
	{
		std::stringstream result;
		result 
			<< std::setfill('0')		// set fill character to '0': " B" -> "0B"
			<< std::setw(outputWidth)	// set output width to 2*number of bytes: "B" -> " B"
			<< std::uppercase			// 
			<< std::hex << +uint;		// promote uint8/char to int to output value, not ASCII character.
		return result.str();
	}
	
	/// when put to utils.h (& non-static), Build fails (with "hex() multiple definitions" error)
	static std::string hex(const std::vector<uint32_t>& vec, const uint8_t outputWidth = 2*sizeof(uint32_t))
	{
		std::stringstream result;
		for (const uint32_t& uint : vec)
		{
			result << hex(uint, outputWidth) << " ";
		}
		return result.str();
	}

	static std::string str(const float value, const int32_t precision = -1, bool isScientific = false)
	{
		std::stringstream stream;
		stream << (isScientific ? std::scientific : std::fixed);
		if (precision != -1)
		{
			stream << std::setprecision(precision);
		}
		stream << value;
		return stream.str();
	}

	template<typename T>
	void printVector(std::vector<T> vec, bool withIndexes=false, std::string separator = " ")
	{
		if (!withIndexes)
		{
			for(T element: vec)
			{
				std::cout << element << separator;
			}
		}
		else
		{
			unsigned int vecLen = vec.size();
			for(unsigned int i = 0; i < vecLen; i++)
			{
				std::cout << i << ": " << vec[i] << " \n";
			}
		}
	}

	template<typename T>
	std::vector<T> slice(const std::vector<T>& input, const unsigned int firstIndex, const unsigned int lastIndex)
	{
		if (firstIndex < 0 or (input.size()-1) < firstIndex)
		{
			throw std::invalid_argument("firstIndex must be in range [0; input.size)");
		}
		if (lastIndex < 0 or (input.size()-1) < lastIndex)
		{
			throw std::invalid_argument("lastIndex must be in range [0; input.size]");
		}

		if (lastIndex < firstIndex)
		{
			throw std::invalid_argument("lastIndex must be greater or equal to firstIndex");
		}
		unsigned int sliceLen = lastIndex - firstIndex + 1;

		std::vector<T> slice(input.begin()+firstIndex, input.begin()+lastIndex+1);
		return slice;
	}

	namespace file
	{
		/// <summary>
		///		Determine the size of "filename" file measured in bytes.
		/// </summary>
		/// <param name="filename">  - name, relative path or absolute path of the target file  </param>
		/// <returns> Size of the target file in bytes </returns>
		static unsigned int fileSizeBytes(const char* filename)
		{
			std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
			unsigned int filesize = unsigned int(file.tellg());
			file.close();
			return filesize;
		}

		/// <summary>
		///		Read file in binary mode (byte by byte). 
		///		The result std::vector<u8> should contain the same number of bytes as file size in bytes.
		///		* C++ style function utilizes ifstream, but reads file byte-by-byte so performance may not be the best (unverified).
		///		** Compatible with C ? (unverified).
		/// </summary>
		/// <param name="filename"> - name, relative path or absolute path of the target file </param>
		/// <returns> Vector of bytes retrieved from file in binary mode </returns>
		static std::vector<u8> getFilebytes_v4_isCcompatible(const char* filename)
		{
			unsigned int filesizeB = fileSizeBytes(filename);
			std::ifstream file(filename, std::fstream::in | std::ifstream::binary);
			if (!file)
			{
				std::cerr << "Error opening file!\n";
				throw std::runtime_error("Error opening file!");
			}

			// read file
			const unsigned int filebytesNum = 674;
			u8 filebytes[filebytesNum] = {0};
			char byte;
			for (unsigned int i = 0; i < filesizeB; i++)
			{
				// std::streampos filePos = file.tellg();	// current position in file after READ/WRITE
				// printf("filepos= %8u : ", unsigned int(filePos));
				file.read(&byte, 1);
				if (file.fail())
				{
					// "failbit" or "badbit" is set in "rdstate".
					throw std::runtime_error("File operation failed: READ");
				}
				filebytes[i] = byte & 0xFF;
			}
			file.close();
			std::vector<u8> filebytesVec(filebytes, filebytes+filebytesNum); // array to vector
			if (filebytesVec.size() != filesizeB)
			{
				throw std::runtime_error("Number of bytes read is different than file size in bytes. Reading may lost bytes or saved extra (false) bytes.");
			}
			return filebytesVec;
		}

		/// <summary>
		///		Read file in binary mode (byte by byte). 
		///		The result std::vector<u8> should contain the same number of bytes as file size in bytes.
		///		* C++ style function utilizes std::istreambuf_iterator, so reading performance may be better (untested).
		///		** May be incompatible with C (unverified).
		/// </summary>
		/// <param name="filename">  - name, relative path or absolute path of the target file  </param>
		/// <returns> Vector of bytes retrieved from file in binary mode </returns>
		static std::vector<u8> getFilebytes_v5_CppOnly(const char* filename)
		{
			//if (filename.empty())
			//{
			//	throw std::invalid_argument("Filename is empty string. Please, provide existing file path.");
			//}

			unsigned int filesizeB = fileSizeBytes(filename);
			std::ifstream file(filename, std::fstream::in | std::ifstream::binary);
			if (!file)
			{
				throw std::runtime_error("Error opening file! Check the file is in the same directory with .exe / VS-project used.");
			}
			std::vector<u8> filebytesVec(std::istreambuf_iterator<char>(file), {});		// read file (copy all data into "filebytes")
			file.close();
			if (filebytesVec.size() != filesizeB)
			{
				throw std::runtime_error("Number of bytes read is different than file size in bytes. Reading may lost bytes or saved extra (false) bytes.");
			}
			return filebytesVec;
		}
	}

	namespace print
	{
		static void asTable(const u8* arr, const unsigned int arrLen, const unsigned int columns = 16, const std::string elemPrintfFormat = "%u", const std::string elemSeparator = " ", const std::string rowSeparator = "\n")
		{
			// const unsigned int arrLen = sizeof(arr);
			if (columns < 1 or arrLen < columns)
			{
				throw std::invalid_argument("columns must be in range [1; arrLength]");
			}

			if (columns == 16)
			{
				printf("_________ +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f \n");
			}
			for(unsigned int i = 0; i < arrLen; i++)
			{
				if (i % columns == 0)
				{
					printf("%8.8x: ", i); // row index offset
				}
				printf(elemPrintfFormat.c_str(), arr[i]);
				printf(elemSeparator.c_str());
				if ((i+1) % columns == 0)
				{
					printf(rowSeparator.c_str());
				}
			}
		}

		static void asChars(const u8* bytes, const unsigned int bytesNum)
		{
			{
				u8 test = bytes[bytesNum]; // try read last element
			}
			std::cout.write((char*)bytes, bytesNum);
		}

		static void asBytes(const u8* bytes, const unsigned int bytesNum)
		{
			asTable(bytes, bytesNum, 16, "%2.2X");
		}

		static void asCharUint8Byte(const u8* bytes, const unsigned int bytesNum)
		{
			for (unsigned int i = 0; i < bytesNum; i++)
			{
				printf("i= %8u : '%c' = %.3u = %2.2X \n", i, bytes[i], bytes[i], bytes[i]);
			}
		}

		static void asChar(const std::vector<u8>& bytes, bool VIEW_TABLE=true)
		{
			if (VIEW_TABLE)
			{
				printf("___________ 0123456789abcdef \n");
				for(unsigned int i = 0; i < bytes.size(); i++)
				{
					if (i % 16 == 0)
					{
						printf("0x%8.8x: ", i); // print data offset within file
					}
					printf("%c", bytes[i]);
					if ((i+1) % 16 == 0)
					{
						printf("\n");
					}
				}
			}
			else
			{
				utils::printVector<u8>(bytes, false, "");
			}
		}

		static void asBytes(const std::vector<u8>& bytes)
		{
			printf("___________ +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f \n");
			for(unsigned int i = 0; i < bytes.size(); i++)
			{
				if (i % 16 == 0)
				{
					printf("0x%8.8x: ", i); // print data offset within file
				}
				printf("%2.2X ", bytes[i]);
				if ((i+1) % 16 == 0)
				{
					printf("\n");
				}
			}
		}

		static void asCharUint8Byte(const std::vector<u8>& bytes)
		{
			for(unsigned int i = 0; i < bytes.size(); i++)
			{
				printf("%3.u : \'%c\' = %3.3u = %2.2X \n", i, bytes[i], bytes[i], bytes[i]);
			}
		}
	};

}
