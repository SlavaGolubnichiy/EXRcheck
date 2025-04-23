#pragma once
#include <stdexcept>
#include <string>
#include <vector>

#include "types.h"
#include "exrData/exrTypes.h"

namespace exrAttribExperimental
{	
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



	/// basically the same thing as exrData/exrTypes.h::Channel, but has constructor without params for classes below
	// OK
	class Channel
	{
		public:
		uint32_t channel_firstByteIndex = 0;
		uint32_t channel_lastByteIndex = 0;
		uint32_t name_firstByteIndex = 0;
		uint32_t name_lastByteIndex = 0;
		uint32_t pixelType_firstByteIndex = 0;
		uint32_t pixelType_lastByteIndex = 0;
		uint32_t pLinear_firstByteIndex = 0;
		uint32_t pLinear_lastByteIndex = 0;
		uint32_t reserved_firstByteIndex = 0;
		uint32_t reserved_lastByteIndex = 0;
		uint32_t samplingX_firstByteIndex = 0;
		uint32_t samplingX_lastByteIndex = 0;
		uint32_t samplingY_firstByteIndex = 0;
		uint32_t samplingY_lastByteIndex = 0;

		std::string name = "";
		uint32_t pixelType = 0xFFFFFFFF;
		uint8_t pLinear = 0xFF;
		std::vector<uint8_t> reserved = {0xFF, 0xFF, 0xFF};		// OpenEXR: 3 reserved bytes
		int32_t samplingX = 0;
		int32_t samplingY = 0;

		Channel(bool versionFieldBit10, const std::vector<u8>& filebytes, const uint32_t channelFirstByteIndex)
			: 
			channel_firstByteIndex(channelFirstByteIndex),
			channel_lastByteIndex(analyseChannelAndGetLastByteIndex(filebytes, channelFirstByteIndex))
		{
			// find "channels\0chlist\0" byte-substring in filebytes and analyse the following bytes
		}

		/// ~ copy of printChannelInfoAndGetLastByteIndex
		uint32_t analyseChannelAndGetLastByteIndex(const std::vector<u8>& bytes, const uint32_t channelFirstByteIndex)
		{
			// chlist channel name
			name_firstByteIndex = channelFirstByteIndex;
			name = exrTypes::readCString(bytes.cbegin() + name_firstByteIndex, bytes.cend());
			name_lastByteIndex = name_firstByteIndex + name.length();	// no -1 because counting +1 '\0'
			// channel pixel type
			pixelType_firstByteIndex = name_lastByteIndex + 1;
			pixelType_firstByteIndex = pixelType_firstByteIndex + sizeof(pixelType)-1; // int32_t is 4 bytes => offset= 3
			pixelType = exrTypes::read4Bytes((bytes.cbegin()+pixelType_firstByteIndex)._Ptr);
			// channel pLinear
			pLinear_firstByteIndex = pixelType_lastByteIndex + 1;
			pLinear_lastByteIndex = pLinear_firstByteIndex;
			this->pLinear = bytes[pLinear_firstByteIndex];
			printf("\t\t [0x%4.4X _ ______] pLinear= \t0x%2.2X \n", pLinear_firstByteIndex, pLinear);
			// channel 3 reserved bytes
			reserved_firstByteIndex = pLinear_lastByteIndex + 1;
			reserved_lastByteIndex = reserved_firstByteIndex + reserved.size()-1; // 3 bytes => offset= 2
			for (uint32_t i = 0; i < reserved.size(); i++)
			{
				reserved[i] = bytes[reserved_firstByteIndex+i];
			}
			// channel xSampling int
			samplingX_firstByteIndex = reserved_lastByteIndex + 1;
			samplingX_lastByteIndex = samplingX_firstByteIndex + sizeof(samplingX)-1; // int32_t is 4 bytes => offset= 3
			samplingX = exrTypes::read4Bytes((bytes.cbegin()+samplingX_firstByteIndex)._Ptr);
			// channel ySampling int
			samplingY_firstByteIndex = samplingX_lastByteIndex + 1;
			samplingY_lastByteIndex = samplingY_firstByteIndex + sizeof(samplingY); // int32_t is 4 bytes => offset= 3
			samplingY = exrTypes::read4Bytes((bytes.cbegin()+samplingY_firstByteIndex)._Ptr);
			// return last byte of channel data
			return samplingY_lastByteIndex;
		}

		uint32_t getValueSizeBytes() const
		{
			if (channel_firstByteIndex == 0 and channel_lastByteIndex == 0)
			{
				throw std::runtime_error("value is not yet analysed. Call analyseChannelAndGetLastByteIndex() first, then getValueSizeBytes() to get true size.");
			}
			return channel_lastByteIndex - channel_firstByteIndex + 1;	// channel value size in bytes


		}

