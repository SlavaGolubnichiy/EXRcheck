/// !note: this program can read only single-part EXR files.
/*
	USER MANUAL: (Release build)
	1. Find .exe of this program in File Explorer of your OS.
	2. Find .exr (your OpenEXR image) for .exe to analyse.
		Note: program works only with single-part RGBA 32-bit float per channel images.
	3. Drag & drop your OpenEXR image over the program's .exe file inside File Explorer.
	4. Look at the console window to check the analysis results (program does not lock the image file).
	5. Press any button or close the window to quit the program.

*/

/*
	Programmer's tip: search the code, linked by document tag, 
		1) select tag text -> 
		2) in Visual Studio: use Ctrl+Shift+F3 (search in files).
*/
/// see the _docs/doc_OpenEXR.txt
/// see the _docs/doc_todo.txt

// naming convention
// m_name = non-static private member
// s_name = static variable
// g_name = gloabal variable
// ??? ------v 
// c_name = constant
// s_name = struct
// t_name = template argument

// Visual Studio: _DEBUG	= 1 when Config= Release, = 0 when Config= Debug.
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
//#include "exrData/exrTypesExperimental.h"		// experimental

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
	int32_t exr_magicNum = exr::magicNumber(filebytes);
	printf("[03~00] Magic number = 0x%8.8X = %i ", exr_magicNum, exr_magicNum);
	if (exr_magicNum == exr2::consta::magicNumber)		printf("(valid EXR file). \n");
	else	throw std::runtime_error("ERROR: invalid EXR file. Further analysis suspended.");
	/// OpenEXR version field (version of OpenEXR standard + other info)
	VersionField vf = VersionField(filebytes);
	vf.printData();
	printf("\n");

	printf("-------- Header Attributes -------- \n");
	/// todo
	// 1. implement ExrHeaderReader class, which reads OpenEXr file header attributes and stores them, allows accessing OpenEXR standard required
	//		header attributes, allows accessing optional attributes (analyzed and found inside .exr file)
	// 2. implement check of attributes' firstByteIndex() and lastByteIndex() -es to search for byte gaps left after analysis 
	//		to locate missed an unread attributes.
	// 3. (maybe) improve algorithm by reading through .exr byte-by-byte and search for null-terminated strings (attributes names), then
	//		pass found attributes names strings into AttribX constructor to try reading the attribute assumed to have the name found earlier.
	/// OpenEXR header attributes
	// brief example on unique_ptr: 
	// {
	//	std::unique_ptr<MyObject> pointer;
	//	pointer = std::make_unique<MyObject>(/* MyObject constructor parameters */);
	//	pointer->doSomething();		// use pointer as usual
	//	// no need to "delete" manually - it is automatically cleaned-up here
	// }
	//
	std::unique_ptr<exrTypes::AttribChlist> chlist;				// does not need: delete myPtr;
	std::unique_ptr<exrTypes::AttribCompression> compression;
	std::unique_ptr<exrTypes::AttribBox2i> dataWindow;
	std::unique_ptr<exrTypes::AttribBox2i> displayWindow;
	std::unique_ptr<exrTypes::AttribLineorder> lineOrder;
	std::unique_ptr<exrTypes::AttribFloat32> pixelAspectRatio;
	std::unique_ptr<exrTypes::AttribV2f> screenWindowCenter;
	std::unique_ptr<exrTypes::AttribFloat32> screenWindowWidth;
	std::unique_ptr<exrTypes::AttribFloat32> xDensity;
	bool isNoCompression = true;

	chlist = std::make_unique<exrTypes::AttribChlist>(exr::consta::StdAttribName::channels, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", chlist->toString().c_str());
	compression = std::make_unique<exrTypes::AttribCompression>(exr::consta::StdAttribName::compression, filebytes, vf.bit10_HasLongNames());		// must read attribute => no try-catch
	printf("%s \n", compression->toString().c_str());
	dataWindow = std::make_unique<exrTypes::AttribBox2i>(exr::consta::StdAttribName::dataWindow, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", dataWindow->toString().c_str());
	printCaughtException(
		displayWindow = std::make_unique<exrTypes::AttribBox2i>(exr::consta::StdAttribName::displayWindow, filebytes, vf.bit10_HasLongNames());		// may not read attribute => enclosed in try-catch
		printf("%s \n", displayWindow->toString().c_str());
	);
	//printCaughtException(
	lineOrder = std::make_unique<exrTypes::AttribLineorder>(exr::consta::StdAttribName::lineOrder, filebytes, vf.bit10_HasLongNames());			// must read attribute
	printf("%s \n", lineOrder->toString().c_str());
	//);
	printCaughtException(
		pixelAspectRatio = std::make_unique<exrTypes::AttribFloat32>(exr::consta::StdAttribName::pixelAspectRatio, filebytes, vf.bit10_HasLongNames());
		printf("%s \n", pixelAspectRatio->toString().c_str());
	);
	printCaughtException(
		screenWindowCenter = std::make_unique<exrTypes::AttribV2f>(exr::consta::StdAttribName::screenWindowCenter, filebytes, vf.bit10_HasLongNames());
		printf("%s \n", screenWindowCenter->toString().c_str());
	);
	screenWindowWidth = std::make_unique<exrTypes::AttribFloat32>(exr::consta::StdAttribName::screenWindowWidth, filebytes, vf.bit10_HasLongNames());		// must read attribute for code below => no try-catch
	printf("%s \n", screenWindowWidth->toString().c_str());
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
	else						exrHeaderFinalNullIndex = screenWindowWidth->value_lastByteIndex()+1;

	// if versionField.bit12==1 or versionField.bit11==1	=> then => attribute (with name=) "chunkCount", type="int" is required
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
	if (compression->value() != exr2::consta::compression::NO)
	{
		printf("\n\n WARNING: [0x%2.2X ~ 0x%2.2X] compression is not NO_COMPRESSION. OffsetTable & pixelData will not be read. Analysis stops here, after header section.\n\n", compression->value_firstByteIndex(), compression->value_lastByteIndex());
		isNoCompression = false;
		return;
	}
	uint32_t imageRows = uint32_t(dataWindow->value().yMax() + 1);
	uint32_t imageCols = uint32_t(dataWindow->value().xMax() + 1);

	/// OpenEXR image data section
	/* document tag [OPENEXR-OFFSET-TABLE-01] */
	printf("-------- Offset Table -------- \n");
	std::unique_ptr<exrTypes::OffsetTable> offsetTable;
	printCaughtException(
		offsetTable = std::make_unique<exrTypes::OffsetTable>(filebytes, exrHeaderFinalNullIndex+1, dataWindow->value().yMax(), vf.bit12_IsMultipart(), doesExrHas_chunkCount_Attribute);
	);
	for (uint32_t i = 0; i < offsetTable->length(); i++)
	{
		printf("\t %s \n", offsetTable->toString(i).c_str());
	}
	printf("\n");

	/* document tag [OPENEXR-PIXEL-DATA-01] */
	uint32_t imageChannelsNum = chlist->channelsNum();
	std::vector<std::string> channelsNames = chlist->channelsNames();
	std::unique_ptr<exrPixeldata::PixelData<float>> pixelData;
	printCaughtException(
		pixelData = std::make_unique<exrPixeldata::PixelData<float>>(filebytes, offsetTable->lastByteIndex()+1, imageRows, imageCols, imageChannelsNum, lineOrder->value());
	);
	printf("-------- Pixel Data -------- \n");
	printf("%s", pixelData->toStringAsExrPixeldata(channelsNames).c_str());

	if (pixelData->lastByteIndex() == filebytes.size()-1)
		printf("-------- End of .exr file. --------\n\n");
	else
		printf("-------- End of file is not reached. Your file hmay have more than expected. Check file bytes above.\n\n");

	printf("\n");
	printf("------------------------------------- \n");
	printf("-------- Summary (user-view) -------- \n");
	printf("------------------------------------- \n");
	printf(".exr file info: \n");
	printf("size: _______________ %u x %u \n", imageCols, imageRows);
	printf("channels (ordered as in chlist header attribute): \n");
	for (uint32_t i = 0; i < chlist->channelsNum(); i++)
	{
		printf("\t %s, %s \n", chlist->channelName(i).c_str(), exrToUserChannelDataTypeName(chlist->channelDataTypeName(i)).c_str());	
	}
	printf("compression: ________ %s \n", compression->compressionName().c_str());
	printf("pixel aspect ratio: _ %.6f \n", pixelAspectRatio->value());
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


/// as of 2025.04.18, Release configurations do not work properly (some 
/// error occurs when trying to read .exr file). Therefore, x64-Debug and x86-Debug .exe 
/// from dist/ folder is shipped until Release configurations are not fixed.
#define ASSET_INPUT_MODE__DEBUG 0	// wherever used, earlier was:	#if not _DEBUG

void Application(const int argc, char* argv[])
{
	/*
	printf("Required system information.\n");
	pcinfo::System thisPC = pcinfo::System();
	printf("Endianess: %s \n", thisPC.isBigEndianToString().c_str());
	printf("System test end. ------------------------------------------ \n\n");
	*/

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
	std::string filename = "RgbaF32CodecNo-nonColor_DeviceSRGB-ViewAGX-SqncrSRGB.exr";					// colorspace = nonColor,	Device = sRGB
	//std::string filename = "RgbaF32CodecNo-sRGB_DeviceSRGB-ViewAGX-SqncrSRGB.exr";					// colorspace = sRGB,		Device = sRGB
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

int main(int argc, char* argv[])		// argc = , argv = program .exe path
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
