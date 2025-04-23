#pragma once
#include <type_traits>

namespace templateConcept
{
	namespace numerical
	{
		/// C++20: concept (example in use)
		// 1. define a compile-time expression that checks if type T satisfies the requirements
		template <typename T>
		constexpr bool isStdNumericType = !std::is_same_v<T, bool> and (std::is_integral_v<T> or std::is_floating_point_v<T>);	// requires signed/unsigned int, int_t... float_t ... float, double, long double
		/*
			same as using this:
			void tryValidateTemplateType()
			{
				bool isValidType = 
					// C-types, unsigned
					std::is_same_v<NumberT, unsigned char>
					or std::is_same_v<NumberT, unsigned short>
					or std::is_same_v<NumberT, unsigned int>
					...
					// C-types, signed
					or std::is_same_v<NumberT, char>
					or std::is_same_v<NumberT, signed char>
					or std::is_same_v<NumberT, short>
					or std::is_same_v<NumberT, int>
					or std::is_same_v<NumberT, long>
					or std::is_same_v<NumberT, long long>
					...
					// fixed-width, unsigned
					or std::is_same_v<NumberT, uint8_t>
					or std::is_same_v<NumberT, uint16_t>
					...
					// fixed-width, signed
					or std::is_same_v<NumberT, int8_t>
					or std::is_same_v<NumberT, int16_t>
					...
					// C-types, floating-point
					or std::is_same_v<NumberT, float>
					or std::is_same_v<NumberT, double>
					or std::is_same_v<NumberT, long double>
					...
					// fixed-width, floating-point
					or std::is_same_v<NumberT, float32_t>
					or std::is_same_v<NumberT, float64_t>
					or std::is_same_v<NumberT, float80_t>;	// long double, 80 bits (if supported)
				return isValidType;
			}
		*/
		// 2. define a concept to constraint the template type
		template <typename T>
		concept StdNumeric = isStdNumericType<T>;
		// 3. use concept name instead of typename/class to tell compiler NumberT type must meet the StdNumeric constraint (=> isStdNumericType requirements)
		/*
			template <StdNumeric NumberT>
			class Range
			{
				Range(const NumberT first, const NumberT last) {...}
			};
		*/

		// other concepts

	}

	namespace functional
	{
		/// c++20: concept (short example in use)
		// 1. define a concept to constraint the template type
		template <typename T>
		concept CopyAssignable = std::is_copy_assignable_v<T>;	// MemIndexed uses only this->value = value; => requires value to be CopyAssignable
		// 2. use concept name instead of typename/class to tell compiler that
		/*
			template <CopyAssignable ValueType>
			class IndexedValue
			{
				public:
				IndexedValue(const uint32_t valueBeginIndex, const uint32_t valueEndIndex, ValueType value)
					: u_valueIndex(utils::Range<uint32_t>(valueBeginIndex, valueEndIndex)), u_value(value)	// calls u_value = value;
				{ ... }
				IndexedValue(const IndexedValue<ValueType>& other)
					: u_valueIndex(other.u_valueIndex), u_value(other.u_value)	// calls u_value = value;
				{ ... }
				IndexedValue& operator=(const IndexedValue& other)
				{
					u_value = other.u_value;	// calls u_value = value;
					...
					return *this;
				}

				private:
				ValueType u_value;
				utils::Range<uint32_t> u_valueIndex;
			};
		*/

		// other concepts


	}

}