		void printDetailed() const
		{
			printf("\t channel [0x%4.4X ~ 0x%4.4X] name= \t\'%s\'+\'\\0\' \n", name_firstByteIndex, name_lastByteIndex, name.c_str());
			const char* ch0PxTypeStr = 
				(pixelType == PIXELTYPE::UINT) ? "UINT" : 
				(pixelType == PIXELTYPE::HALF) ? "HALF" : 
				(pixelType == PIXELTYPE::FLOAT) ? "FLOAT" : "UNDEFINED TYPE";
			printf("\t\t [0x%4.4X ~ 0x%4.4X] type= \t0x%8.8X (%s) \n", pixelType_firstByteIndex, pixelType_lastByteIndex, pixelType, ch0PxTypeStr);
			printf("\t\t [0x%4.4X ~ 0x%4.4X] reserved= \t0x%2.2X 0x%2.2X 0x%2.2X \n", reserved_firstByteIndex, reserved_lastByteIndex, reserved[0], reserved[1], reserved[2]);
			printf("\t\t [0x%4.4X ~ 0x%4.4X] xSamplimg= \t0x%8.8X = %i \n", samplingX_firstByteIndex, samplingX_lastByteIndex, samplingX, samplingX);
			printf("\t\t [0x%4.4X ~ 0x%4.4X] ySamplimg= \t0x%8.8X = %i \n", samplingY_firstByteIndex, samplingY_lastByteIndex, samplingY, samplingY);
		}

		private:
		const enum PIXELTYPE
		{
			UINT = 0x00,
			HALF = 0x01,
			FLOAT = 0x02
		};

	};

	// -----------------------------------------------------------------------------

	/// Different architectural approaches to wrapping over the Channel(s) and Chlist for Attributes storing any type to have the same interface
	// v1, v2, v3
	/*

	/// v1.
	// holds the idea, but incorrect(?) class structure / sequence of operations.
	/// <summary>
	///		Class for general interface of several derived classes, such as Chlist, Compression, Box2i etc...
	///		Important that it stores type string, initialized once for and inside ("type" is private) the base class using AttribValueMaster constructor.
	///		Derived class must call this (base) class constructor explicitly in its own constructor using Constructor : BaseConstructor("myTypeName") {} pre-initializer code in C++. 
	///		On the other hand, base class provides a method getType() to retrieve the type name string later within the derived class.
	///		Also, its important to keep AttribValueMaster class virtual, since creating its instance must be impossible.
	/// </summary>
	/// <derivedReadme>
	///		Derived class must explicitly call AttribValueMaster constructor in its own pre-initializer section, like this:
	///		MyDerived() : AttribValueMaster("myTypeName") {...}		// "myTypeName" is written to "type" parameter of AttribValueMaster
	///																// it is recommended to be set statically once within that line of code.
	///		To retrieve "type" value once (to "myTypeName" in this case), Derived class may call const std::string getType() function of the base class.
	///		Warning: overriding getType() of base class inside the derived will disallow you to retrieve the value of "type" string from the base class.
	/// </derivedReadme>
	class AttribValueMaster_v1
	{
		public:
		AttribValueMaster_v1(const std::string type) : type(type) {}
		virtual ~AttribValueMaster_v1() = default;
		virtual const void printInfo() const = 0;		// pure virtual, must be defined by derived class
		const std::string getType() const
		{
			return type;
		}

		private:
		const std::string type;
	};

	class Chlist_v1 : public AttribValueMaster_v1
	{
		public:
		Chlist_v1()
			: AttribValueMaster_v1("chlist")
		{
		}

		const void printInfo() const override
		{
			printf("type = %s", getType().c_str());
			return;
		}
	};



	/// v2: same idea, but 
	// using polymorphism
	class AttribValueMaster_v2
	{
		public:
		virtual const std::string getType() const final
		{
			return type;
		}

		protected:
		// "explicit" prevents implicit conversions whe na single parameter constructor is used.
		// without it, this constructor can be used for implicit conversions, which may lead to unintended behaviour.
		// with "explicit", the following code wont compile:
		// class A { public: explicit A(std::string str) : u_str(str) {} }
		// int main() { 
		//		A a = "myStr";	// COMPILATION ERROR (implicit conversion is not allowed)
		//		A a("myStr");	// OK (explicitly calling the constructor)
		// }
		explicit AttribValueMaster_v2(const std::string type) : type(type) {}
		virtual ~AttribValueMaster_v2() = default;

		private:
		const std::string type;
	};

	class ChlistMaster_v2 : public AttribValueMaster_v2
	{
		protected:
		ChlistMaster_v2() : AttribValueMaster_v2(type) {}
		virtual ~ChlistMaster_v2() = default;

		private:
		static const std::string type;

		// const std::string getType() const override {return "COMPILE ERROR"; }	// must be un-overridable
	};
	const std::string ChlistMaster_v2::type = "chlist";

	class Chlist_v2 : public ChlistMaster_v2
	{
		public:
		Chlist_v2()
		{
		}

		const void printInfo() const
		{
			printf("type = %s", getType().c_str());
			return;
		}
	};



	/// v3: same idea, but
	// using Compile-time polymorphism
	template <typename Derived>
	class AttribValueMaster_v3
	{
		public:
		virtual const std::string getType() const final
		{
			return type;
		}

		protected:
		static const std::string type;
	};

	template <typename Derived>
	const std::string AttribValueMaster_v3<Derived>::type = Derived::staticTypeName;

	class Chlist_v3 : public AttribValueMaster_v3<Chlist_v3>
	{
		public:
		static constexpr const char* staticTypeName = "chlist";
	};



	*/

