/// !note: this program can read only single-part EXR files.
/*
	USER MANUAL: (Release build)
	1. Find .exe of this program in File Explorer of your OS.
	2. Find .exr (your OpenEXR image) to analyse.
		Note: program works only with single-part RGBA 32-bit float per channel images (yet)
	3. Drag & drop your OpenEXR image over the program's .exe file inside File Explorer.
	4. Look at the console window to check the analysis results (program does not lock the image file).
	5. Press any button or close the window to quit the program.

*/

/// see the _docs/doc_OpenEXR.txt
/// see the _docs/doc_todo.txt

// naming convention
// m_name = non-static non-public member
// s_name = static variable (static variable, static class instance)
// g_name = global variable
// ??? ------v
// c_name = constant
// t_name = template argument

#include <algorithm>
#include <cstring>
#include <exception>
#include <filesystem>
namespace fs = std::filesystem;
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "ExeParams.h"
#include "exrData/exrConsta.h"
#include "exrData/exrTypes.h"
#include "exrData/MagicNumber.h"
#include "exrData/Pixeldata.h"
#include "exrData/VersionField.h"
#include "pcinfo.h"
#include "types.h"
#include "utils.h"

#define printCaughtException(x) \
try								\
{								\
	x							\
}								\
catch(const std::exception& e)	\
{								\
	printf("%s", e.what());		\
}

std::string exrToUserChannelDataTypeName(const std::string exrStandardChannelDataTypeName)
{
	if (exrStandardChannelDataTypeName == exr2::consta::channel::channelDataTypeName(exr2::consta::channel::datatype::UINT))		return "32-bit Unsigned Integer";
	else if (exrStandardChannelDataTypeName == exr2::consta::channel::channelDataTypeName(exr2::consta::channel::datatype::HALF))	return "16-bit Floating point";
	else if (exrStandardChannelDataTypeName == exr2::consta::channel::channelDataTypeName(exr2::consta::channel::datatype::FLOAT))	return "32-bit Floating point";
	else return "<unknown-type. Contact developer>";
}



void printAnalysisSummary
(
	const VersionField& vf,
	const uint32_t imageRows, 
	const uint32_t imageCols, 
	const std::unique_ptr<exrTypes::AttribChlist>& s_chlist, 
	const std::unique_ptr<exrTypes::AttribCompression>& s_compression, 
	const std::unique_ptr<exrTypes::AttribFloat32>& s_pixelAspectRatio,
	const std::unique_ptr<exrPixeldata::PixelData<float>>& pixelData,
	const bool hasAttribute_xDensity,
	const std::unique_ptr<exrTypes::AttribFloat32>& xDensity
)
{
	printf("\n");
	printf("------------------------------------- \n");
	printf("-------- Summary (user-view) -------- \n");
	printf("------------------------------------- \n");
	printf(".exr file info: \n");
	printf("size: _______________ %u x %u \n", imageCols, imageRows);
	printf("channels (ordered as in chlist header attribute): \n");
	for (uint32_t i = 0; i < s_chlist->channelsNum(); i++)
	{
		printf("\t %s, %s \n", s_chlist->channelName(i).c_str(), exrToUserChannelDataTypeName(s_chlist->channelDataTypeName(i)).c_str());	
	}
	printf("compression: ________ %s \n", s_compression->compressionName().c_str());
	printf("pixel aspect ratio: _ %.6f \n", s_pixelAspectRatio->value());
	if (hasAttribute_xDensity)
	{
		printf("pixel density: ______ %.6f pixels / square inch (- unverified measurement unit - ) \n", xDensity->value());
	}
	printf("OpenEXR version: ____ OpenEXR version %u \n", vf.exrVersion());

	printf("\n");
	printf("Pixels values: \n");
	printf("\t * note: pixel 0, 1 means pixel at first (0) row (from top) and second (1) column (from left) \n");
	printf("%s \n", pixelData->toStringAsRGBAPixels(false, 5).c_str());
}

