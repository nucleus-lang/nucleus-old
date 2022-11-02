workspace "Nucleus"
	staticruntime "on"
	configurations { "Release" }
	platforms { "x86_64", "x86" }
	startproject "Pickaxe"
	warnings "off"


project "Nucleus"
	kind "ConsoleApp"
	language "C++"
	location "%{prj.name}"
	targetdir "%{prj.name}"

	files 
	{ 
		"%{prj.name}/**.h", 
		"%{prj.name}/**.c",
		"%{prj.name}/**.hpp",
		"%{prj.name}/**.cpp"
	}

	filter "configurations:Release"
		defines 
		{ 
			"_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING"
		}
		optimize "On"

    filter "platforms:x86"
	if(system == "windows") then
		defines
		{
			"WIN32"
		}
	end
        architecture "x86"

    filter "platforms:x86_64"
        architecture "x86_64"

    filter "system:windows"
    	systemversion "latest"