	// -----------------------------------------------------------------------------

	// v5: same idea, but compilable
	/// <summary>
	///		Adds a common wrapping around ValueType, so the interface is common for all valueTypes.
	///		Ideally, it should've had ValueType getValue() instead of std::sting getValueString(). But since it was not proved 
	///		to work (requires testing) => AttribValue_v5 class's implements getValueString() returning value string, not value itself.
	/// </summary>
	/// <how-to-use>
	///		You can use AttribValue_v5 class to create your specific AttribValueTypeXYZValue in a way that it shares finctionality with other
	///		AttribValueType___Value classes (derived from AttribValue_v5). To implement thi, do the following:
	///		1.	Create a class that inherits from AttribValue_v5 (public inheritance).
	///		2.	Inside your new derived class, you must implement pure virtual type() function of the base class.
	///			Note: type()'s implementation must return the string with the name of the type (the type of attribute value), like this. 
	///			We strongly recommend to keep the returned string value the same as the class name.
	///				const std::string type() const override { return "myTypeName"; }
	///		3. Define type-specific/desired data fields to store data and/or methods. Those methods will work only with the instance of this (derived) class.
	///		4. Use your class to store OpenEXR's attribute(s) value(s) according to the type of value specified in the derived class. At the same time, 
	///			this class shares functionality with each class derived from AttribValue_v5.
	/// </how-to-use>
	/// <pros-cons>
	///		(type) string is not stored neither inside the base class (copy per each instance's base class), 
	///		nor statically through the complex structure of classes (once per sub-base class (like, TypeXYZMaster)).
	/// </pros-cons>
	// template <typename ValueType>	// doesnt need this because of std::string getValueStr() instead of ValueType getValue()
	class AttribValue_v5
	{
		std::string u_name = "";

		public:
		virtual const std::string name() const final
		{
			return u_name;
		}
		virtual const std::string type() const = 0;				// requires implementation in derived class
		virtual const uint32_t valueSizeInBytes() const = 0;	//  requires implementation in derived class
		virtual const std::string getValueString() const = 0;

		protected:
		AttribValue_v5(const std::string name) : u_name(name) {}
		virtual ~AttribValue_v5() = default;

	};

	/* document tag [EXR-ATTRIB-TYPES-01] */
	typedef std::vector<Channel> storedValueType;
	class Chlist_v5 : public AttribValue_v5
	{
		storedValueType channels;

		public:
		Chlist_v5(const std::string& name)
			: 
			AttribValue_v5(name)
		{
		}

		Chlist_v5(const std::string& name, const storedValueType& value)
			: 
			AttribValue_v5(name),
			channels(value)
		{
		}

		const std::string type() const override
		{
			return std::string("chlist");
		}

		const uint32_t valueSizeInBytes() const override
		{
			return 5;
		}

		const std::string getValueString() const override
		{
			return "channels string";
		}

	};

	/// todo ? (check next version for this already done)
	// holds the ideas, addition to the last version of class structure
	// move to Chlist and Attribute classes (above) accordingly.
	/*
	class Chlist
	{
		public:
		uint8_t chlist_some_data;

		Chlist(uint8_t somedata) {}
		void* analyse(const std::vector<u8>& probablyExrFilebytes) {}
	};

	extern class Attribute
	{
		public:
		std::string name = "";
		std::string type = "";
		uint32_t size = 0;		// size of value, in bytes
		std::vector<u8> value;

		Attribute(const std::vector<u8>& filebytes) {}
	};
	*/

