include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "Sloth"
	architecture "x86_64"
	startproject "Sloth-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/premake"
	include "Sloth/vendor/Box2D"
	include "Sloth/vendor/GLFW"
	include "Sloth/vendor/Glad"
	include "Sloth/vendor/imgui"
	include "Sloth/vendor/yaml-cpp"
group ""

include "Sloth"
include "Sandbox"
include "Sloth-Editor"
