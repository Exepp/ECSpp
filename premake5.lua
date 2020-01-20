workspace "ECSpp"

	language "C++"

	architecture "x64"

	cppdialect "C++17"

	configurations { "Debug", "Release" }


	filter {"system:windows"}
		links { "Shlwapi" }
		systemversion "latest"

	filter {"system:linux"}
		toolset "clang"
		buildoptions{"-fPIC -Wno-dangling-else"}


	filter "configurations:Debug"
		defines "EPP_DEBUG"
		symbols "On"
		
	filter "configurations:Release"
		defines {"EPP_RELEASE"}
		optimize "Speed"
		symbols "On"
	filter{}
	
	includedirs
	{
		"./include/",
		"./external/include/"
	}

	libdirs
	{
		"./external/lib/"
	}

outDir = "%{cfg.system}_%{cfg.architecture}/%{cfg.buildcfg}/"

	targetdir ("bin/" .. outDir .. "%{prj.name}")

	objdir ("bin_inter/" .. outDir .. "%{prj.name}")


project "ECSpp"

	filter "configurations:Debug"
		targetsuffix "_d"
	filter {}

	kind "StaticLib"

	location "./"

	files
	{
		"%{prj.location}/src/%{prj.name}/**",
		"%{prj.location}/external/src/%{prj.name}/**",
	}


project "Tests"

	kind "ConsoleApp"

	location "./"

	files
	{
		"%{prj.location}/src/%{prj.name}/**"
	}
	links {"ECSpp", "gtest_main", "gtest", "pthread"}
	-- filter "configurations:Debug"
	-- 	links {}
	-- filter "configurations:Release"
	-- 	links {}
	filter {}

	
project "Benchmarks"

	kind "ConsoleApp"

	location "./"

	files
	{
		"%{prj.location}/src/%{prj.name}/**"
	}

	links {"pthread", "ECSpp", "benchmark_main", "benchmark"}
	-- filter "configurations:Debug"
	-- 	links {}
	-- filter "configurations:Release"
	-- 	links {}
	-- filter {}