INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ System)

### Spring defines
SET(USE_MMGR FALSE CACHE BOOL "Use memory manager?")
if (USE_MMGR)
	ADD_DEFINITIONS(-DUSE_MMGR)
endif (USE_MMGR)

SET(USE_GML FALSE CACHE BOOL "Use the GML OpenGL threading library?")
if (USE_GML)
	ADD_DEFINITIONS(-DUSE_GML)
endif (USE_GML)

SET(TRACE_SYNC FALSE CACHE BOOL "Enable sync tracker")
if (TRACE_SYNC)
	ADD_DEFINITIONS(-DTRACE_SYNC)
endif (TRACE_SYNC)

SET(SYNCDEBUG FALSE CACHE BOOL "Enable sync debugger (needs SYNCCHECK=true)")
if (SYNCDEBUG)
	ADD_DEFINITIONS(-DSYNCDEBUG)
endif (SYNCDEBUG)

### Find include directories and add platform specific libraries
IF (MINGW)
	FIND_PACKAGE(Win32Libs REQUIRED)
	LIST(APPEND spring_libraries glu32 opengl32 ${WIN32_LIBRARIES} glew32 mingw32)
	#TODO make FindLibraryX work with mingwlibs
	set (VORBISFILE_LIBRARY vorbisfile)
	set(VORBIS_LIBRARY vorbis)
	set (OGG_LIBRARY ogg)
	set(FREETYPE_LIBRARIES freetype)
ELSE (MINGW)
	FIND_PACKAGE(X11 REQUIRED)
	FIND_PACKAGE(OpenAL REQUIRED)
	if (NOT OPENAL_FOUND)
		message (SEND_ERROR "OpenAL libraries not found")
	endif (NOT OPENAL_FOUND)
	FIND_PACKAGE(OpenGL REQUIRED)
	FIND_PACKAGE(GLU REQUIRED)
	FIND_PACKAGE(GLUT REQUIRED)
	FIND_PACKAGE(GLEW REQUIRED)
	FIND_PACKAGE(OggVorbis REQUIRED)

	FIND_PACKAGE(Freetype REQUIRED)
	INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIR})
	LIST(APPEND spring_libraries ${OPENAL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${X11_X11_LIB} ${X11_Xcursor_LIB} ${GLEW_LIBRARIES})
ENDIF (MINGW)

### libraries needed on all platforms
LIST(APPEND spring_libraries  ${SDL_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_THREAD_LIBRARY} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY} ${FREETYPE_LIBRARIES} ${DEVIL_LIBRARIES} )

### build all libraries in lib (has its own CMakeLists.txt)
ADD_SUBDIRECTORY(lib)
LIST(APPEND spring_libraries lua 7zip hpiutil2 minizip streflop luabind)
if (USE_GML)
	list (APPEND spring_libraries gml)
endif (USE_GML)

### include directories needed for compiling spring
INCLUDE_DIRECTORIES(BEFORE lib/lua/include lib/luabind)

### all spring source directories
### each directorie which contains source has to be here
AUX_SOURCE_DIRECTORY(Game gamefiles)
AUX_SOURCE_DIRECTORY(Game/Camera gamefiles)
AUX_SOURCE_DIRECTORY(Game/Server gamefiles)
AUX_SOURCE_DIRECTORY(Game/StartScripts gamefiles)
AUX_SOURCE_DIRECTORY(Game/UI gamefiles)

AUX_SOURCE_DIRECTORY(Lua luafiles)

AUX_SOURCE_DIRECTORY(ExternalAI aifiles)
AUX_SOURCE_DIRECTORY(ExternalAI/Interface aifiles)

AUX_SOURCE_DIRECTORY(Map mapfiles)
AUX_SOURCE_DIRECTORY(Map/SM3 mapfiles)
AUX_SOURCE_DIRECTORY(Map/SM3/terrain mapfiles)
AUX_SOURCE_DIRECTORY(Map/SMF mapfiles)

AUX_SOURCE_DIRECTORY(Rendering renderfiles)
AUX_SOURCE_DIRECTORY(Rendering/Env renderfiles)
AUX_SOURCE_DIRECTORY(Rendering/GL renderfiles)
AUX_SOURCE_DIRECTORY(Rendering/Textures renderfiles)
AUX_SOURCE_DIRECTORY(Rendering/UnitModels renderfiles)

AUX_SOURCE_DIRECTORY(Sim simfiles)
AUX_SOURCE_DIRECTORY(Sim/Features simfiles)
AUX_SOURCE_DIRECTORY(Sim/Misc simfiles)
AUX_SOURCE_DIRECTORY(Sim/MoveTypes simfiles)
AUX_SOURCE_DIRECTORY(Sim/MoveTypes/MoveMath simfiles)
AUX_SOURCE_DIRECTORY(Sim/Objects simfiles)
AUX_SOURCE_DIRECTORY(Sim/Path simfiles)
AUX_SOURCE_DIRECTORY(Sim/Projectiles simfiles)
AUX_SOURCE_DIRECTORY(Sim/Projectiles/WeaponProjectiles simfiles)
AUX_SOURCE_DIRECTORY(Sim/Projectiles/Unsynced simfiles)
AUX_SOURCE_DIRECTORY(Sim/Units simfiles)
AUX_SOURCE_DIRECTORY(Sim/Units/COB simfiles)
AUX_SOURCE_DIRECTORY(Sim/Units/CommandAI simfiles)
AUX_SOURCE_DIRECTORY(Sim/Units/UnitTypes simfiles)
AUX_SOURCE_DIRECTORY(Sim/Weapons simfiles)

AUX_SOURCE_DIRECTORY(System sysfiles)
AUX_SOURCE_DIRECTORY(System/Platform sysfiles)
### only use the directory for target platform
IF (UNIX)
	AUX_SOURCE_DIRECTORY(System/Platform/Linux sysfiles)
ELSE (UNIX)
	AUX_SOURCE_DIRECTORY(System/Platform/Win sysfiles)
ENDIF (UNIX)
AUX_SOURCE_DIRECTORY(System/FileSystem sysfiles)
AUX_SOURCE_DIRECTORY(System/Net sysfiles)
AUX_SOURCE_DIRECTORY(System/Script sysfiles)
AUX_SOURCE_DIRECTORY(System/Sync sysfiles)
AUX_SOURCE_DIRECTORY(System/creg sysfiles)

### Add icon and manifest to exe using windres
IF (MINGW)
	FIND_PROGRAM(WINDRES NAMES windres i686-mingw32-windres DOC "path to mingw's windres executable")
	ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o COMMAND ${WINDRES} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/icon.rc -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
	LIST(APPEND sysfiles icon.o)
ENDIF (MINGW)

ADD_EXECUTABLE(spring ${gamefiles} ${luafiles} ${mapfiles} ${fsfiles} ${renderfiles} ${simfiles} ${sysfiles} ${aifiles})

TARGET_LINK_LIBRARIES(spring ${spring_libraries})
install (TARGETS spring DESTINATION ${BINDIR})
