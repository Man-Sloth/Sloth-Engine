#pragma once

#include "Sloth/Core/Base.h"
#include "Sloth/Core/Log.h"
#include <filesystem>

#ifdef SLTH_ENABLE_ASSERTS

	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define SLTH_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { SLTH##type##ERROR(msg, __VA_ARGS__); SLTH_DEBUGBREAK(); } }
	#define SLTH_INTERNAL_ASSERT_WITH_MSG(type, check, ...) SLTH_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define SLTH_INTERNAL_ASSERT_NO_MSG(type, check) SLTH_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", SLTH_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define SLTH_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define SLTH_INTERNAL_ASSERT_GET_MACRO(...) SLTH_EXPAND_MACRO( SLTH_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, SLTH_INTERNAL_ASSERT_WITH_MSG, SLTH_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define SLTH_ASSERT(...) SLTH_EXPAND_MACRO( SLTH_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define SLTH_CORE_ASSERT(...) SLTH_EXPAND_MACRO( SLTH_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define SLTH_ASSERT(...)
	#define SLTH_CORE_ASSERT(...)
#endif