/// <summary>
///		Read file byte-by-byte, verify its a valid .exr file and, if so, analyse it
///		and print the analysis results to the console.
/// </summary>
/// <param name="filename"> Vector of bytes retrieved from file </param>
/// <returns> void </returns>
void exrAnalysisDetailed(const std::vector<ui8>& filebytes)
{
	printf("-------- Magic number & Version field -------- \n");
	/// OpenEXR magicNumber (.exr file validator)
	int32_t exr_magicNum = exr::c_magicNumber(filebytes);
	printf("[03~00] Magic number = 0x%8.8X = %i ", exr_magicNum, exr_magicNum);
	if (exr_magicNum == exr2::consta::c_magicNumber)		printf("(valid EXR file). \n");
	else	throw std::runtime_error("ERROR: invalid EXR file. Further analysis suspended.");
	/// OpenEXR version field (version of OpenEXR standard + other info)
	VersionField vf = VersionField(filebytes);
	vf.printData();
	printf("\n");

	printf("-------- Header Attributes -------- \n");
	/// OpenEXR header attributes
	// brief example on unique_ptr: 
	// {
	//	std::unique_ptr<MyObject> pointer;
	//	pointer = std::make_unique<MyObject>(/* MyObject constructor parameters */);
	//	pointer->doSomething();		// use pointer as usual
	//	// no need to "delete" manually - it is automatically cleaned-up here
	// }
	std::unique_ptr<exrTypes::AttribChlist> s_chlist;				// does automatically: delete myPtr;
	std::unique_ptr<exrTypes::AttribCompression> s_compression;
	std::unique_ptr<exrTypes::AttribBox2i> s_dataWindow;
	std::unique_ptr<exrTypes::AttribBox2i> s_displayWindow;
	std::unique_ptr<exrTypes::AttribLineorder> s_lineOrder;
	std::unique_ptr<exrTypes::AttribFloat32> s_pixelAspectRatio;
	std::unique_ptr<exrTypes::AttribV2f> s_screenWindowCenter;
	std::unique_ptr<exrTypes::AttribFloat32> s_screenWindowWidth;
	std::unique_ptr<exrTypes::AttribFloat32> xDensity;
	bool isNoCompression = true;

	s_chlist = std::make_unique<exrTypes::AttribChlist>(exr::consta::StdAttribName::s_channels, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", s_chlist->toString().c_str());
	s_compression = std::make_unique<exrTypes::AttribCompression>(exr::consta::StdAttribName::s_compression, filebytes, vf.bit10_HasLongNames());		// must read attribute => no try-catch
	printf("%s \n", s_compression->toString().c_str());
	s_dataWindow = std::make_unique<exrTypes::AttribBox2i>(exr::consta::StdAttribName::s_dataWindow, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", s_dataWindow->toString().c_str());
	printCaughtException(
		s_displayWindow = std::make_unique<exrTypes::AttribBox2i>(exr::consta::StdAttribName::s_displayWindow, filebytes, vf.bit10_HasLongNames());		// may not read attribute => enclosed in try-catch
		printf("%s \n", s_displayWindow->toString().c_str());
	);
	//printCaughtException(
	s_lineOrder = std::make_unique<exrTypes::AttribLineorder>(exr::consta::StdAttribName::s_lineOrder, filebytes, vf.bit10_HasLongNames());			// must read attribute
	printf("%s \n", s_lineOrder->toString().c_str());
	//);
	printCaughtException(
		s_pixelAspectRatio = std::make_unique<exrTypes::AttribFloat32>(exr::consta::StdAttribName::s_pixelAspectRatio, filebytes, vf.bit10_HasLongNames());
		printf("%s \n", s_pixelAspectRatio->toString().c_str());
	);
	printCaughtException(
		s_screenWindowCenter = std::make_unique<exrTypes::AttribV2f>(exr::consta::StdAttribName::s_screenWindowCenter, filebytes, vf.bit10_HasLongNames());
		printf("%s \n", s_screenWindowCenter->toString().c_str());
	);
	s_screenWindowWidth = std::make_unique<exrTypes::AttribFloat32>(exr::consta::StdAttribName::s_screenWindowWidth, filebytes, vf.bit10_HasLongNames());		// must read attribute for code below => no try-catch
	printf("%s \n", s_screenWindowWidth->toString().c_str());
	// .exr custom header attribute
	uint32_t exrHeaderFinalNullIndex = 0;
	bool hasAttribute_xDensity = false;
	try
	{
		xDensity = std::make_unique<exrTypes::AttribFloat32>("xDensity", filebytes, vf.bit10_HasLongNames());
		printf("%s \n", xDensity->toString().c_str());
		hasAttribute_xDensity = true;
	}
	catch(const std::exception e)
	{
		hasAttribute_xDensity = false;
	}
	if (hasAttribute_xDensity)	exrHeaderFinalNullIndex = xDensity->value_lastByteIndex()+1;
	else						exrHeaderFinalNullIndex = s_screenWindowWidth->value_lastByteIndex()+1;

	// if versionField.bit12==1 or versionField.bit11==1	=> then => attribute (name="chunkCount", type="int") must be in .exr
	bool doesExrHas_chunkCount_Attribute = vf.bit12_IsMultipart() and vf.bit11_HasDeepData();
	/// to be implemented ...
	


	if (filebytes[exrHeaderFinalNullIndex] == 0x00)
	{
		printf("[0x%s] byte = 0x%2.2X (end of .exr header section) \n\n", utils::hex(exrHeaderFinalNullIndex, 4).c_str(), 0x00);
	}
	else
	{
		throw std::runtime_error("WARNING: 0x00 byte ending the header section with attributes is not found. Further analysis suspended (impossible).");
	}
	if (s_compression->value() != exr2::consta::s_compression::NO)
	{
		printf("\n\n WARNING: [0x%2.2X ~ 0x%2.2X] compression is not NO_COMPRESSION. OffsetTable & pixelData will not be read. Analysis stops here, after header section.\n\n", s_compression->value_firstByteIndex(), s_compression->value_lastByteIndex());
		isNoCompression = false;
		return;
	}
	uint32_t imageRows = uint32_t(s_dataWindow->value().yMax() + 1);
	uint32_t imageCols = uint32_t(s_dataWindow->value().xMax() + 1);

	/// OpenEXR image data section
	/* document tag [OPENEXR-OFFSET-TABLE-01] */
	printf("-------- Offset Table -------- \n");
	std::unique_ptr<exrTypes::OffsetTable> offsetTable;
	printCaughtException(
		offsetTable = std::make_unique<exrTypes::OffsetTable>(filebytes, exrHeaderFinalNullIndex+1, s_dataWindow->value().yMax(), vf.bit12_IsMultipart(), doesExrHas_chunkCount_Attribute);
	);
	for (uint32_t i = 0; i < offsetTable->length(); i++)
	{
		printf("\t %s \n", offsetTable->toString(i).c_str());
	}
	printf("\n");

	/* document tag [OPENEXR-PIXEL-DATA-01] */
	uint32_t imageChannelsNum = s_chlist->channelsNum();
	std::vector<std::string> channelsNames = s_chlist->channelsNames();
	std::unique_ptr<exrPixeldata::PixelData<float>> pixelData;
	printCaughtException(
		pixelData = std::make_unique<exrPixeldata::PixelData<float>>(filebytes, offsetTable->lastByteIndex()+1, imageRows, imageCols, imageChannelsNum, s_lineOrder->value());
	);
	printf("-------- Pixel Data -------- \n");
	printf("%s", pixelData->toStringAsExrPixeldata(channelsNames).c_str());

	if (pixelData->lastByteIndex() == filebytes.size()-1)
		printf("-------- End of .exr file. --------\n\n");
	else
		printf("-------- End of file is not reached. Your file hmay have more than expected. Check file bytes above.\n\n");

	printAnalysisSummary(vf, imageRows, imageCols, s_chlist, s_compression, s_pixelAspectRatio, pixelData, hasAttribute_xDensity, xDensity);

}




/// as of 2025.04.18, Release configurations do not work properly (some 
/// error occurs when trying to read .exr file). Therefore, x64-Debug and x86-Debug .exe 
/// from dist/ folder is shipped until Release configurations are not fixed.
#define ASSET_INPUT_MODE__DEBUG 1	// wherever used, earlier was:	#if not _DEBUG

void Application(const int argc, char* argv[])
{
	#if not ASSET_INPUT_MODE__DEBUG
	std::string userTip_specifyExrFilepath = "The easiest way to specify .exr file path is to \'drag-and-drop\' .exr file over .exe of this program.";
	std::unique_ptr<exe::ExeParams> app;
	try
	{
		app = std::make_unique<exe::ExeParams>(argc, (const char**)argv);
	}
	catch(std::exception e)
	{
		throw std::runtime_error("ERROR: " + std::string(e.what()) + ". Existing .exr file path is expected.\n");
	}
	if (app->paramsNum() < 2)
	{
		throw std::runtime_error("ERROR: Program parameter [1] is not provided. Existing .exr file path is expected.\n" + userTip_specifyExrFilepath + "\n");
	}
	fs::path filepath = app->param(1);
	if (not fs::exists(filepath))
	{
		throw std::runtime_error("ERROR: Program parameter [1] is invalid file path. Existing .exr file path is expected.\n" + userTip_specifyExrFilepath + "\n");
	}
	#else
	//std::string filename = "RgbaF32CodecNo-nonColor_DeviceSRGB-ViewAGX-SqncrSRGB.exr";					// colorspace = nonColor,	Device = sRGB
	std::string filename = "RgbaF32CodecNo-sRGB_DeviceSRGB-ViewAGX-SqncrSRGB.exr";					// colorspace = sRGB,		Device = sRGB
	//std::string filename = "exr_no-xDensity-attrib_RgbaF32-nonColor_resaved-using-GIMP3.0.0-RC3.exr";	// colorspace = nonColor
	//std::string filename = "exr_no-xDensity-attrib_RgbaF32-sRGB_resaved-using-GIMP3.0.0-RC3.exr";		// colorspace = sRGB
	fs::path filepath = fs::path("assets") / fs::path("images") / fs::path("test") / fs::path(filename);
	#endif

	printf("file: %s\n\n", filepath.generic_string().c_str());
	std::vector<ui8> filebytes = utils::file::getFilebytes_v5_CppOnly(filepath.string().c_str());
	printf("EXR data (char view) -------------------------------------- \n");
	utils::print::asChar(filebytes);
	printf("\nEOF ------------------------------------------------------- \n\n");
	printf("EXR data (hex view) --------------------------------------- \n");
	utils::print::asBytes(filebytes);
	printf("\nEOF ------------------------------------------------------- \n\n");
	printf("OpenEXR file analysis result.\n");
	std::size_t filesizeB = filebytes.size();
	printf("File size = %u Bytes = %.6f KB = %.6f MB \n\n", uint32_t(filesizeB), float(filesizeB)/1024, float(filesizeB)/1024/1024);
	exrAnalysisDetailed(filebytes);
}

int main(int argc, char* argv[])		// argc = , argv[0] = program .exe path, argv[1...] = input .exe parameters
{
	try
	{
		Application(argc, argv);
	}
	catch(const std::exception e)
	{
		printf("FATAL ERROR: %s \n", e.what());
		utils::waitForUser();
		return -1;
	}

	printf
	(
		"--------------------\n"
		"End of analysis.\n"
		"Program closes after you press any button.\n\n"
	);
	utils::waitForUser();
	return 0;
}
