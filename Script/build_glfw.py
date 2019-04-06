import aceutils

pathname = 'glfw'

aceutils.cdToScript()
aceutils.mkdir('../Downloads')

with aceutils.CurrentDir('../Downloads'):
	aceutils.rmdir(r"glfw_bin")
	aceutils.rmdir(r"glfw_bin_x64")

	aceutils.editCmakeForACE(pathname + r'/CMakeLists.txt')
	aceutils.mkdir(r"glfw_bin")
	aceutils.mkdir(r"glfw_bin_x64")

	with aceutils.CurrentDir('glfw_bin'):
		if aceutils.isWin():
			aceutils.call(aceutils.cmd_cmake+r'-D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF ../' + pathname + '/')
			aceutils.call('cmake --build . --config Debug')
			aceutils.call('cmake --build . --config Release')
		elif aceutils.isMac():
			aceutils.call(r'cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D BUILD_SHARED_LIBS:BOOL=OFF -D CMAKE_INSTALL_PREFIX:PATH=../../Dev "-DCMAKE_OSX_ARCHITECTURES=x86_64' + (';i386' if aceutils.Isi386() else '') + r'" ../' + pathname + '/')
			aceutils.call(r'make install')
		else:
			aceutils.call(r'cmake -G "Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D BUILD_SHARED_LIBS:BOOL=OFF -D CMAKE_INSTALL_PREFIX=../../Dev ../' + pathname + '/')
			aceutils.call(r'make install')

	with aceutils.CurrentDir('glfw_bin_x64'):
		if aceutils.isWin():
			aceutils.call(aceutils.cmd_cmake_x64+r'-D USE_MSVC_RUNTIME_LIBRARY_DLL:BOOL=OFF ../' + pathname + '/')
			aceutils.call('cmake --build . --config Debug')
			aceutils.call('cmake --build . --config Release')
	
	aceutils.mkdir(r"../Dev/include/GLFW/")

	if aceutils.isWin():

		aceutils.mkdir(r'../Dev/lib/x86/')
		aceutils.mkdir(r'../Dev/lib/x86/Debug')
		aceutils.mkdir(r'../Dev/lib/x86/Release')

		aceutils.mkdir(r'../Dev/lib/x64/')
		aceutils.mkdir(r'../Dev/lib/x64/Debug')
		aceutils.mkdir(r'../Dev/lib/x64/Release')

		aceutils.copy(pathname + r'/include/GLFW/glfw3.h', r'../Dev/include/GLFW/')
		aceutils.copy(pathname + r'/include/GLFW/glfw3native.h', r'../Dev/include/GLFW')

		aceutils.copy(r'glfw_bin/src/Debug/glfw3.lib', r'../Dev/lib/x86/Debug/')
		aceutils.copy(r'glfw_bin/src/Release/glfw3.lib', r'../Dev/lib/x86/Release/')

		aceutils.copy(r'glfw_bin_x64/src/Debug/glfw3.lib', r'../Dev/lib/x64/Debug/')
		aceutils.copy(r'glfw_bin_x64/src/Release/glfw3.lib', r'../Dev/lib/x64/Release/')

	else:
		aceutils.copy(pathname + r'/include/GLFW/glfw3.h', r'../Dev/include/GLFW/')
		aceutils.copy(pathname + r'/include/GLFW/glfw3native.h', r'../Dev/include/GLFW')

		aceutils.copy(r'glfw_bin/src/libglfw3.a', r'../Dev/lib')
