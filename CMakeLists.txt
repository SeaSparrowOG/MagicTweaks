cmake_minimum_required(VERSION 3.24)

option(FULL_RELEASE "Finalizes the build, compressing everything into a BSA." OFF)

# -------- Project ----------
project(
	MagicTweaks
	VERSION 1.2.0
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(External/CLibHelper)
include_directories(External/CLibHelper)

get_filename_component(
	Skyrim64Path
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Bethesda Softworks\\Skyrim Special Edition;installed path]"
	ABSOLUTE CACHE
)

list(APPEND CMAKE_IGNORE_PATH "${PROJECT_BINARY_DIR}")
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules"
)
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/Bethesda-CMakeModules/Modules"
)
include(SKSEPlugin)
set(SKSE_USE_XBYAK ON)
add_compile_definitions(SKSE_SUPPORT_XBYAK)

SKSEPlugin_Add(
	${PROJECT_NAME}
	SOURCE_DIR src
	SOURCES
		README.md
		vcpkg.json
	PRECOMPILE_HEADERS
		src/common/PCH.h
)

find_package(jsoncpp CONFIG REQUIRED)

target_link_libraries(
		"${PROJECT_NAME}"
		PRIVATE
			JsonCpp::JsonCpp
)

set_target_properties(CommonLibSSE PROPERTIES
	FOLDER External
)

set(DATA_DIR "${PROJECT_SOURCE_DIR}/data")

include(components.cmake)

# ---- Post build ----
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
	if(FULL_RELEASE)
		set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
	else()
		set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME} - Test")
	endif()
endif()

# ---- Compile Papyrus Scripts and Pack Into Archive ----
# include(Papyrus)
# include(BSArchive)

#file(GLOB ${PROJECT_NAME}_SOURCES
#	LIST_DIRECTORIES false
#	CONFIGURE_DEPENDS
#	"Data/Source/Scripts/*.psc"
#)

#Papyrus_Add(
#	"Papyrus"
#	GAME ${Skyrim64Path}
#	IMPORTS ${CMAKE_CURRENT_SOURCE_DIR}/Data/Source/Scripts
#	SOURCES ${${PROJECT_NAME}_SOURCES}
#	OPTIMIZE ANONYMIZE
#)

#BSArchive_Add(
#	"BSA"
#	OUTPUT "${PROJECT_NAME}.bsa"
#	FORMAT SSE
#	FILES ${Papyrus_OUTPUT}
#)

# ---- Automatically copy artifacts to MO2 ----
if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"

		# Clean Config Directory
		COMMAND "${CMAKE_COMMAND}" -E remove_directory "${OUTPUT_FOLDER}/SKSE/Plugins/${PROJECT_NAME}"
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${OUTPUT_FOLDER}/SKSE/Plugins/${PROJECT_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory 
			"${PROJECT_SOURCE_DIR}/Data/SKSE/Plugins/${PROJECT_NAME}"
			"${OUTPUT_FOLDER}/SKSE/Plugins/${PROJECT_NAME}"

		# Update Settings INI
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			"${PROJECT_SOURCE_DIR}/Data/SKSE/Plugins/${PROJECT_NAME}.ini"
			"${OUTPUT_FOLDER}/SKSE/Plugins/${PROJECT_NAME}.ini"

		# Update ESP plugin and the corresponding archive
#		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
#			"${PROJECT_SOURCE_DIR}/Data/${PROJECT_NAME}.esp"
#			"${OUTPUT_FOLDER}/${PROJECT_NAME}.esp"
#		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bsa" "${OUTPUT_FOLDER}/${PROJECT_NAME}.bsa"
        VERBATIM
    )

	# Loose scripts for non-release builds (why? Because I said so smile)
#    if(NOT FULL_RELEASE)
#        add_custom_command(
#            TARGET "${PROJECT_NAME}"
#            POST_BUILD
#			COMMAND "${CMAKE_COMMAND}" -E remove_directory "${OUTPUT_FOLDER}/Source"
#			COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/Data/Source/Scripts" "${OUTPUT_FOLDER}/Source/Scripts"

#            COMMAND "${CMAKE_COMMAND}" -E remove_directory "${OUTPUT_FOLDER}/Scripts"
#            COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_BINARY_DIR}/Scripts" "${OUTPUT_FOLDER}/Scripts"
#            VERBATIM
#        )
#    endif()
endif()
