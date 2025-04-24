#pragma once
#include <cstdint>
#include <string>
#include <format>

namespace exrTypes_demo
{
	// ---- demo variant (v1) of what I want from code architecture
	/// delete later if not used

	/// <summary>
	///		Architecture utilizes the fact that base class constructor is called first, 
	///		then derived class constructor is called, allowing us to do the common
	///		initialization part for all derived classes within the base constructor
	///		and do the derived-class-specific part within the derived class constructor.
	/// </summary>
	/// <pros-cons>
	///		Pros: 
	///		[+] reduces code redundancy & code repeat.
	///		Cons: 
	///		[-] additional ***Master base class is created, making the code more complex.
	/// </pros-cons>
	/// <todo>
	///		Create classes: demo_AttribMaster_v2, demo_AttribABC_v2, TypeMaster_v2, and TypeXYZ_v2.
	///		Combine this behaviour with the new flexible value bahaviour. 
	///		Explanation: lets say, instead of Type value; we create class TypeMaster 
	///		and class TypeXYZ. TypeMaster is base class of TypeXYZ. 
	///		AttribMaster stores ( TypeXYZ value; ) of a fixed type (& therefore fixed functionality).
	///		From now, AttribMaster stores new ( TypeMaster* value; ) which allows assigning
	///		it objects of any class derived from TypeMaster, allowing flexible data & functionality.
	///		
	///		This new behaviour is based on facts, that
	///		1) base class pointers can store pointers to objects of self-type or any derived type.
	///		2) calling methods from a BASE class pointer which 
	///			holds DERIVED class object pointer, calls the method of DERIVED class. (? test)
	/// </todo>
	class demo_AttribMaster
	{
		public:
		demo_AttribMaster(const std::string& p_value)
		{
			// allocate memory to copy data from input
			value = new std::string(p_value.c_str());
		}

		virtual void initValue() = 0;
		virtual void print() const = 0;

		~demo_AttribMaster()
		{
			delete value;
			value = nullptr;
		}

		private:
		std::string name = "";
		std::string type = "";
		int32_t valueSizeBytes = 0;

		protected:
		std::string* value = nullptr;
	};

	class demo_AttribABC : private demo_AttribMaster
	{
		public:
		demo_AttribABC(const std::string& input)
			: demo_AttribMaster(input)
		{
			//initValue();		// variant 1
			(*value) += "ABC";	// variant 2
		}

		private:
		// optional method if you'd like to put value initialization into dedicated method
		void initValue() override
		{
			//(*value) += "ABC";	// variant 1
			return;					// variant 2
		}

		public:
		void print() const override
		{
			printf("AttribABC.value = %s \n", (*value).c_str());
		}
	};

	// ----- demo variant (v2) of what I want from code architecture & behaviour

	/// <summary>
	///		TypeInterface specifies common data & methods for all data types (that will be used in attribute value).
	///			Must be implemented by derived class(es). How to implement - see TypeABC, TypeXYZ description.
	///		TypeABC, TypeXYZ etc... implement common interface & their own specific data & methods for their stored value type.
	///			User's class that impelemnts TypeInterface (using public inheritance), must override methods of interface as follows:
	///				std::string type() const { /* return string of name of type of your stored value (it is used as your type name-id) */ }
	///				uint32_t sizeInBytes() const { /* return number of bytes that only your value takes */ }
	///				std::string toString() const { /* return all your class data (intended for user) into a form of string */ }
	///		AttribInterface specifies common code for all OpenEXR attributes (unpacking attribName, attribValueType, attribValueSizeInBytes) from an .exr file.
	///		AttribABC implements value-type-specific code (unpacking value of its correspondong OpenEXR attribute value type 
	///			from .exr file) and stores the unpacked value & (optionally) additonal OpenEXR attribute data. 
	///			To do so, Attrib___ (for example, AttribABC) class MUST
	///				* store object of class derived from TypeInterface class, as follows:
	///					private: TypeABC value; // or TypeABC* valuePtr = nullptr;
	///				* further initialize it inside the AttribABC constructor, as follows:
	///					<redo>
	///					AttribABC(const std::string& name, const int32_t valueSizeBytes, const TypeStr_v2& value) 
	///						: AttribInterface(name, value.sizeInBytes())
	///					{
	///						val = new TypeStr_v2(value.val + "+demo_AttribTypeStr_v2.work");	// value-type-specific code
	///					}
	///					</redo>
	///				* and implement AttribInterface class (using private inheritance), as follows:
	///					<redo>
	///						std::string valueType() const { /* return value->type(); */ }
	///					</redo>
	///				* implement method(s) with type-specific declaration:
	///					TypeABC value() const { /* return a copy of the stored value */ }
	///				
	/// </summary>

