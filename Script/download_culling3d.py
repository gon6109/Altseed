
import aceutils

aceutils.cdToScript()

with aceutils.CurrentDir('../Downloads'):
	aceutils.rm(r'master.zip')
	aceutils.rmdir(r'Culling3D-master')
	aceutils.rmdir(r"culling3d_bin")
	aceutils.rmdir(r"culling3d_bin_x64")

	aceutils.wget(r'https://github.com/altseed/Culling3D/archive/master.zip')
	aceutils.unzip(r'master.zip')
	aceutils.editCmakeForACE(r'Culling3D-master/CMakeLists.txt','cp932')
	aceutils.mkdir(r"culling3d_bin")
	aceutils.mkdir(r"culling3d_bin_x64")

	with aceutils.CurrentDir('culling3d_bin'):
		if aceutils.isWin():
			aceutils.call(aceutils.cmd_cmake+r'-D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF -D USE_INTERNAL_LOADER:BOOL=OFF ../Culling3D-master/')
			aceutils.call(aceutils.cmd_compile + r'Culling3D.sln /p:configuration=Debug')
			aceutils.call(aceutils.cmd_compile + r'Culling3D.sln /p:configuration=Release')
		elif aceutils.isMac():
			aceutils.call(r'cmake -G "Unix Makefiles" -D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF -D USE_INTERNAL_LOADER:BOOL=OFF "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386" ../Culling3D-master/')
			aceutils.call(r'make')
		else:
			aceutils.call(r'cmake -G "Unix Makefiles" -D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF -D USE_INTERNAL_LOADER:BOOL=OFF ../Culling3D-master/')
			aceutils.call(r'make')

	with aceutils.CurrentDir('culling3d_bin_x64'):
		if aceutils.isWin():
			aceutils.call(aceutils.cmd_cmake_x64+r'-D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF -D USE_INTERNAL_LOADER:BOOL=OFF ../Culling3D-master/')
			aceutils.call(aceutils.cmd_compile + r'Culling3D.sln /p:configuration=Debug')
			aceutils.call(aceutils.cmd_compile + r'Culling3D.sln /p:configuration=Release')

	if aceutils.isWin():
		aceutils.mkdir(r'../Dev/lib/x86/')
		aceutils.mkdir(r'../Dev/lib/x86/Debug')
		aceutils.mkdir(r'../Dev/lib/x86/Release')

		aceutils.mkdir(r'../Dev/lib/x64/')
		aceutils.mkdir(r'../Dev/lib/x64/Debug')
		aceutils.mkdir(r'../Dev/lib/x64/Release')

		aceutils.copy(r'Culling3D-master/Culling3D/Culling3D.h', r'../Dev/include/')
		
		aceutils.copy(r'culling3d_bin/Debug/Culling3D.lib', r'../Dev/lib/x86/Debug/')
		aceutils.copy(r'culling3d_bin/Release/Culling3D.lib', r'../Dev/lib/x86/Release/')

		aceutils.copy(r'culling3d_bin_x64/Debug/Culling3D.lib', r'../Dev/lib/x64/Debug/')
		aceutils.copy(r'culling3d_bin_x64/Release/Culling3D.lib', r'../Dev/lib/x64/Release/')

	else:
		aceutils.copy(r'Culling3D-master/Culling3D/Culling3D.h', r'../Dev/include/')
		aceutils.copy(r'culling3d_bin/libCulling3D.a', r'../Dev/lib/')
