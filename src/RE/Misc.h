#pragma once

#include "offset.h"

namespace RE
{
	inline void ExampleAddedFunc() 
	{
		using func_t = decltype(&ExampleAddedFunc);
		static REL::Relocation<func_t> func{ Offset::Example::PleaseEditMe };
		return func();
	}
}