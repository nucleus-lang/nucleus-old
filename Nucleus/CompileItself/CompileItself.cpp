#include "CompileItself.h"

void CompileItself::Compile()
{
	std::cout << "Compiling Nucleus..." << std::endl;

    std::string getMSBuild = Files::GetPathFromCommand(Files::ConvertToWindowsCmdPath("C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe") + " -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find MSBuild/**/Bin/MSBuild.exe");
    std::string visualStudioVersion = Files::GetValueFromCommand(Files::ConvertToWindowsCmdPath("C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe") + " -latest -prerelease -products * -property productLineVersion");
    std::string premakeCmd = "premake5-windows.exe vs" + visualStudioVersion;

    if (getMSBuild != "")
    {
        system(premakeCmd.c_str());
        system(Files::ConvertToWindowsCmdPath(getMSBuild).c_str());
        std::cout << "Nucleus Compiled!" << std::endl;
    }
    else
    {
        std::cout << "No compiler found :c" << std::endl;
    }
}