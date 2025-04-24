#pragma once

#include <memory>
#include <string>
#include <vector>
#include "exrData/exrConsta.h"
#include "exrData/exrTypes.h"
#include "exrData/MagicNumber.h"
#include "exrData/Pixeldata.h"
#include "exrData/VersionField.h"
#include "types.h"

std::string exrToUserChannelDataTypeName(const std::string exrStandardChannelDataTypeName)
{
	if (exrStandardChannelDataTypeName == exr2::consta::channel::channelDataTypeName(exr2::consta::channel::datatype::UINT))		return "32-bit Unsigned Integer";
	else if (exrStandardChannelDataTypeName == exr2::consta::channel::channelDataTypeName(exr2::consta::channel::datatype::HALF))	return "16-bit Floating point";
	else if (exrStandardChannelDataTypeName == exr2::consta::channel::channelDataTypeName(exr2::consta::channel::datatype::FLOAT))	return "32-bit Floating point";
	else return "<unknown-type. Contact developer>";
}

class exrFileData
{
	public:

	exrFileData(std::vector<ui8>& filebytes) 
		: m_filebytes(filebytes)
	{
	}

	/// <summary>
	///		Read file byte-by-byte, verify its a valid .exr file and, if so, analyse it
	///		and print the analysis results to the console.
	/// </summary>
	/// <param name="filename"> Vector of bytes retrieved from file </param>
	/// <returns> void </returns>
	void exrAnalysisDetailed()
	{
		printf("-------- Magic number & Version field -------- \n");
		/// Magic number (allows validating input file)
		saveAndPrintMagicNumber();
		/// OpenEXR version field (version of OpenEXR standard + other info)
		m_vf = std::make_unique<VersionField>(m_filebytes);
		if (m_vf->exrVersion() == exr2::consta::c_versionNumber)
		{
			if (not m_vf->isValidExr2_0())
			{
				printf("EXR file version field is NOT VALID. Further analysis suspended.");
				return;
			}
		}
		else
		{
			printf("EXR file uses OpenEXR standard version different from 2.x. Further analysis results may be incorrect.");
		}
		m_vf->printData();
		printf("\n");

		/// OpenEXR header attributes
		printf("-------- Header Attributes -------- \n");
		// .exr header required (standard)
		saveAndPrintExrHeaderRequiredAttribs();
		// .exr header custom (optional) attributes
		saveAndPrintExrHeaderCustomAttribs();
		if (m_filebytes[m_exrHeaderFinalNullIndex] == 0x00)
		{
			printf("[0x%s] byte = 0x%2.2X (end of .exr header section) \n\n", utils::hex(m_exrHeaderFinalNullIndex, 4).c_str(), 0x00);
		}
		else
		{
			throw std::runtime_error("WARNING: 0x00 byte ending the header section with attributes is not found. Further analysis suspended (impossible).");
		}

		saveAndPrintExrPixeldata();
		if (m_pixelData->lastByteIndex() == m_filebytes.size()-1)
			printf("-------- End of .exr file. --------\n\n");
		else
			printf("-------- End of file is not reached. Your file hmay have more than expected. Check file bytes above.\n\n");

		printAnalysisSummary();

	}

	private:
	// exr file header
	std::vector<ui8>& m_filebytes;
	int32_t m_magicNumber = 0;
	// brief example on unique_ptr: 
	// {
	//	std::unique_ptr<MyObject> pointer;
	//	pointer = std::make_unique<MyObject>(/* MyObject constructor parameters */);
	//	pointer->doSomething();		// use pointer as usual
	//	// no need to "delete" manually - it is automatically cleaned-up here
	// }
	std::unique_ptr<VersionField> m_vf = nullptr;
	std::unique_ptr<exrTypes::AttribChlist> m_chlist = nullptr;				// automatically deletes itself when out of scope
	std::unique_ptr<exrTypes::AttribCompression> m_compression = nullptr;
	std::unique_ptr<exrTypes::AttribBox2i> m_dataWindow = nullptr;
	std::unique_ptr<exrTypes::AttribBox2i> m_displayWindow = nullptr;
	std::unique_ptr<exrTypes::AttribLineorder> m_lineOrder = nullptr;
	std::unique_ptr<exrTypes::AttribFloat32> m_pixelAspectRatio = nullptr;
	std::unique_ptr<exrTypes::AttribV2f> m_screenWindowCenter = nullptr;
	std::unique_ptr<exrTypes::AttribFloat32> m_screenWindowWidth = nullptr;
	std::unique_ptr<exrTypes::AttribFloat32> m_xDensity = nullptr;	bool m_hasAttribute_xDensity = false;
	// additional analysis results
	bool m_doesRequire_chunkCount_Attribute = false;
	uint32_t m_exrHeaderFinalNullIndex = 0;
	uint32_t m_imageRows = 0, m_imageCols = 0;
	// image data
	std::unique_ptr<exrTypes::OffsetTable> m_offsetTable = nullptr;
	std::unique_ptr<exrPixeldata::PixelData<float>> m_pixelData = nullptr;

