IMPORTANT

The original starter code is from ImageDisplay_C++_cross_platform at: https://github.com/Junyingw/CSCI-576-starter-code#

Clone
	After clonning this repository, `git submodule update --init --recursive` is needed.


Build and Run

Configuration:
	Machine: Apple Silicon Mac
	Chip: Apple M3 Maxs
	OS: Sequoia 15.7.3
	Architecture: arm64
	Compiler: gcc (Apple clang version 17.0.0)
	CMake: 4.2.3

5. Configure CMake

	Note: All vscode commands are run from the vscode command palette. To activate the Command Palette,
	Windows / Linux: Ctrl+Shift+P,
 	Mac:  Cmd+Shift+P

	Command Palette > Type in "CMake: Configure" and execute it.
	
	1. Find your compiler installation (gcc or clang) in the options that appear.
	   You may use the [scan for kits] option to scan the device for compiler installations.

	2. If prompted, Choose the correct location for CMakeLists
	   file - ${workspaceFolder}/ImageDisplay_C++_cross_platform/CMakeLists.txt

	At this point, the Configure command should execute and you may see some output in vscode output window.

6. Set up Configuration Provider

	Command Palette > Type in "C/C++: Change Configuration Provider" and run it.
	'CMake Tools' should be an option listed if the previous configure step was successful. Choose that.

	Command Palette > Type in "C/C++: Edit Configurations (UI)" and run it.
	This opens up a UI where you can edit the configurations. 
	
	1. Change compiler path and intellisense mode accordingly.
	2. Change C standard and C++ standard to c17 and c++17 respectively.

7. Build

	Command Palette > Type in "CMake: Set build target" and run it.
	Choose the target you want to build. You may choose MyImageApplication to create an executable
	with that name.

	Command Palette > Type in "CMake: Delete cache and reconfigure" and run it.
	This just cleans up the repository to prepare for a clean rebuild.

	Command Palette > Type in "CMake: Build" and run it.
	
	- Initial build might take some time, because the dependencies need to be built first,
	but later builds should be faster.
	- Rarely, you may want to run "CMake: Clean rebuild", but this would
	rebuild the whole project, including the dependencies, which may take a long time.


8. Run

	- In the /build folder, you will find the executable (MyImageApplication).
	- To run this, navigate to the build folder in a terminal and run the executable file.
	- The given starter code takes exactly one argument - a file path to a 512x512 rgb image file
	- This should be invoked as ./MyImageApplication '<path to rgb file>'
	- Example - ./MyImageApplication '../../Lena_512_512.rgb'

9. Rebuilds

	After making changes to the source code, you need to build the executable again.
	Command Palette > Type in "CMake: Build" and run it.

	On restarting vscode, you may need to configure CMake by running "CMake: Configure"
	After that, continue building using "CMake: Build".



For Linux users, this starter code is only tested on Ubuntu 22.04 with default gcc compiler.
For windows users, this starter code is only tested on Windows 10 with mingw-w64 gcc 12.2.0 compiler.
For mac users, this starter code is tested on an Apple Silicon mac, using Mac OS 13.4.1 with clang version 14.0.3 (clang-1403.0.22.14.1) compiler.

If you run into intellisense issues, we recommend the following steps.
	1. Command Palette > Type in "C/C++: Restart IntelliSense for Active File" and run it
	2. If it still doesn't work, you can try to reset the intellisense database.
	3. If still doesn't work, you can try to restart vscode.
	4. If still does not work, then you may have some problems in intellisense configuration.

From my perspective, choosing cmake as the intellisense configuration provider is the best choice
to setup intellisense to index dependency headers.