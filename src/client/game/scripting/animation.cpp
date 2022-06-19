#include <std_include.hpp>
#include "array.hpp"
#include "script_value.hpp"
#include "execution.hpp"

namespace scripting
{
	animation::animation(unsigned int value)
		: value_(value)
	{
	}

	uint64_t animation::get_value() const
	{
		return this->value_;
	}
}
