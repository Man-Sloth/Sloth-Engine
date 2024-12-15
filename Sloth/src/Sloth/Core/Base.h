#pragma once

#include "Sloth/Core/PlatformDetection.h"

#include <memory>

#ifdef SLTH_DEBUG
	#if defined(SLTH_PLATFORM_WINDOWS)
		#define SLTH_DEBUGBREAK() __debugbreak()
	#elif defined(SLTH_PLATFORM_LINUX)
		#include <signal.h>
		#define SLTH_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define SLTH_ENABLE_ASSERTS
#else
	#define SLTH_DEBUGBREAK()
#endif

#define SLTH_EXPAND_MACRO(x) x
#define SLTH_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define SLTH_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Sloth {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#include "Sloth/Core/Log.h"
#include "Sloth/Core/Assert.h"