	class Attribute_v5
	{
		private:
		std::string u_name = "";
		const AttribValue_v5* u_value;			// required to store any object of type derived from AttribValue, pointer??? need a copy

		public:
		virtual const std::string getName() const final		// prohibits overriding
		{
			return u_name;
		}
		virtual const std::string getType() const final
		{
			return u_value->type();
		}
		virtual const uint32_t getValueSizeBytes() const final
		{
			return u_value->valueSizeInBytes();
		}
		virtual const std::string getValueString() const final
		{
			u_value->getValueString();
		}

		protected:
		Attribute_v5(const std::string& name, const AttribValue_v5* value) : u_name(name), u_value(value) {}
		virtual ~Attribute_v5() = default;

	};

	// ------------------------------------------------------------------------------

	// v6
	/// v6: pros & cons
	/// using v6 now, if you need Attribute_v6 to store differently structured value (instead of Chlist), you no longer need 
	/// to rewrite neither AttribValue_v6 nor Attribute_v6, because AttribValue_v6 secures & provides common interface.
	
	class AttribValue_v6
	{
		public:
		virtual const std::string type() const = 0;				// requires implementation in derived class
		virtual const uint32_t valueSizeInBytes() const = 0;	//  requires implementation in derived class
		virtual const std::string getValueString() const = 0;

		protected:
		AttribValue_v6(/*const std::string name*/)/* : u_name(name) */{}
		virtual ~AttribValue_v6() = default;

	};

	/* document tag [EXR-ATTRIB-TYPES-01] */
	typedef std::vector<Channel> storedValueType;
	class Chlist_v6 : public AttribValue_v6
	{
		storedValueType channels;

		public:
		Chlist_v6()
		{
		}

		Chlist_v6(const storedValueType& value)
			: channels(value)
		{
		}

		const std::string type() const override
		{
			return std::string("chlist");
		}

		const uint32_t valueSizeInBytes() const override
		{
			return 5;
		}

		const std::string getValueString() const override
		{
			return "channels string";
		}

	};

	/// <summary>
	///		Using AttribValue_v6* secures & provides common interface for accessing value underneath it, so the implementation 
	///		of Attribute_v6 no longer dependent on internal data structure of different values it can store.
	/// </summary>
	class Attribute_v6
	{
		private:
		std::string u_name = "";
		const AttribValue_v6* u_value;			// required to store any object of type derived from AttribValue, pointer??? need a copy

		public:
		const std::string getName() const		// non-virtual method can not be overriden
		{
			return u_name;
		}
		const std::string getType() const
		{
			return u_value->type();
		}
		const uint32_t getValueSizeBytes() const
		{
			return u_value->valueSizeInBytes();
		}
		const std::string getValueString() const
		{
			u_value->getValueString();
		}

		protected:
		Attribute_v6(const std::string& name, std::vector<u8> filebytes, uint32_t attribFirstByteIndex)
			: 
			u_name(name)
		{
			/// todo
			// analyse filebytes and save values to AttribValue or something
			// then do u_value = new AttribValue_v6( /* pass acquired data for AttribValue */ );
			
		}
		virtual ~Attribute_v6() = default;

	};
}

static void AttribValue_vX_test()
{
	/// v2
	//exrTypes::Chlist_v2 value0 = exrTypes::Chlist_v2();
	//exrTypes::Chlist_v2 value1 = exrTypes::Chlist_v2();
	//exrTypes::AttribValueMaster_v2 v1 = exrTypes::AttribValueMaster_v2("chlist");	// must be inaccessible
	//exrTypes::ChlistValueMaster_v2 v2 = exrTypes::ChlistValueMaster_v2();			// must be inaccessible
	//printf("\n\n ------------------- \n value0 type = [0x%8.8X] %s", value0.getType().c_str(), value0.getType().c_str());	// 1 and 2 give diff. string address, idk why
	//printf("\n\n ------------------- \n value0 type = [0x%8.8X] %s", value0.getType().c_str(), value0.getType().c_str());
	//printf("\n\n ------------------- \n value1 type = [0x%8.8X] %s", value1.getType().c_str(), value1.getType().c_str());

	/// v5 (with std::string u_name)
	//exrAttribExperimental::Chlist_v5 ch0 = exrAttribExperimental::Chlist_v5("exrStandardChlist");
	//printf("ch0 type = %s", ch0.type().c_str());

	/// v6 (without std::string u_name)
	// Attribute_v6 requires filebytes, so better test it with real .exr file bytes
	///exrAttribExperimental::Attribute_v6 att = exrAttribExperimental::Attribute_v6("exrStandardChlist");
	///printf("ch0 type = %s", ch0.type().c_str());

}