	void saveAndPrintMagicNumber()
	{
		/// OpenEXR magicNumber (.exr file validator)
		m_magicNumber = exr::magicNumber(m_filebytes);
		printf("[03~00] Magic number = 0x%8.8X = %i ", m_magicNumber, m_magicNumber);
		if (m_magicNumber == exr2::consta::c_magicNumber)
		{
			printf("(file is valid EXR). \n");
		}
		else
		{
			throw std::runtime_error("ERROR: file is NOT VALID exr. Further analysis suspended.");
			return;
		}
	}

	void saveAndPrintExrHeaderRequiredAttribs()		// if exrFileData_asFunctions already outdated -> delete it and remove "Exr" form this method name
	{
		m_chlist = std::make_unique<exrTypes::AttribChlist>(exr::consta::StdAttribName::s_channels, m_filebytes, m_vf->bit10_HasLongNames());
		printf("%s \n", m_chlist->toString().c_str());
		m_compression = std::make_unique<exrTypes::AttribCompression>(exr::consta::StdAttribName::s_compression, m_filebytes, m_vf->bit10_HasLongNames());		// must read attribute => no try-catch
		printf("%s \n", m_compression->toString().c_str());
		m_dataWindow = std::make_unique<exrTypes::AttribBox2i>(exr::consta::StdAttribName::s_dataWindow, m_filebytes, m_vf->bit10_HasLongNames());
		printf("%s \n", m_dataWindow->toString().c_str());
		printCaughtException(
			m_displayWindow = std::make_unique<exrTypes::AttribBox2i>(exr::consta::StdAttribName::s_displayWindow, m_filebytes, m_vf->bit10_HasLongNames());		// may not read attribute => enclosed in try-catch
			printf("%s \n", m_displayWindow->toString().c_str());
		);
		m_lineOrder = std::make_unique<exrTypes::AttribLineorder>(exr::consta::StdAttribName::s_lineOrder, m_filebytes, m_vf->bit10_HasLongNames());				// must read attribute
		printf("%s \n", m_lineOrder->toString().c_str());
		printCaughtException(
			m_pixelAspectRatio = std::make_unique<exrTypes::AttribFloat32>(exr::consta::StdAttribName::s_pixelAspectRatio, m_filebytes, m_vf->bit10_HasLongNames());
			printf("%s \n", m_pixelAspectRatio->toString().c_str());
		);
		printCaughtException(
			m_screenWindowCenter = std::make_unique<exrTypes::AttribV2f>(exr::consta::StdAttribName::s_screenWindowCenter, m_filebytes, m_vf->bit10_HasLongNames());
			printf("%s \n", m_screenWindowCenter->toString().c_str());
		);
		m_screenWindowWidth = std::make_unique<exrTypes::AttribFloat32>(exr::consta::StdAttribName::s_screenWindowWidth, m_filebytes, m_vf->bit10_HasLongNames());	// must read attribute for code below => no try-catch
		printf("%s \n", m_screenWindowWidth->toString().c_str());
	}

