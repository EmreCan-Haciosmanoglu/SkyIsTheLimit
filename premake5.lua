project "Game"
    location "Game"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"
        
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
        
    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.cpp",
    }
        
    includedirs
    {
        "$(ProjectDir)src/",
        "$(SolutionDir)Can/vendor/spdlog/include",
        "$(SolutionDir)Can/src",
        "$(SolutionDir)%{IncludeDir.imgui}",
        "$(SolutionDir)%{IncludeDir.glm}",
        "$(SolutionDir)%{IncludeDir.EnTT}",
        "$(SolutionDir)%{IncludeDir.FreeType}"
    }
    
    links
    {
        "imgui",
        "Can"
    }
        
    filter "system:windows"
        systemversion "latest"
        
    filter "configurations:Debug"
        defines "CAN_DEBUG"
        runtime "Debug"
        symbols "on"
        
    filter "configurations:Release"
        defines "CAN_RELEASE"
        runtime "Release"
        optimize "on"
        
    filter "configurations:Dist"
        defines "CAN_DIST"
        runtime "Release"
        optimize "on"
            