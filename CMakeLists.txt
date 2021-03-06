cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

if(POLICY CMP0044)
    cmake_policy(SET CMP0044 OLD)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "Setting build type to 'Release' as none was specified.")
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
ENDIF(NOT CMAKE_BUILD_TYPE)

message(STATUS "Build type CMAKE_BUILD_TYPE set to ${CMAKE_BUILD_TYPE}")

########################################
# Configuration                        #
########################################
project(FreeOrion)

include(GNUInstallDirs)

set(FreeOrion_VERSION 0.4.6+)

option(BUILD_TESTS "Controls generation of unit tests." OFF)

message(STATUS "Building tests BUILD_TESTS set to ${BUILD_TESTS}")
if (BUILD_TESTS)
  message( STATUS "Building Tests")
  enable_testing()
else()
  message( STATUS "Not building Tests")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(WIN32)
    set(CMAKE_INSTALL_BINDIR ".")
    set(FreeOrion_INSTALL_LIBDIR ".")
else()
    set(FreeOrion_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/freeorion")
endif()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${FreeOrion_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_compile_options(
    # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wall-316
    $<$<CXX_COMPILER_ID:gnu>:-Wall>
    # http://clang.llvm.org/docs/DiagnosticsReference.html#wall
    $<$<CXX_COMPILER_ID:clang>:-Wall>
    # https://msdn.microsoft.com/en-us/library/thxezb7y.aspx#Anchor_1
    $<$<CXX_COMPILER_ID:msvc>:/W4>
)

if (WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_DEPRECATE
        -D_SCL_SECURE_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_DEPRECATE
        -DFREEORION_WIN32
        -D_WIN32_WINNT=0x0501
    )
    if (MSVC)
        set (CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/../")
        set (BOOST_ROOT "${CMAKE_BINARY_DIR}/../Boost")
        set (CMAKE_STATIC_LIBRARY_PREFIX "lib")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
        set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    endif ()
elseif (APPLE)
    add_definitions(-DFREEORION_MACOSX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(
        -DFREEORION_LINUX
        -DENABLE_BINRELOC
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    add_definitions(
        -DFREEORION_FREEBSD
        -DENABLE_BINRELOC
        -DBOOST_DATE_TIME_NO_LOCALE
    )
else ()
    message(FATAL_ERROR "Unknown platform type! Your setup is not a supported platform for FreeOrion.")
endif ()

# Need to know if the boost libraries are linked statically or dynamically
# If dynamically, add some macro definitions to prevent potential link errors
option(FO_LINK_STATIC_BOOST_LIBS "Link static boost libraries." OFF)

if (NOT FO_LINK_STATIC_BOOST_LIBS)
    add_definitions(
        -DBOOST_ALL_NO_LINK
        -DBOOST_ALL_DYN_LINK
        -DBOOST_LOG_DYN_LINK
    )
endif ()


########################################
# Dependencies                         #
########################################
set(MINIMUM_PYTHON_VERSION 2.7)
set(MINIMUM_BOOST_VERSION 1.54.0)

find_package(Threads)
find_package(PythonInterp ${MINIMUM_PYTHON_VERSION} REQUIRED)
find_package(PythonLibs ${MINIMUM_PYTHON_VERSION} REQUIRED)
find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        chrono
        date_time
        filesystem
        iostreams
        locale
        log
        regex
        serialization
        python
        signals
        system
        thread
    REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Freetype REQUIRED)


########################################
# Build common code library            #
########################################

# To run the version generation every compile we need to deferr the
# execution to a separate target and the existing python command
add_custom_target(freeorionversion
    COMMAND
    "${PYTHON_EXECUTABLE}"
    "${CMAKE_SOURCE_DIR}/cmake/make_versioncpp.py"
    "${CMAKE_SOURCE_DIR}"
    "CMake"
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/util/Version.cpp
    PROPERTIES
    GENERATED TRUE
)

set(BUILD_DEVEL_PACKAGE OFF CACHE INTERNAL "Disables installation of GiGi development files." FORCE)
set(_ORIG_CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_LIBDIR "${FreeOrion_INSTALL_LIBDIR}")
add_subdirectory(GG)
set(CMAKE_INSTALL_LIBDIR ${_ORIG_CMAKE_INSTALL_LIBDIR})
unset(_ORIG_CMAKE_INSTALL_LIBDIR)

if(${Boost_VERSION} LESS "105600")
    # Starting with boost 1.56 boost::spirit uses boost::phoenix v3 exclusively.
    # Before that version there spirit provides an translation layer, which can
    # be enabled by defining BOOST_SPIRIT_USE_PHOENIX_V3.
    add_definitions(
        -DBOOST_SPIRIT_USE_PHOENIX_V3
    )
endif()

if(${Boost_VERSION} EQUAL "106100" OR ${Boost_VERSION} GREATER "106100")
    # with boost 1.61 some boost::optional internals were changed. However
    # boost::spirit relies on some API the old implementation provided.  This
    # define enables the usage of the old boost::optional implementation.
    # boost upstream tracks this bug as #12349
    add_definitions(
        -DBOOST_OPTIONAL_CONFIG_USE_OLD_IMPLEMENTATION_OF_OPTIONAL
    )
endif()

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    GG
)

link_directories(
    ${Boost_LIBRARY_DIRS}
)

set (freeorioncommon_HEADER
    combat/CombatEvent.h
    combat/CombatEvents.h
    combat/CombatLogManager.h
    combat/CombatSystem.h
    Empire/Diplomacy.h
    Empire/Empire.h
    Empire/EmpireManager.h
    Empire/ResourcePool.h
    Empire/Supply.h
    network/Message.h
    network/MessageQueue.h
    network/Networking.h
    python/SetWrapper.h
    python/CommonWrappers.h
    universe/Building.h
    universe/Condition.h
    universe/EffectAccounting.h
    universe/Effect.h
    universe/Encyclopedia.h
    universe/Enums.h
    universe/Field.h
    universe/Fighter.h
    universe/Fleet.h
    universe/Meter.h
    universe/ObjectMap.h
    universe/Planet.h
    universe/PopCenter.h
    universe/Predicates.h
    universe/ResourceCenter.h
    universe/ShipDesign.h
    universe/Ship.h
    universe/Special.h
    universe/Species.h
    universe/System.h
    universe/Tech.h
    universe/Universe.h
    universe/UniverseObject.h
    universe/TemporaryPtr.h
    universe/ValueRef.h
    universe/ValueRefFwd.h
    util/AppInterface.h
    util/blocking_combiner.h
    util/Directories.h
    util/EnumText.h
    util/i18n.h
    util/Logger.h
    util/ModeratorAction.h
    util/MultiplayerCommon.h
    util/OptionsDB.h
    util/OptionValidators.h
    util/Order.h
    util/OrderSet.h
    util/Process.h
    util/Random.h
    util/SaveGamePreviewUtils.h
    util/ScopedTimer.h
    util/Serialize.h
    util/Serialize.ipp
    util/SitRepEntry.h
    util/StringTable.h
    util/VarText.h
    util/Version.h
    util/XMLDoc.h
)

set (freeorioncommon_SOURCE
    combat/CombatEvent.cpp
    combat/CombatEvents.cpp
    combat/CombatLogManager.cpp
    Empire/Diplomacy.cpp
    Empire/Empire.cpp
    Empire/EmpireManager.cpp
    Empire/ResourcePool.cpp
    Empire/Supply.cpp
    network/Message.cpp
    network/MessageQueue.cpp
    network/Networking.cpp
    universe/Building.cpp
    universe/Condition.cpp
    universe/EffectAccounting.cpp
    universe/Effect.cpp
    universe/Encyclopedia.cpp
    universe/Enums.cpp
    universe/Field.cpp
    universe/Fighter.cpp
    universe/Fleet.cpp
    universe/Meter.cpp
    universe/ObjectMap.cpp
    universe/Planet.cpp
    universe/PopCenter.cpp
    universe/Predicates.cpp
    universe/ResourceCenter.cpp
    universe/Ship.cpp
    universe/ShipDesign.cpp
    universe/Special.cpp
    universe/Species.cpp
    universe/System.cpp
    universe/Tech.cpp
    universe/Universe.cpp
    universe/UniverseObject.cpp
    universe/ValueRef.cpp
    util/AppInterface.cpp
    util/Directories.cpp
    util/EnumText.cpp
    util/i18n.cpp
    util/Logger.cpp
    util/ModeratorAction.cpp
    util/MultiplayerCommon.cpp
    util/OptionsDB.cpp
    util/Order.cpp
    util/OrderSet.cpp
    util/Process.cpp
    util/Random.cpp
    util/SaveGamePreviewUtils.cpp
    util/ScopedTimer.cpp
    util/SerializeEmpire.cpp
    util/SerializeModeratorAction.cpp
    util/SerializeMultiplayerCommon.cpp
    util/SerializeOrderSet.cpp
    util/SerializeUniverse.cpp
    util/SitRepEntry.cpp
    util/StringTable.cpp
    util/VarText.cpp
    util/Version.cpp
    util/XMLDoc.cpp
)

if (NOT WIN32)
    list(APPEND freeorioncommon_SOURCE
        util/binreloc.c
    )
endif ()

add_library(freeorioncommon
    ${freeorioncommon_HEADER}
    ${freeorioncommon_SOURCE}
)

target_compile_options(
    freeorioncommon PRIVATE
    $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
    $<$<CXX_COMPILER_ID:clang>:-ftemplate-depth=512>
)

add_dependencies(freeorioncommon freeorionversion)

target_link_libraries(freeorioncommon
    ${Boost_CHRONO_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_LOG_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_SIGNALS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${ZLIB_LIBRARIES}
)

set_target_properties(freeorioncommon
    PROPERTIES
    DEFINE_SYMBOL "FREEORION_BUILD_COMMON"
)

########################################
# Recurse Into Sources                 #
########################################
add_subdirectory(parse)
add_subdirectory(server)
add_subdirectory(client/AI)
add_subdirectory(client/human)
add_subdirectory(doc)

########################################
# Packaging                            #
########################################
set(BUILD_PACKAGING
    false
    CACHE BOOL
    "Whether to build installer/binary package. (This is the executable installer on Win32, and an RPM/DEB/tarball on Linux."
)

install(
    DIRECTORY default/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/freeorion/default
    COMPONENT COMPONENT_FREEORION_DATA
    PATTERN "*~" EXCLUDE
    PATTERN "*.pyc" EXCLUDE
)

install(
    TARGETS freeorioncommon
    LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)

install(
    FILES
    ${CMAKE_SOURCE_DIR}/freeorion.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    COMPONENT COMPONENT_FREEORION
)

foreach(SIZE 16 24 32 64 128 256)
    install(
        FILES
        ${CMAKE_SOURCE_DIR}/default/data/art/icons/FO_Icon_${SIZE}x${SIZE}.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${SIZE}x${SIZE}/apps/
        RENAME freeorion.png
        COMPONENT COMPONENT_FREEORION
    )
endforeach()

if (WIN32)
    install(
        FILES
        OpenAL32.dll
        boost_date_time-vc90-mt-1_44.dll
        boost_filesystem-vc90-mt-1_44.dll
        boost_iostreams-vc90-mt-1_44.dll
        boost_python-vc90-mt-1_44.dll
        boost_regex-vc90-mt-1_44.dll
        boost_serialization-vc90-mt-1_44.dll
        boost_signals-vc90-mt-1_44.dll
        boost_system-vc90-mt-1_44.dll
        boost_thread-vc90-mt-1_44.dll
        glew32.dll
        libexpat.dll
        libogg.dll
        libpng13.dll
        libvorbis.dll
        libvorbisfile.dll
        python26.dll
        wrap_oal.dll
        z.dll
        zlib1.dll
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT COMPONENT_FREEORION_WIN32_RUNTIME_DEPS
    )
endif ()

set(CPACK_PACKAGE_VERSION ${FreeOrion_VERSION})

set(CPACK_PACKAGE_VENDOR "FreeOrion Community")
set(CPACK_PACKAGE_CONTACT http://freeorion.org/forum)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FreeOrion is a free, open source, turn-based space empire and galactic conquest (4X) computer game being designed and built by the FreeOrion project. FreeOrion is inspired by the tradition of the Master of Orion games, but is not a clone or remake of that series or any other game.")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_HOME_DIRECTORY}/default/COPYING)
if (BUILD_PACKAGING)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
    if (WIN32)
        set(PACKAGE_FILE_SYSTEM_NAME win32)
    else ()
        set(PACKAGE_FILE_SYSTEM_NAME ${CPACK_SYSTEM_NAME})
    endif ()
    set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${PACKAGE_FILE_SYSTEM_NAME}")
    string(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)
    set(CPACK_MONOLITHIC_INSTALL ON)
endif ()

# NSIS-specific settings
set(CPACK_NSIS_COMPRESSOR bzip2)
set(CPACK_NSIS_URL_INFO_ABOUT http://freeorion.org)
string(REPLACE "/" "\\\\" cmake_home_directory_with_native_windows_path_separators ${CMAKE_HOME_DIRECTORY})

# RPM-specific settings
set(CPACK_RPM_PACKAGE_LICENSE GPL)
set(CPACK_RPM_PACKAGE_REQUIRES)
# TODO

# Deb-specific settings
set(CPACK_DEBIAN_PACKAGE_SECTION games)
# TODO

if (UNIX)
    set(CPACK_GENERATOR "TGZ;TBZ2")
    find_program(RPMBUILD rpmbuild)
    if (RPMBUILD)
        list(APPEND CPACK_GENERATOR RPM)
    endif ()
    set(RPMBUILD ${RPMBUILD} CACHE INTERNAL "")
    find_program(DPKG dpkg)
    if (DPKG)
        list(APPEND CPACK_GENERATOR DEB)
    endif ()
    set(DPKG ${DPKG} CACHE INTERNAL "")
elseif (WIN32)
    set(CPACK_GENERATOR "NSIS")
endif ()


########################################
# Source Packaging                     #
########################################

if (UNIX)
    set(CPACK_SOURCE_GENERATOR "TGZ")
elseif (WIN32)
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif ()

set(CPACK_SOURCE_IGNORE_FILES
    "~$"
    "\\\\.asm$"
    "\\\\.bz2$"
    "/CMakeCache\\\\.txt$"
    "/CMakeFiles/"
    "/cmake_install\\\\.cmake$"
    "/CPackConfig.cmake$"
    "/_CPack_Packages/"
    "/CPackSourceConfig.cmake$"
    "/CTestTestfile\\\\.cmake$"
    "\\\\.dll$"
    "\\\\.exe$"
    "\\\\.exp$"
    "/freeorion$"
    "/freeorionca$"
    "/freeoriond$"
    "GG/GG/Config.h$"
    "\\\\.git/"
    "\\\\.gz$"
    "\\\\.lib$"
    "/Makefile$"
    "\\\\.pc$"
    "\\\\.pdb$"
    "\\\\.pyc$"
    "/Release/"
    "\\\\.so$"
    "\\\\.swp$"
)

set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-v${FreeOrion_VERSION}-${FreeOrion_WC_REVISION}-source"
)

include(CPack)

if (WIN32)
    cpack_add_component(
        COMPONENT_FREEORION_WIN32_RUNTIME_DEPS
        DISPLAY_NAME FreeOrion Win32 Runtime Dependencies
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )
endif ()

cpack_add_component(
    COMPONENT_FREEORION_DATA
    DISPLAY_NAME FreeOrion Data
    DESCRIPTION ""
    HIDDEN
    REQUIRED
)

cpack_add_component(
    COMPONENT_FREEORION
    DISPLAY_NAME FreeOrion Client
    DESCRIPTION ""
    HIDDEN
    REQUIRED
)

cpack_add_component(
    COMPONENT_FREEORIOND
    DISPLAY_NAME FreeOrion Server
    DESCRIPTION ""
    HIDDEN
    REQUIRED
)

cpack_add_component(
    COMPONENT_FREEORIONCA
    DISPLAY_NAME FreeOrion AI
    DESCRIPTION ""
    HIDDEN
    REQUIRED
)

if (BUILD_TESTS)
    add_subdirectory(test EXCLUDE_FROM_ALL)
endif ()