	class TypeInterface_v2
	{
		public:
		virtual std::string type() const = 0;
		virtual uint32_t sizeInBytes() const = 0;
		virtual std::string toString() const = 0;

		protected:
		TypeInterface_v2() {}
	};

	// 1. user defines their own class that specifies the handling of the value they are about to store in the derived class of AttribMaster
	class TypeStr_v2 : public TypeInterface_v2
	{
		// private:
		public:
		std::string val = "";

		public:
		TypeStr_v2(const std::string input) : val(input) {}
		TypeStr_v2(const TypeStr_v2& copy) : val(copy.val) {}

		std::string type() const { return "string"; }
		uint32_t sizeInBytes() const override { return val.size(); }
		std::string toString() const override { return val; }

		//std::string value() const { return val; }
		//void setValue(const std::string& newValue) { val = newValue; }

	};

	class TypeUint_v2 : public TypeInterface_v2
	{
		// private:
		public:
		uint32_t val = 0;

		public:
		TypeUint_v2(const uint32_t input) : val(input) {}
		TypeUint_v2(const TypeUint_v2& copy) : val(copy.val) {}

		std::string type() const { return "uint32_t"; }							// user-defined type name string
		uint32_t sizeInBytes() const override { return sizeof(val); }			// user-defined sizeInBytes calculation
		std::string toString() const override { return std::to_string(val); }	// user-defined to-string conversion

		//uint32_t value() const { return val; }
		//void setValue(const uint32_t& newVal) { val = newVal; }
	};

	//// todo: instead of storing valueType & valueSieInBytes, take const TypeInterface* ptr as input (the object 
	////		of TypeABC is stored in derived class), store it and implement AttribInterface's methods as follows:
	////			virtual std::string name() const final { return u_name; }					// stored by each Attrib => AttribInterface
	////			virtual std::string valueType() const final { return valPtr->type(); }		// stored by each value => TypeInterface
	////			virtual uint32_t valueSizeInBytes() const final { return valPtr->sizeIOnBytes(); }	// stored by TypeInterface
	class AttribInterface_v2
	{
		protected:
		AttribInterface_v2(const std::string name, const TypeInterface_v2* valuePointer)
		{
			/// common code for all sub-classes of AttribMaster
			/// set attrib name, attrib value type & sizeInBytes + attribName_FirstByteIndex, ..., attribValue_FirstByteIndex
			u_name = "fromFilebytes.name";
			
			/// following code should be outside (in .exr attrib unpacker)
			std::string valueType = "fromFilebytes.valueType";
			if (valueType == "str")
			{
				u_valuePtr = new TypeStr_v2("ABC");
			}
			else
			{
				u_valuePtr = new TypeUint_v2(69);
			}
			/// code above should be outside (in .exr attrib unpacker code section)

			u_valueSizeBytes = u_valuePtr->sizeInBytes();
		}

		public:
		virtual std::string name() const final { return u_name; }	// un-overridable in derived classes, Attrib.name
		virtual std::string valueType() const final { return u_valuePtr->type(); }
		virtual uint32_t valueSizeInBytes() const final { return u_valuePtr->sizeInBytes(); }
		virtual std::string toString() const = 0;

		private:
		std::string u_name = "";
		int32_t u_valueSizeBytes = 0;	// not needed if value pointer is stored

		const TypeInterface_v2* u_valuePtr = nullptr;
	};

	class AttribTypestr_v2 : private AttribInterface_v2
	{
		public:
		AttribTypestr_v2(const std::string& name, const int32_t valueSizeBytes, const TypeStr_v2& value)
			: AttribInterface_v2(name, &value)
		{
			/// set attrib value from filebytes using attribValue_FirstByteIndex from base class, set base.attribValue_LastByteIndex



			//val = new TypeStr_v2(value.val + "+demo_AttribTypeStr_v2.work");	// value-type-specific code
			val = new TypeStr_v2("fromFiles.value");	// value-type-specific code
		}

