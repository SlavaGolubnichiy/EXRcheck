#pragma once

/*
	Programmer's tip: search the code, linked by document tag, 
		1) select tag text -> 
		2) in Visual Studio: use Ctrl+Shift+F3 (search in files).
*/

// OpenEXR documentation used for implementing the program.
/*
	Header(s).
		sources:
			https://openexr.com/en/latest/OpenEXRFileLayout.html#component-three-header
			https://openexr.com/en/latest/OpenEXRFileLayout.html#header-attributes-all-files
		date: 2025.03.12
		code:

	Each header is a sequence of attributes ended by a null byte.

	For a single-part file, its header component holds a single header (same file structure as OpenEXR 1.7).
	Multi-part bit (bit 12) must = 0, and the single null byte signaling the end of the headers must be omitted.
	This is the same for single-part deep data files.

	For a multi-part file (new in OpenEXR 2.0), its header component holds a set of headers, with separate header for each part
	and a null byte signalling the end of the header component.
	
	Header structure.
	As said, a header is a sequence of attributes ended by a null byte.
	
	----

	Version Field structure.
		source: https://openexr.com/en/latest/OpenEXRFileLayout.html#version-field
		date: 2025.03.11
		code: document tag [VERSIONFIELD-01] [VERSIOFIELD-BIT-08] [VERSIOFIELD-BIT-13-31]

	The version field, of type int, is the four-byte group following the magic number, 
	and it is treated as two separate bit fields. 
	------------------------------------------------------------------------------------------
	bitNum		|	description and notes
	------------------------------------------------------------------------------------------
	bits 0~7	|	The 8 least significant bits, they contain the file format version number. 
				|	The current OpenEXR version number is version 2.
	------------------------------------------------------------------------------------------
	The 24 most significant bits, these are treated as a set of boolean flags.
	------------------------------------------------------------------------------------------
	bit 08		| unused, must be = 0 (in current version of EXR). ------------ ??? test it [VERSIOFIELD-BIT-08]
	bit 09		| single-part file in tiled format indicator.
				|	= 1 = single-part image & pixels are stored as tiles; bits 11 and 12 must be = 0)
				|	= 0 = if bit11==0 and bit12==0, then the data is stored as regular single part scan-line file.
	bit 10		| indicates file contains "long names".
				|	= 1 = the max. length is 255 bytes.
				|	= 0 = the max. length of attribute name, attribute type names and channel names is 21 bytes.
	bit 11		| indicates file contains "non-image" (deep data) parts.
				|	= 1 = there is at least 1 part not a regular scan line image or regular tiled image (i.e. in deep format).
				|	= 0 = all parts are entirely single or multiple scan line or tiled images.
	bit 12		| multi-part file indicator
				|	= 1 = file contains not exactly 1 part; the "end of header" byte must be included at end of each header part, and * the part number fields must be added to the chunks.
				|	= 0 = file is not multi-part; the "end of header" and part number fields must be omitted.
	bits 13~31	| remaining 19 bits must be 0 (in current version of EXR). ------- ??? [VERSIOFIELD-BIT-13-31]

	Note: 
		The version field bits define what capabilities must be available in the software so it can handle the file, 
		rather than the exact format of the file. While the 9 and 11 bit settings must agree with the type attributes of all parts, 
		in OpenEXR 2.0 the data format of each type is definitively set by the type attribute in that part’s header alone.

	----

	Valid version fields.
		source: https://openexr.com/en/latest/OpenEXRFileLayout.html#version-field-valid-values
		date: 2025.03.12
		code: document tag [VERSIONFIELD-02]
	----------------------------------------------------------------------------------------------------------------------
	Description															| Compatibile with			| bits	9, 10, 11, 12
	----------------------------------------------------------------------------------------------------------------------
	Single-part scan line. One normal scan line image.					| All versions of OpenEXR.	|		0,  0,  0,  0
	Single-part tile. One normal tiled image.							| All versions of OpenEXR.	|		1,  ~,  0,  0
	Multi-part (new in 2.0). Multiple images (scan line and/or tiled).	| OpenEXR 2.0.				|		0,  ~,  0,  1
	Single-part deep data (new in 2.0). Deep tile or deep scan line.	| OpenEXR 2.0.				|		0,  ~,  1,  0
	Multi-part deep data (new in 2.0).									| OpenEXR 2.0.				|		0,  ~,  1,  1
	----------------------------------------------------------------------------------------------------------------------
	Any combination of: tiles, scan lines, deep tiles, deep scan lines.

	----

	Attribute layout. 
		source: https://openexr.com/en/latest/OpenEXRFileLayout.html#attribute-layout
		date: 2025.03.23
		code: document tag [ATTRIBUTE-01]

	Attribute layout:
		attribute name, 
		attribute type, 
		attribute size, 
		attribute value.
	Attribute name - null-terminated string. Size: at least 1 byte, at most (if bit10 =0) 31 bytes or (if bit10 =1) 255 bytes. 
	Attribute type - null-terminated string. Size: at least 1 byte, at most (if bit10 =0) 31 bytes or (if bit10 =1) 255 bytes.
	Attribute size - type: int. Indicates size of attribute value (in bytes).
	Attribute value - layout of value depends on the attribute type. 
		OpenEXR predefines several types. Software can define and store additional attribute types.

	Header Attributes (All Files).
		source: https://openexr.com/en/latest/OpenEXRFileLayout.html#header-attributes-all-files
		date: 2025.03.22
		code: document tag [STD-ATTRIBUTE-01]

	The header of every OpenEXR file must contain at least the following attributes:
	---------------------------------------
	attribute name		|	attribute type
	---------------------------------------
	channels			|	chlist
	compression			|	compression
	dataWindow			|	box2i
	displayWindow		|	box2i
	lineOrder			|	lineOrder
	pixelAspectRatio	|	float
	screenWindowCenter	|	v2f
	screenWindowWidth	|	float
	---------------------------------------
	* For descriptions of what these attributes are for, see Technical Introduction to OpenEXR.
	
	----

	OpenEXR predefined attribute types.
		source: https://openexr.com/en/latest/OpenEXRFileLayout.html#predefined-attribute-types
		date: 2025.03.22
		code: document tag [EXR-ATTRIB-TYPES-01]

	Standard C types sizes.
	name					|	size (bytes)
	-------------------------------------------
	unsigned int, uint		|	4 bytes (32 bits)
	signed int, int			|	4 bytes (32 bits)
	unsigned char, uchar	|	1 byte	(8 bits)
	signed char, char		|	1 byte	(8 bits)
	float					|	4 bytes	(32 bits)
	double					|	8 bytes (64 bits)

	OpenEXR predefined attribute types.
	Type			| Structure
	---------------------------------------------------------------------------------------------
	box2i			|	four int's: xMin, yMin, xMax, yMax
	----------------|----------------------------------------------------------------------------
	box2f			|	four float's: xMin, yMin, xMax, yMax
	----------------|----------------------------------------------------------------------------
	chlist			|	sequence of channels, finalized with 0x00 byte.
					|----------------------------------------------------------------------------
					|	name	|	string 1 to 255 bytes long, terminated with '\0'
					|-----------|----------------------------------------------------------------
					|	pixel	|	int, possible values:
					|	type	|	0= UINT, 1= HALF (16-bit float), 2= FLOAT (32-bit)
					|-----------|----------------------------------------------------------------
					|	pLinear |	unsigned char, possible values are 0 and 1
					|-----------|----------------------------------------------------------------
					| reserved	|	three char's (3 bytes), should be zero
					| ----------|----------------------------------------------------------------
					| xSampling |	int
					|-----------|----------------------------------------------------------------
					| ySampling |	int
					|-----------|----------------------------------------------------------------
					|	0x00	|	unsigned char (1 byte), 0x00 finalizing chlist value
	----------------|----------------------------------------------------------------------------
	chromaticities	|	eight float's:	redX, redY, greenX, greenY, blueX, blueY, whiteX, whiteY
	----------------|----------------------------------------------------------------------------
	compression		|	unsigned char, possible values:
					|		0 = NO_COMPRESSION,		1 = RLE_COMPRESSION
					|		2 = ZIPS_COMPRESSION,	3 = ZIP_COMPRESSION
					|		4 = PIZ_COMPRESSION,	5 = PXR24_COMPRESSION
					|		6 = B44_COMPRESSION,	7 = B44A_COMPRESSION
					|		8 = DWAA_COMPRESSION,	9 = DWAB_COMPRESSION
	----------------|----------------------------------------------------------------------------
	double			|	double (8 bytes = 64 bits)
	----------------|----------------------------------------------------------------------------
	envmap			|	unsigned char, possible values:		0 = ENVMAP_LATLONG, 1 = ENVMAP_CUBE
	----------------|----------------------------------------------------------------------------
	float			|	float (4 bytes = 32 bits)
	----------------|----------------------------------------------------------------------------
	int				|	int (4 bytes = 32 bits)
	----------------|----------------------------------------------------------------------------
	keycode			|	seven int's (7 * 4 bytes):	filmMfcCode, filmType, prefix, count, 
					|								perfOffset, perfsPerFrame, perfsPerCount
	----------------|----------------------------------------------------------------------------
	lineOrder		|	unsigned char, possible values are:	
					|		0= INCREASING_Y, 1= DECREASING_Y, 2= RANDOM_Y
	----------------|----------------------------------------------------------------------------
	m33f			|	9 float's (9 * 4 bytes)
	----------------|----------------------------------------------------------------------------
	m44f			|	16 float's (16 * 4 bytes)
	----------------|----------------------------------------------------------------------------
	preview			|	Two unsigned int's: width and height.
					|	Followed by (4 * width * height) unsigned char's of pixel data. 
					|	Scan lines are stored top to bottom; 
					|	Within a scan line pixels are stored from left to right. 
					|	A pixel consists of four unsigned char's: R, G, B, A.
	----------------|----------------------------------------------------------------------------
	rational		|	An int (4 bytes), followed by an unsigned int (4 bytes).
	----------------|----------------------------------------------------------------------------
	string			|	String length, of type int (4 bytes), followed by a sequence of char‘s.
	----------------|----------------------------------------------------------------------------
	stringvector	|	Sequence of zero or more text strings. 
					|	Each string is: string length of type int, followed by sequence of chars. 
					|	Number of strings can be inferred from total attribute size (see 
					|	"Attribute Layout" section).
	----------------|----------------------------------------------------------------------------
	tiledesc		|	Two unsigned int's: xSize, ySize, followed by 
					|		mode (unsigned char), where (mode = levelMode + roundingMode * 16;)			/// what is levelMode? and roundingMode?
					|	Possible values for levelMode:	0= ONE_LEVEL
					|									1= MIPMAP_LEVELS
					|									2= RIPMAP_LEVELS
					|	Possible values for roundingMode:	0= ROUND_DOWN
					|										1= ROUND_UP
	----------------|----------------------------------------------------------------------------
	timecode		|	Two unsigned int's (2 * 4 bytes): timeAndFlags, userData.
	----------------|----------------------------------------------------------------------------
	v2i				|	Two int's		(2* 4 bytes)
	----------------|----------------------------------------------------------------------------
	v2f				|	Two float's		(2* 4 bytes)
	----------------|----------------------------------------------------------------------------
	v3i				|	Three int's		(3* 4 bytes)
	----------------|----------------------------------------------------------------------------
	v3f				|	Three float's	(3* 4 bytes)
	---------------------------------------------------------------------------------------------



	===================================== document tag [EXR-TILED-1] =======================================



	OpenEXR data structure.
		sources: https://openexr.com/en/latest/TechnicalIntroduction.html#file-structure
		date: 2025.03.13
		code: 

	OpenEXR is made up of: the header and the pixels.
	The header is a list of attributes that describe the pixels. An attribute is a named data item of an arbitrary type. 
	To ensure that OpenEXR files written by one program can be read by other programs, certain required attributes must 
	be present in all OpenEXR file headers.
	----------------------------------------------------------------------------------------------------------------------
	ATTRIBUTE NAME		|	DESCRIPTION
	----------------------------------------------------------------------------------------------------------------------
	displayWindow,		|	The image’s display and data window. 
	dataWindow			|
	--------------------|-------------------------------------------------------------------------------------------------
	pixelAspectRatio	|	Width divided by height of a pixel when the image is displayed with the correct aspect ratio. 
						|	A pixel’s width (height) is the distance between the centers of two horizontally (vertically) 
						|	adjacent pixels on the display.
	--------------------|-------------------------------------------------------------------------------------------------
	channels			|	Description of the image channels stored in the file.
	--------------------|-------------------------------------------------------------------------------------------------
	compression			|	Specifies the compression method applied to the pixel data of all channels in the file.
	--------------------|-------------------------------------------------------------------------------------------------
	lineOrder			|	Specifies in what order the scan lines in the file are stored in the file (increasing Y, 
						|	decreasing Y, or, for tiled images, also random Y).
	--------------------|-------------------------------------------------------------------------------------------------
	screenWindowWidth,	|	Describe the perspective projection that produced the image. Programs that deal with 
	screenWindowCenter 	|	images as purely two-dimensional objects may not be able so generate a description of 
						|	a perspective projection. Those programs should set screenWindowWidth to 1, 
						|	and screenWindowCenter to (0, 0).
	--------------------|-------------------------------------------------------------------------------------------------
	tiles				|	This attribute is required only for tiled files. It specifies the size of the tiles, 
						|	and the file’s level mode.
	----------------------------------------------------------------------------------------------------------------------
	*	In addition to required attributes, a program may place any number of additional attributes in the file’s header. 
		Often it is necessary to annotate images with additional data, for example color timing information, process tracking 
		data, or camera position and view direction. Those data can be packaged as extra attributes in the image file’s header.
	**	For multi-part & deep data headers, there are special requirements. For more info, see 
		https://openexr.com/en/latest/TechnicalIntroduction.html#file-structure

	----

	Pixels.
		source: https://openexr.com/en/latest/TechnicalIntroduction.html#file-structure
		date: 2025.03.13
	
	A chunk is a set of pixel data of particular format or data type (scanlines (or groups of scanlines), tiles and deep data). 
	The structure of a chunk is defined by the type of pixel data stored in it.

	For more info, see
		document tag [OPENEXR-OFFSET-TABLE-01] and
		document tag [OPENEXR-PIXEL-DATA-01]

	----



	============================================ document tag [CHLIST-1] ===================================



*/
/// 4. EXR file offset table
/// 	source: https://openexr.com/en/latest/OpenEXRFileLayout.html#component-four-offset-tables
///		date: 2025.04.02
///		code: document tag [OPENEXR-OFFSET-TABLE-01]
/*
	4. Offset Tables.
		
	Offset table allows random access to pixel data chunks. Offset table is a sequence of offsets, 
	with one offset per chunk. Each offset (of type unsigned long) indicates the distance, in bytes, 
	between the start of the file and the start of the chunk.
	Chunks can be of any of the four data types.

	Offset Table Size.
		The number of entries in an offset table is defined in one of two ways:
		If the multipart (12) bit (versionField.bit12) = 0 and the chunkCount is not present, 
			the number of entries in the chunk table is computed using the dataWindow and tileDesc attributes and the compression format.
		If the multipart (12) bit (versionField.bit12) = 1, 
			the header must contain a chunkCount attribute (which indicates the size of the table and the number of chunks).
	
		Scan Lines.
		For scan line blocks, the line offset table is a sequence of scan line offsets, with one offset per scan line block. 
		In the table, scan line offsets are ordered according to increasing scan line y coordinates.

		Tiles.
		For tiles, the offset table is a sequence of tile offsets, one offset per tile. 
		In the table, scan line offsets are sorted the same way as tiles in INCREASING_Y order.

		Multi-Part.
		For multi-part files, each part defined in the header component has a corresponding chunk offset table.
*/
/// 5. EXR file pixel data
///		source: https://openexr.com/en/latest/OpenEXRFileLayout.html#component-five-pixel-data
///		date: 2025.04.03
///		code: document tag [OPENEXR-PIXEL-DATA-01]
/*
	5. Pixel data
		
	Chunk Layout
		A "chunk" is a general term for a pixel data block. 
		The scan line and tile images have the same format that they did in OpenEXR 1.7. 
		OpenEXR 2.0 introduces two new types (deep scan line and deep tile).
		
		The layout of each chunk is:
		name			| type, size
		-----------------------------------------------
		part number		| unsigned long int (8 bytes)		// present only if multipart bit = 1 (versionfield.bit12)
		----------------|------------------------------
		chunk data		| ...
		
		If multi-part bit == 1 (versionfield.bit12) => the [part number] is present, indicating 
			which part this chunk belongs to (0 means first header & first offset table).
		If multipart bit == 0 => the [part number] is omitted (for saving space & backwards compatibility
			for the software that does not support multi-part files).

	The [chunk data] depends on the type attribute, but (other than the part number) has 
		the same structure as a single-part file of the same format:
		part type	| type attribute						| notes
		---------------------------------------------------------------------------------------------------------
		scan line	| indicated by							| Each chunk stores a scan line block, 
					| "type" attribute of "scanlineimage"	| with the minimum y coordinate of the scan line(s) 
					|										| within the chunk. 
					|										| See "Regular scan line image block layout".
		------------|---------------------------------------|----------------------------------------------------
		tiled		| indicated by							| See "Regular image tile layout".
					| "type" attribute of "tiledimage"		|
		------------|---------------------------------------|----------------------------------------------------
		deep scan	| indicated by							| See "Deep scan line layout".
		line		| "type" attribute of "deepscanline"	|
		------------|---------------------------------------|----------------------------------------------------
		deep tile	| indicated by							| See "Deep tiled layout".
					| "type" attribute of "deeptile"		|
		---------------------------------------------------------------------------------------------------------
		Links:
		Regular scan line image block layout:	https://openexr.com/en/latest/OpenEXRFileLayout.html#regular-scan-line-image-block-layout
		Regular image tile layout:				https://openexr.com/en/latest/OpenEXRFileLayout.html#regular-image-tile-layout
		Deep scan line layout:					https://openexr.com/en/latest/OpenEXRFileLayout.html#deep-scan-line-layout
		Deep tiled layout:						https://openexr.com/en/latest/OpenEXRFileLayout.html#deep-tiled-layout

	Regular scan line image block layout.
		name			| type, size
		---------------------------------
		y coordinate	| int, 4 bytes
		----------------|----------------
		pixel data size | int, 4 bytes
		----------------|----------------
		pixel data		| ...

*/
/// 5a. OpenEXR PixelData's scanlines
///		source: ...
///		date: ...	
///		code: document tag [EXR-SCANLINES-01]
/*
	Within the pixel data, scan lines are stored top to bottom. Each scan line is contiguous, and 
	within a scan line the data for each channel are contiguous. Channels are stored in alphabetical order, 
	according to channel names. 
	Within a channel, pixels are stored left to right.
	...

	<to-be-continued...>

*/