	void saveAndPrintExrHeaderCustomAttribs()		// if exrFileData_asFunctions already outdated -> delete it and remove "Exr" form this method name
	{
		try
		{
			m_xDensity = std::make_unique<exrTypes::AttribFloat32>("xDensity", m_filebytes, m_vf->bit10_HasLongNames());
			printf("%s \n", m_xDensity->toString().c_str());
			m_hasAttribute_xDensity = true;
		}
		catch(const std::exception e)
		{
			m_hasAttribute_xDensity = false;
		}
		if (m_hasAttribute_xDensity) m_exrHeaderFinalNullIndex = m_xDensity->value_lastByteIndex()+1;
		else						 m_exrHeaderFinalNullIndex = m_screenWindowWidth->value_lastByteIndex()+1;

		// if versionField.bit12==1 or versionField.bit11==1	=> then => attribute (name="chunkCount", type="int") must be in .exr
		bool doesRequire_chunkCount_Attribute = m_vf->bit12_IsMultipart() and m_vf->bit11_HasDeepData();
		/// to be implemented ...

	}

	void saveAndPrintExrPixeldata()		// if exrFileData_asFunctions already outdated -> delete it and remove "Exr" form this method name
	{
		bool isExrCompressed = (m_compression->value() != exr2::consta::s_compression::NO);
		if (isExrCompressed)
		{
			throw std::logic_error("WARNING: [0x" + utils::hex(m_compression->value_firstByteIndex(), 2) + " ~ " + utils::hex(m_compression->value_lastByteIndex(), 2) + "] compression is not NO_COMPRESSION. OffsetTable & pixelData will not be read. Analysis stops here, after header section.\n\n");
		}

		/// OpenEXR image data section
		/* document tag [OPENEXR-OFFSET-TABLE-01] */
		printf("-------- Offset Table -------- \n");
		m_offsetTable = std::make_unique<exrTypes::OffsetTable>(m_filebytes, m_exrHeaderFinalNullIndex+1, m_dataWindow->value().yMax(), m_vf->bit12_IsMultipart(), m_doesRequire_chunkCount_Attribute);
		printf("%s \n", m_offsetTable->toStringAllEntries().c_str());

		/* document tag [OPENEXR-PIXEL-DATA-01] */
		m_imageRows = uint32_t(m_dataWindow->value().yMax() + 1);
		m_imageCols = uint32_t(m_dataWindow->value().xMax() + 1);
		uint32_t imageChannelsNum = m_chlist->channelsNum();
		std::vector<std::string> channelsNames = m_chlist->channelsNames();
		printf("-------- Pixel Data -------- \n");
		m_pixelData = std::make_unique<exrPixeldata::PixelData<float>>(m_filebytes, m_offsetTable->lastByteIndex()+1, m_imageRows, m_imageCols, imageChannelsNum, m_lineOrder->value());
		printf("%s", m_pixelData->toStringAsExrPixeldata(channelsNames).c_str());
	}

	void printAnalysisSummary() const
	{
		printf("\n");
		printf("------------------------------------- \n");
		printf("-------- Summary (user-view) -------- \n");
		printf("------------------------------------- \n");
		printf(".exr file info: \n");
		printf("size: _______________ %u x %u \n", m_imageCols, m_imageRows);
		printf("channels (ordered as in chlist header attribute): \n");
		for (uint32_t i = 0; i < m_chlist->channelsNum(); i++)
		{
			printf("\t %s, %s \n", m_chlist->channelName(i).c_str(), exrToUserChannelDataTypeName(m_chlist->channelDataTypeName(i)).c_str());	
		}
		printf("compression: ________ %s \n", m_compression->compressionName().c_str());
		printf("pixel aspect ratio: _ %.6f \n", m_pixelAspectRatio->value());
		if (m_hasAttribute_xDensity)
		{
			printf("pixel density: ______ %.6f pixels / square inch (- unverified measurement unit - ) \n", m_xDensity->value());
		}
		printf("OpenEXR version: ____ OpenEXR version %u \n", m_vf->exrVersion());

		printf("\n");
		printf("Pixels values: \n");
		printf("\t * note: pixel 0, 1 means pixel at first (0) row (from top) and second (1) column (from left) \n");
		printf("%s \n", m_pixelData->toStringAsRGBAPixels(false, 5).c_str());
	}

};
