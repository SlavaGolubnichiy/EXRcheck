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
#include <string>
#include <vector>
#include "exrData/exrConsta.h"
#include "exrData/exrTypes.h"
#include "exrData/MagicNumber.h"
#include "exrData/Pixeldata.h"
#include "exrData/VersionField.h"
#include "pcinfo.h"
#include "types.h"
#include "utils.h"
//#include "exrData/exrTypesExperimental.h"		// experimental

static constexpr bool USE_USER_FLOAT_PRECISION = 0;
#if USE_USER_FLOAT_PRECISION
#include <string>
#include <cctype>
#include <charconv>
#include <limits>

/// <summary>
///		Checks that the string (str) represents uint64_t, following the requirements:
///		- is not allowed.
///		+ is allowed.
///		Fractional part is only allowed if it’s .0, .00, etc.
///		Value must be in range [0, 2^64 - 1] (i.e., up to 18446744073709551615).
///		No extra characters or junk.
/// </summary>
/// <param name="str"> - input str that should represent 64-bit unsigned integer </param>
/// <param name="result"> - result of conversion of (str) to a valid uint64_t </param>
/// <returns></returns>
bool isValidUnsignedInteger(const std::string& str, uint32_t& result)
{
	std::string::size_type i = 0;

	// Allow optional '+'
	if (i < str.size() && str[i] == '+')
	{
		++i;
	}

	// At least one digit must be present
	if (i == str.size() || !std::isdigit(str[i]))
	{
		return false;
	}

	// Extract the integer part
	std::string::size_type int_start = i;
	while (i < str.size() && std::isdigit(str[i]))
	{
		++i;
	}

	std::string int_part = str.substr(int_start, i - int_start);
	std::string frac_part;

	// Optional fractional part
	if (i < str.size() && str[i] == '.')
	{
		++i;
		std::string::size_type frac_start = i;
		while (i < str.size() && std::isdigit(str[i]))
		{
			if (str[i] != '0') return false;  // only '0's allowed in fractional part
			++i;
		}
		frac_part = str.substr(frac_start, i - frac_start);
	}

	// No extra characters allowed
	if (i != str.size()) return false;

	// Parse the integer part using std::from_chars for fast, safe conversion
	result = 0;
	auto [ptr, ec] = std::from_chars(int_part.data(), int_part.data() + int_part.size(), result);
	if (ec != std::errc()) return false;

	return true;
}
#endif



