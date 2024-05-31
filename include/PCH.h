#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <Windows.h>
#include <fstream>
#include <spdlog/sinks/basic_file_sink.h>

#include <ClibUtil/editorID.hpp>
#include <ClibUtil/simpleINI.hpp>
#include <ClibUtil/singleton.hpp>
#include <ClibUtil/string.hpp>

#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;

#include "Version.h"

#define _loggerInfo SKSE::log::info
#define _1_6_1170 (unsigned short)1U, (unsigned short)6U, (unsigned short)1170U, (unsigned short)0U

namespace stl {
	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		SKSE::AllocTrampoline(14);

		auto& trampoline = SKSE::GetTrampoline();
		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}
}

#ifdef SKYRIM_AE
#	define OFFSET(se, ae) ae
#	define OFFSET_3(se, ae, vr) ae
#else
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) se
#endif