workspace "ECSpp"

	language "C++"

	architecture "x64"

	cppdialect "C++17"

	configurations { "Debug", "Release" }


	filter {"system:windows"}
		links { "Shlwapi" }
		systemversion "latest"

	filter {"system:linux"}
		links { "pthread" }
		toolset "clang"
		buildoptions{"-fPIC"}


	filter "configurations:Debug"
		defines "EPP_DEBUG"
		symbols "On"
		
	filter "configurations:Release"
		defines "EPP_RELEASE"
		optimize "On"
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
		-- targetname ("%{prj.name}" .. "_d")
	filter {}

	kind "StaticLib"

	location "./"

	files
	{
		"%{prj.location}/src/**",
		"%{prj.location}/external/src/**",
	}


project "Tests"

	kind "ConsoleApp"

	location "./"

	files
	{
		"%{prj.location}/Tests.cpp"
	}

	filter "configurations:Debug"
		links { "ECSpp", "gtest_maind", "gtestd" }
	filter "configurations:Release"
		links { "ECSpp", "gtest_main", "gtest", }
	filter {}

	
project "Benchmarks"

	kind "ConsoleApp"

	location "./"

	files
	{
		"%{prj.location}/Benchmarks.cpp"
	}

	filter "configurations:Debug"
		links { "ECSpp", "benchmark_maind", "benchmarkd" }
	filter "configurations:Release"
		links { "ECSpp", "benchmark_main", "benchmark" }
	filter {}