		~AttribTypestr_v2()
		{
			delete val;
			val = nullptr;
		}

		public:
		std::string toString() const override
		{
			return std::string("AttribTypeStr" + std::format(" {:08X} :", uint32_t(this)) + " name= " + name() + ", value= " + val->toString());
		}



		private:
		TypeStr_v2* val = nullptr;

	};

	// 2. User creates demo_AttribTypeXYZ class (derived from AttribMaster_v2) that implements the base AttribMaster (attribute interface) 
	//		+ specifies handling of (value) of the user-specific type it stores.

	void demo_arch_v2_test()
	{
		TypeStr_v2 strABC = TypeStr_v2("ABC");
		AttribTypestr_v2 attribABC = AttribTypestr_v2("nameOfABC", 47, strABC);
		printf("%s", attribABC.toString().c_str());
	}




	// --------- v3








	/// <summary>
	///		TypeInterface specifies common data & methods for all data types (that will be used in attribute value).
	///			Must be implemented by derived class(es). How to implement - see TypeABC, TypeXYZ description.
	///		TypeABC, TypeXYZ etc... implement common interface & their own specific data & methods for their stored value type.
	///			User's class that impelemnts TypeInterface (using public inheritance), must override methods of interface as follows:
	///				std::string type() const { /* return string of name of type of your stored value (it is used as your type name-id) */ }
	///				uint32_t sizeInBytes() const { /* return number of bytes that only your value takes */ }
	///				std::string toString() const { /* return all your class data (intended for user) into a form of string */ }
	///		AttribInterface specifies common code for all OpenEXR attributes (unpacking attribName, attribValueType, attribValueSizeInBytes) from an .exr file.
	///		AttribABC implements value-type-specific code (unpacking value of its correspondong OpenEXR attribute value type 
	///			from .exr file) and stores the unpacked value & (optionally) additonal OpenEXR attribute data. 
	///			To do so, Attrib___ (for example, AttribABC) class MUST
	///				* store object of class derived from TypeInterface class, as follows:
	///					private: TypeABC value; // or TypeABC* valuePtr = nullptr;
	///				* further initialize it inside the AttribABC constructor, as follows:
	///					<redo>
	///					AttribABC(const std::string& name, const int32_t valueSizeBytes, const TypeStr_v2& value) 
	///						: AttribInterface(name, value.sizeInBytes())
	///					{
	///						val = new TypeStr_v2(value.val + "+demo_AttribTypeStr_v2.work");	// value-type-specific code
	///					}
	///					</redo>
	///				* and implement AttribInterface class (using private inheritance), as follows:
	///					<redo>
	///						std::string valueType() const { /* return value->type(); */ }
	///					</redo>
	///				* implement method(s) with type-specific declaration:
	///					TypeABC value() const { /* return a copy of the stored value */ }
	///				
	/// </summary>

	class TypeInterface_v3
	{
		public:
		virtual std::string type() const = 0;
		virtual uint32_t sizeInBytes() const = 0;
		virtual std::string toString() const = 0;

		protected:
		TypeInterface_v3() {}
	};

	// 1. user defines their own class that specifies the handling of the value they are about to store in the derived class of AttribMaster
	class TypeStr_v3 : public TypeInterface_v3
	{
		// private:
		public:
		std::string val = "";

		public:
		TypeStr_v3(const std::string input) : val(input) {}
		TypeStr_v3(const TypeStr_v2& copy) : val(copy.val) {}

		std::string type() const { return "string"; }
		uint32_t sizeInBytes() const override { return val.size(); }
		std::string toString() const override { return val; }

		//std::string value() const { return val; }
		//void setValue(const std::string& newValue) { val = newValue; }

	};

	class TypeUint_v3 : public TypeInterface_v3
	{
		// private:
		public:
		uint32_t val = 0;

		public:
		TypeUint_v3(const uint32_t input) : val(input) {}
		TypeUint_v3(const TypeUint_v2& copy) : val(copy.val) {}

		std::string type() const { return "uint32_t"; }							// user-defined type name string
		uint32_t sizeInBytes() const override { return sizeof(val); }			// user-defined sizeInBytes calculation
		std::string toString() const override { return std::to_string(val); }	// user-defined to-string conversion

