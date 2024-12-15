project "Sloth-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Sloth/vendor/spdlog/include",
		"%{wks.location}/Sloth/src",
		"%{wks.location}/Sloth/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}"
	}

	links
	{
		"Sloth"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "SLTH_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SLTH_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SLTH_DIST"
		runtime "Release"
		optimize "on"