/// <summary>
///		Read file byte-by-byte, verify its a valid .exr file and, if so, analyse it
///		and print the analysis results to the console.
/// </summary>
/// <param name="filename"> Vector of bytes retrieved from file </param>
/// <returns> void </returns>
void exrAnalysisDetailed(const std::vector<ui8>& filebytes, const bool hasAttribute_xDensity = false)
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
	// re-orient attribute classes to read not based on found string of attribute name (first constructor parameter),
	// but based on input attributeFirstByteIndex. (
	// Pros & cons: 
	//		[+] Allows sequential read, without missing attributes
	//		[-] Requires implementation of forward attribute type reading, to define which class to use for reading and storing attribute value.
	///
	/// OpenEXR header attributes
	exrTypes::AttribChlist chlist = exrTypes::AttribChlist(exr::consta::StdAttribName::channels, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", chlist.toString().c_str());
	exrTypes::AttribCompression compression = exrTypes::AttribCompression(exr::consta::StdAttribName::compression, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", compression.toString().c_str());
	exrTypes::AttribBox2i dataWindow = exrTypes::AttribBox2i(exr::consta::StdAttribName::dataWindow, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", dataWindow.toString().c_str());
	exrTypes::AttribBox2i displayWindow = exrTypes::AttribBox2i(exr::consta::StdAttribName::displayWindow, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", displayWindow.toString().c_str());
	exrTypes::AttribLineorder lineOrder = exrTypes::AttribLineorder(exr::consta::StdAttribName::lineOrder, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", lineOrder.toString().c_str());
	exrTypes::AttribFloat32 pixelAspectRatio = exrTypes::AttribFloat32(exr::consta::StdAttribName::pixelAspectRatio, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", pixelAspectRatio.toString().c_str());
	exrTypes::AttribV2f screenWindowCenter = exrTypes::AttribV2f(exr::consta::StdAttribName::screenWindowCenter, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", screenWindowCenter.toString().c_str());
	exrTypes::AttribFloat32 screenWindowWidth = exrTypes::AttribFloat32(exr::consta::StdAttribName::screenWindowWidth, filebytes, vf.bit10_HasLongNames());
	printf("%s \n", screenWindowWidth.toString().c_str());
	// custom .exr header attribute
	exrTypes::AttribFloat32* xDensity = nullptr;
	if (hasAttribute_xDensity)
	{
		xDensity = new exrTypes::AttribFloat32("xDensity", filebytes, vf.bit10_HasLongNames());
		printf("%s \n", xDensity->toString().c_str());
	}
	const bool doesExrHas_chunkCount_Attribute = false;		// attribute with name = "chunkCount", type= "int" (required if versionField.bit12==1 or versionField.bit11==1)
	uint32_t exrHeaderFinalNullIndex = 0;
	if (hasAttribute_xDensity)	exrHeaderFinalNullIndex = xDensity->value_lastByteIndex()+1;
	else						exrHeaderFinalNullIndex = screenWindowWidth.value_lastByteIndex()+1;

	if (filebytes[exrHeaderFinalNullIndex] == 0x00)		printf("[0x%s] byte = 0x%2.2X (end of .exr header section) \n\n", utils::hex(exrHeaderFinalNullIndex, 4).c_str(), 0x00);
	else												throw std::runtime_error("WARNING: 0x00 byte ending the header section with attributes is not found. Further analysis suspended (impossible).");

	/// OpenEXR image data section
	/* document tag [OPENEXR-OFFSET-TABLE-01] */
	printf("-------- Offset Table -------- \n");
	exrTypes::OffsetTable offsetTable = exrTypes::OffsetTable(filebytes, exrHeaderFinalNullIndex+1, dataWindow.value().yMax(), vf.bit12_IsMultipart(), doesExrHas_chunkCount_Attribute);
	for (uint32_t i = 0; i < offsetTable.length(); i++)
	{
		printf("\t %s \n", offsetTable.toString(i).c_str());
	}
	printf("\n");

	/* document tag [OPENEXR-PIXEL-DATA-01] */
	uint32_t imageRows = uint32_t(dataWindow.value().yMax() + 1);
	uint32_t imageCols = uint32_t(dataWindow.value().xMax() + 1);
	uint32_t imageChannelsNum = chlist.channelsNum();
	std::vector<std::string> channelsNames = chlist.channelsNames();
	exrPixeldata::PixelData<float> pixelData = exrPixeldata::PixelData<float>(filebytes, offsetTable.lastByteIndex()+1, imageRows, imageCols, imageChannelsNum, lineOrder.value());
	printf("-------- Pixel Data -------- \n");
	printf("%s", pixelData.toStringAsExrPixeldata(channelsNames).c_str());

	if (pixelData.lastByteIndex() == filebytes.size()-1)	printf("-------- End of .exr file. --------\n\n");
	else													printf("-------- End of file is not reached. Your file hmay have more than expected. Check file bytes above.\n\n");

	printf("Input .exr file pixels (table view): \n");
	printf("%s \n", pixelData.toStringAsRGBAPixels(false, 5).c_str());

	delete xDensity; xDensity = nullptr;	// delete xDensity (heap-allocated)
}

#if not _DEBUG
using argc_t = uint64_t;
using argv_t = char**;
using argvElem_t = char*;

// argc = number of paramters addressed by (argv) array of char*.
// argv = array of char* (char pointers), 
//		where each argv[i] stores pointer to string representing 
//		another program parameter (from running .exe from command-line or elsewhere)
class ExeParams
{
	public:
	ExeParams(const int argc, const char** argv)		
	{
		if (argc < 0)
		{
			throw std::invalid_argument("(argc) is less than 0. Program can not have 0 parameters (first parameter is always .exe path and name)");
		}
		u_argc = argc_t(argc);
		u_argvStrings = std::vector<std::string>(u_argc);
		for (uint64_t i = 0; i < u_argc; i++)
		{
			u_argvStrings[i] = std::string(argv[i]);
		}
		// check that any parameter is not empty string
		for (uint64_t i = 0; i < u_argc; i++)
		{
			if (param(i).empty())
			{
				throw std::runtime_error("parameter [" + std::to_string(i) + "] is empty string");
			}
		}
	}
	argc_t paramsNum() const { return u_argc; }
	std::string param(const argc_t index) const
	{
		if (index < u_argc-1 and u_argc-1 < index)
		{
			throw std::invalid_argument("ExeParams::getArgv(index). (index) value is out of valid range [0; u_argc-1]");
		}
		return u_argvStrings[index];
	}
	std::string pathAndName() const { return u_argvStrings[0]; }
	std::string toString() const
	{
		std::string result = "argc = " + std::to_string(u_argc) + "\n";
		result += "argv[i]: \n";
		for (argc_t i = 0; i < u_argc; i++)
		{
			result += std::to_string(i) + ": " + u_argvStrings[i] + "\n";
		}
		return result;
	}

	private:
	argc_t u_argc = 0;
	std::vector<std::string> u_argvStrings;
};
#endif



int main(int argc, char* argv[])	// argc = , argv = program .exe path
{
	/*
	printf("Required system information.\n");
	pcinfo::System thisPC = pcinfo::System();
	printf("Endianess: %s \n", thisPC.isBigEndianToString().c_str());
	printf("System test end. ------------------------------------------ \n\n");
	*/

	#if not _DEBUG
	ExeParams* exe = nullptr;
	try
	{
		exe = new ExeParams(argc, (const char**)argv);
	}
	catch(std::exception e)
	{
		std::cerr << "ERROR: " << e.what() << ". Existing .exr file path is expected.\n";
		system("pause");
		return 0;
	}
	if (exe->paramsNum() < 2)
	{
		std::cerr << "ERROR: Program parameter [1] is not provided. Existing .exr file path is expected.\n";
		system("pause");
		return 0;
	}
	fs::path filepath = exe->param(1);
	if (not fs::exists(filepath))
	{
		std::cerr << "ERROR: Program parameter [1] is invalid file path. Existing .exr file path is expected.\n";
		system("pause");
		return 0;
	}
	#else
	//std::string filename = "RgbaF32CodecNo-nonColor_DeviceSRGB-ViewAGX-SqncrSRGB.exr";	// exr colorspace = nonColor 	// Device = sRGB
	std::string filename = "RgbaF32CodecNo-sRGB_DeviceSRGB-ViewAGX-SqncrSRGB.exr";			// exr colorspace = sRGB 		// Device = sRGB
	fs::path filepath = fs::path("assets") / fs::path("images") / fs::path("test") / fs::path(filename);
	#endif

	/// Ask user about required floating-point numbers precision showed (unfinished)
	#if USE_USER_FLOAT_PRECISION
	bool isInputValid = false;
	uint32_t floatPrecision = 0;
	/// v1
	/*
	while (isInputValid == false)
	{
		printf("Floating-point numbers precision to show (number of fraction digits): ");
		
		if (scanf_s("%u", &floatPrecision) != 1)
		{
			printf("Incorrect input: unsigned integer is expected. Try again.\n");
			system("pause");
			system("cls");
			char c;
			while ((c = getchar()) != EOF && c != '\n') continue;
			if (c == EOF) return 0;
		}
		else
		{
			isInputValid = true;
		}
	}
	*/
	/// v2
	printf("This is some text.\nThis is some more text.\n");
	// 1. get and validate user input string
	std::string input;
	while(true)
	{
		printf("Floating-point numbers precision to show (number of fraction digits): ");
		std::getline(std::cin, input);
		if (isValidUnsignedInteger(input, floatPrecision))
		{
			break;
		}
		printf("\tIncorrect input: unsigned integer is expected. Try again.\n");
		//system("pause");
		//system("cls");
	}
	printf("floatPrecision is = %u\n", floatPrecision);
	system("pause");
	// 2. convert (input) string to uint32_T floatPrecision value
	// 3. and use floatPrecision to specify the number of fraction digits of floats when printing them to console.
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
	exrAnalysisDetailed(filebytes, true);
	
	//printf("\n\n\n =========== TEST SECTION ==================\n\n\n");
	//printf("\n\n\n =========== END OF SECTION ==================\n\n\n");

	printf
	(
		"--------------------\n"
		"End of analysis.\n"
		"Program closes after you press any button.\n\n"
	);
	system("pause");
	return 0;
}