		//uint32_t value() const { return val; }
		//void setValue(const uint32_t& newVal) { val = newVal; }
	};

	//// todo: instead of storing valueType & valueSieInBytes, take const TypeInterface* ptr as input (the object 
	////		of TypeABC is stored in derived class), store it and implement AttribInterface's methods as follows:
	////			virtual std::string name() const final { return u_name; }					// stored by each Attrib => AttribInterface
	////			virtual std::string valueType() const final { return valPtr->type(); }		// stored by each value => TypeInterface
	////			virtual uint32_t valueSizeInBytes() const final { return valPtr->sizeIOnBytes(); }	// stored by TypeInterface
	class AttribInterface_v3
	{
		protected:
		AttribInterface_v3(const std::string& name, const std::string filebytes, const uint32_t attrib_FirstByteIndex, const TypeInterface_v3* valuePointer)
		{
			/// common code for all sub-classes of AttribMaster
			/// set attrib name, attrib value type & sizeInBytes + attribName_FirstByteIndex, ..., attribValue_FirstByteIndex
			u_name = "fromFilebytes.name";
			if (u_name != name)
			{
				throw;
				// retrieved attrib name is different from specified, are you sure attrib_FirstByteIndex points to the attribute you need ?
				// or have you specified the correct attribute name ?
			}
			u_valueType = "str";
			u_valueSizeBytes = 02;
			u_value_FirstByteIndex = 03;
			u_value_LastByteIndex = u_value_FirstByteIndex + u_valueSizeBytes-1;

			/// following code should be outside (in .exr attrib unpacker)
			if (u_valueType == "str")
			{
				u_valuePtr = new TypeStr_v3("ABC");
			}
			else
			{
				u_valuePtr = new TypeUint_v3(69);
			}
			/// code above should be outside (in .exr attrib unpacker code section)

		}

		public:
		virtual std::string name() const final { return u_name; }	// un-overridable in derived classes, Attrib.name
		virtual std::string valueType() const final { return u_valuePtr->type(); }
		virtual uint32_t valueSizeInBytes() const final { return u_valuePtr->sizeInBytes(); }
		virtual std::string toString() const = 0;
		virtual uint32_t valueFirstByteIndex() const final { return u_value_FirstByteIndex; }
		virtual uint32_t valueLastByteIndex() const final { return u_value_LastByteIndex; }

		private:
		std::string u_name = "";
		int32_t u_valueSizeBytes = 0;	// not needed if value pointer is stored
		std::string u_valueType = "";
		uint32_t u_value_FirstByteIndex, u_value_LastByteIndex = 0;

		const TypeInterface_v3* u_valuePtr = nullptr;
	};

	class AttribTypestr_v3 : private AttribInterface_v3
	{
		public:
		AttribTypestr_v3(const std::string& name, const std::string& filebytes, const int32_t attrib_FirstByteIndex, const TypeStr_v3& value)
			: AttribInterface_v3(name, filebytes, 01, &value)
		{
			/// set attrib value from filebytes using attribValue_FirstByteIndex from base class, set base.attribValue_LastByteIndex
			// value-type-specific code
			printf("\n value_firstByteIndex = %u \n", valueFirstByteIndex());
			val = new TypeStr_v3("fromFilebytes.attribValue");
			uint32_t value_LastByteIndex = 33;	// position after reading value from filebytes
			if (value_LastByteIndex != valueLastByteIndex())
			{
				throw;	// that value is of different size than specified in attribute, error may have occured during READ.
			}

		}

		~AttribTypestr_v3()
		{
			delete val;
			val = nullptr;
		}

		public:
		std::string toString() const override
		{
			return std::string("AttribTypeStr" + std::format(" {:08X} :", uint32_t(this)) + " name= " + name() + ", value= " + val->toString());
		}

		private:
		TypeStr_v3* val = nullptr;

	};

	// 2. User creates demo_AttribTypeXYZ class (derived from AttribMaster_v2) that implements the base AttribMaster (attribute interface) 
	//		+ specifies handling of (value) of the user-specific type it stores.

	void demo_arch_v3_test()
	{
		TypeStr_v3 strABC = TypeStr_v3("ABC");
		AttribTypestr_v3 attribABC = AttribTypestr_v3("nameOfABC", "filebytes", 47, strABC);
		printf("%s", attribABC.toString().c_str());
	}
}
