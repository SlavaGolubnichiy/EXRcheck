# EXR check App
### Check your .exr file content in binary, hexadecimal and text form.
---

⚠️ Allows only OpenEXR 2.x files, RGBA, 32-bit floating point per channel, NO_COMPRESSION.

### Clone repository

<ol type=a>
	<li> copy SSH or HTTPS link to this repository on GitHub: [Code] -> HTTPS | SSH -> copy link</li>
	<li> open destination directory for repository -> run Git command: <code>git clone change_this_to_repo_link</code> </li>
</ol>

#### *Configure and build - using project CMake scripts*

0. prerequisites:
	* CMake version 4.0.1 or higher installed
	* ... ?
1. clone this repository:
	<ol type=a>
		<li> copy SSH or HTTPS link to this repository on GitHub: [Code] -> HTTPS | SSH -> copy link</li>
		<li> open destination directory for repository -> run Git command: <code>git clone paste_repo_link_here</code> </li>
	</ol>
2. open command prompt -> navigate to project directory (directory of this repository at your local machine).
3. configurate one of presets from CMakePresets.json. <br>
   In command prompt, run one command out of following:
	* <code>cmake --preset x64-debug</code>
	* <code>cmake --preset x86-debug</code>
	* <code>cmake --preset x86-release     # 🔴 builds errors, dont use yet </code>
	* <code>cmake --preset x86-release     # 🔴 builds errors, dont use yet </code>
4. build the executable using the configured preset. <br>
   In command prompt, run one command out of following:
	* <code>cmake --build --preset x64-debug </code>
	* <code>cmake --build --preset x86-debug </code>
	* <code>cmake --build --preset x64-release     # 🔴 builds errors, dont use yet </code>
	* <code>cmake --build --preset x86-release     # 🔴 builds errors, dont use yet </code>

#### *Configure and build - using Visual Studio (VS).*
0. prerequisites:
	* VS Installer -> components -> .NET 8.0 Runtime (LTSC)
	* VS Installer -> components -> .NET Framework 4.6.2 + 4.7.2 + 4.8 Targeting Pack
	* VS Installer -> components -> .NET Framework 4.8 SDK
	* VS Installer -> components -> C++ Make tools for Windows
	* VS Installer -> components -> MSBuild
	* VS Installer -> components -> MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
	* VS Installer -> components -> MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.36-17.6)
	* VS Installer -> components -> C++ core features
	* VS Installer -> components -> C++ ATL for latest v143 build tools (x64 & x86)
	* VS Installer -> components -> Windows 11 SDK (10.0.22621.0)
	* VS Installer -> components -> Windows Universal C Runtime
	* VS Installer -> components -> (other) GitHub copilot, CLR data types for SQL server,
	SQL Server Express 2019 LocalDB, Class Designer, ClickOnce Publishing, Code Map, DGML editor,
	NuGet package manager, NuGet targets and build tasks, Text Template transformation, vcpkg package manager,
	C# and Visual Basic Roslyn compilers, C++ 2022 Redistributable Update, C++ Build Insights, .NET profiling tools,
	C++ Address Sanitiser, C++ profiling tools, IntelliTrace, Just-in-Time debugger, Test Adapter for Boost.Test,
	C# and Visual Basic, F# language support, HLSL tools, IntelliCode, Windows Performance Toolkit.
1. check Visual Studio has: VS Installer -> components -> "C++ CMake tools for Windows" (100% must)
2. open project in VS with better CMake integration:
    1) projectDir -> Right Mouse Button click -> Open folder with Visual Studio
    2) after opening VS automatically runs CMakeLists.txt.
3. save CMakeLists.txt. <code>    Same as running:     cmake --preset ${CONFIG_NAME} </code>
4. after, launch .exe in VS. <code> Same as running:     cmake --build --preset ${CONFIG_NAME} </code>

---

#### *Note: former VS project configuration.*
```
Generate Visual Studio project using CMake -> open .sln file -> VisualStudio: Project: Settings ->
	General:
		Widows SDK version      = Windows 10.0 SDK,
		Platform Toolset        = Visual Studio 2022 (v143)
		C++ Language Standard   = ISO C++20 Standatd (/std:c++20)
		C Language Standard     = Default (Legacy MSVC)
	C++/General:
		Additional Include Directories  = $(ProjectDir)src\
	Linker/Input:
		Additional Dependencies	= $(CoreLibraryDependencies);%(AdditionalDependencies)   // default
```
