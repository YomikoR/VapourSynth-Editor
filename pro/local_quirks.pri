win32:contains(QMAKE_COMPILER, msvc) {

	VC_INCLUDEPATH += 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.14393.0/shared/'
	VC_INCLUDEPATH += 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.14393.0/um/'
	VC_INCLUDEPATH += 'C:/Program Files (x86)/Windows Kits/10/Include/10.0.14393.0/ucrt/'
	VC_INCLUDEPATH += 'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/include/'
	VC_INCLUDEPATH += 'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/include/'

	RC_INCLUDEPATH += $${VC_INCLUDEPATH}
	INCLUDEPATH += $${VC_INCLUDEPATH}

	contains(QMAKE_HOST.arch, x86_64) {
		QMAKE_LIBDIR += 'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/lib/amd64/'
		QMAKE_LIBDIR += 'C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64/'
		QMAKE_LIBDIR += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.14393.0/ucrt/x64/'
	} else {
		QMAKE_LIBDIR += 'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/lib/'
		QMAKE_LIBDIR += 'C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/'
		QMAKE_LIBDIR += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.14393.0/ucrt/x86'
	}

}
