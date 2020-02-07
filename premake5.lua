workspace "ECSpp"

	language "C++"

	architecture "x64"

	cppdialect "C++17"

	configurations { "Debug", "Release" }


	filter {"system:windows"}
		systemversion "latest"
		staticruntime "on"

	filter {"system:linux"}
		toolset "clang"
		buildoptions{"-fPIC",
					"-Wno-dangling-else",
					-- "-Weverything",
					-- "-Wno-c++98-compat",
					-- "-Wno-unused-template",
					-- "-Wno-c++98-compat-pedantic",
					-- "-Wno-extra-semi-stmt",
					-- "-Wno-newline-eof",
					-- "-Wno-documentation-unknown-command",
					-- "-Wno-weak-vtables",
					-- "-Wno-exit-time-destructors",
					-- "-Wno-global-constructors",
					-- "-Wno-padded",
					}

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
		targetsuffix "d"
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

	files { "%{prj.location}/src/%{prj.name}/**" }

	filter "configurations:Debug"
		links {"ECSpp", "gtest_maind", "gtestd"}
	filter "configurations:Release"
		links {"ECSpp", "gtest_main", "gtest"}
	filter{}

	filter {"system:linux"}
		links {"pthread"}
	filter{}

project "Benchmarks"

	kind "ConsoleApp"

	location "./"

	files { "%{prj.location}/src/%{prj.name}/**" }

	filter "configurations:Debug"
		links {"ECSpp", "benchmark_maind", "benchmarkd"}
	filter "configurations:Release"
		links {"ECSpp", "benchmark_main", "benchmark"}
	filter{}

	filter {"system:linux"}
		links {"pthread"}
	filter {"system:windows"}
		links {"Shlwapi"}
	filter{}

project "UBTests"

	kind "ConsoleApp"

	location "./"

	files { "%{prj.location}/src/%{prj.name}/**" }

	filter "configurations:Debug"
		links {"ECSpp"}
	filter "configurations:Release"
		links {"ECSpp"}
	filter{}