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
// c_name = constant
// g_name = global variable
// m_name = non-static non-public member
// s_name = static variable (static variable, static class instance)
// ----- v ? -----
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

#include "exrFileData.h"

/// as of 2025.04.18, Release configurations do not work properly (some 
/// error occurs when trying to read .exr file). Therefore, x64-Debug and x86-Debug .exe 
/// from dist/ folder is shipped until Release configurations are not fixed.
#define ASSET_INPUT_MODE__DEBUG 0	// wherever used, earlier was:	#if not _DEBUG

#if ASSET_INPUT_MODE__DEBUG
const fs::path g_assetsDir = fs::path("assets") / fs::path("images") / fs::path("test");
const std::string g_debugImage = 
	"test1_RgbaF32CompressNo_and_xDensity_nonColor.exr";
//	"test1_RgbaF32CompressNo_and_xDensity_sRGB.exr";
//	"test2_RgbaF32CompressZip_nonColor.exr";
//	"test2_RgbaF32CompressZip_sRGB.exr";
const fs::path g_debugFilepath = g_assetsDir / fs::path(g_debugImage);

#endif

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
	fs::path filepath = g_debugFilepath;
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
	exrFileData file = exrFileData(filebytes);
	file.exrAnalysisDetailed();
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
