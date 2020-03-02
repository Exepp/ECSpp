workspace "ECSpp"

	language "C++"

	architecture "x64"

	cppdialect "C++17"

	configurations { "Debug", "Release" }


	filter {"system:windows"}
		systemversion "latest"
		staticruntime "on"

	filter {"system:linux"}
		if os.getenv("EPP_COVERAGE") == "1" then
			toolset "gcc" -- do not change
		else
			toolset "clang"
		end
		buildoptions{"-Wno-dangling-else",
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

					-- "-fno-omit-frame-pointer",
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

project "Tests"

	kind "ConsoleApp"

	location "./"

	files { "%{prj.location}/src/%{prj.name}/**" }

	filter "configurations:Debug"
		links {"gtest_maind", "gtestd"}
	filter "configurations:Release"
		links {"gtest_main", "gtest"}
	filter "toolset:gcc"
		buildoptions {"--coverage"}
		links {"gcov"}
	filter {"system:linux"}
		links {"pthread"}
	filter{}

project "Benchmarks"

	kind "ConsoleApp"

	location "./"

	files { "%{prj.location}/src/%{prj.name}/**" }

	filter "configurations:Debug"
		links {"benchmark_maind", "benchmarkd"}
	filter "configurations:Release"
		links {"benchmark_main", "benchmark"}
	filter{}

	filter {"system:linux"}
		links {"pthread"}
	filter {"system:windows"}
		links {"Shlwapi"}
	filter{}