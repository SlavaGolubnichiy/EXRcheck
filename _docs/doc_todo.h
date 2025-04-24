#pragma once

/// todo
// Code:
/// [OK] 1. Implement .exr channel analysis based on .exr data observations:
/// 		Chlist value:
///			[0x10+c ~ ...]		= string, null-terminated channel name
///			[0x20+1 ~ 0x10+e]	= uint, (little-endian) pixel type: 0= UINT, 1= HALF (float 16-bit), 2= FLOAT (32-bit)
///			[0x20+2]			= uchar, pLinear (0 or 1)
///			[0x20+3 ~ 0x20+5]	= char x3, 3 reserved bytes, all should be 0
///			[0x20+9 ~ 0x20+6]	= uint, (little-endian) samplingX
///			[0x20+d ~ 0x20+a]	= uint, (little-endian) samplingY
/// [OK] 2. Analyse versionField: bits 11, 12 and check reserved bits for 0 value.
/// [OK] 3. validate versionField by handcoding valid combinations from "Version field, valid values" at https://openexr.com/en/latest/OpenEXRFileLayout.html#version-field-valid-values
/// [OK] 4. exrData/exrTypes.h -> create class to store data read exr file attributes
///	[OK] 4a. class for "chlist" attribute (instead of using static functions)
///		[OK] 4aa. implement exr file analysis using classes above instead of static functions
///		[OK] 4ab. test the restructured code
///		[OK] 4ac. fix if it has problems
///		[OK] 4b. Remake it using class inheritance
///	[OK] 4b. class for attribute storing value of type "compression"
///	[OK] 4c. class for attribute storing value of type "box2i"
///	[OK] 4d. class for attribute storing value of type "lineOrder"
///	[OK] 4e. class for attribute storing value of type "float"
///	[OK] 4f. class for attribute storing value of type "v2f"
// [  ] 5. Implement .exr file pixel content reading (data section).
/// [OK] 5a. Write short document on how to read .exr file pixel data.
/// [OK] 5b. Implement this in code.
/// [OK] 5c. implement reading of pixelData.scanline[i].channel[i].pixel[i] - todo: put class name here
/// [OK] 5d. implement reading of pixelData.scanline[i].channel[i] - todo: put class name here
/// [OK] 5e. implement reading of pixelData.scanline[i] - todo: put class name here
// [  ] 5f. implement reading of pixelData
///		[OK] 5f.1. implement PixelData constructor that reads pixelData area of .exr file and saves that data
///		[OK] 5f.2. (if 1. needs): implement necessary method(s) in classes PixelData depends onto.
///		[OK] 5f.3. Implement std::string PixelData::toString(), also implementing and using corresponding toString() methods of classes PixelData depends onto.
///		[OK] 5f.4. Use PixelData class (providing it with pixelDataFirstByteIndex) in your user code.
///		[OK] 5f.5. Refactor code, improve naming, reduce redundancy.
///		[OK] 5f.6. Test the results and debug if needed.
///		[OK] 5f.7. Check if .exr file stores & pixelData retrieves correct pixel channels values.
// [  ] 6. Improve reading of OpenEXR attributes.
//	[  ] 6a. OpenEXR attributes: implement ExrHeaderReader class, which reads OpenEXr file header attributes and stores them, allows accessing OpenEXR standard required
//			header attributes, allows accessing optional attributes (analyzed and found inside .exr file)
//	[  ] 6b. OpenEXR attributes: implement check of attributes' firstByteIndex() and lastByteIndex() -es to search for byte gaps left after analysis 
//			to locate missed an unread attributes.
//	[  ] 6c. (maybe) for OpenEXR attributes, improve algorithm by reading through .exr byte-by-byte and search for null-terminated strings (attributes names), then
//			pass found attributes names strings into AttribX constructor to try reading the attribute assumed to have the name found earlier.
//
// 
// 
// 
// Refactor:
///	[OK] 0.1 organize related functionality and data into exr MagicNumber class.
///		Class is not needed, function is absolutely enough.
/// [OK] 0.2 organize related functionality and data into exr VersionField class.
//	[  ] 0.3 utils.h -> unname u8 into standard byte type and test the Application (maybe use C++20's concept for comfort)
//	[  ] 0.4 Create a common function for creating string of bytestream entry, like entryStr(byteRangeMin, byteRangeMax, std::str info) to incapsulate
//			and not repeat the same code of creating string of "[" + hex(rangeMin) + "; " + hex(rangeMax) + "] " + info; or smth like this.
//	[  ] 0.5 Put .exr file magicNumber-, versionField-, attribute, etc...-reading code into exrAnalyzer class.
//  
// 
// Docs:
//	[  ] 1. Docs: complete doc-comment about tiled, etc... exr files at marker EXRTILED-1 from https://openexr.com/en/latest/OpenEXRFileLayout.html#header-attributes-all-files
//	[  ] 2. Docs: complete doc-comment about Chlist attrib. type structure at marker CHLIST-1
///	[OK] 3. Organize docs with as little repeats as possible.
//
// Bugs:
///	[FIXED] 1.	toStringHex() when put to utils.h, Build fails with error "toStringHex() multiple definitions"
//
