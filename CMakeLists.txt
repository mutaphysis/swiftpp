cmake_minimum_required( VERSION 3.2.2 )
project( swiftpp )

# Rely on llvm-config.
set( CONFIG_OUTPUT )
find_program( LLVM_CONFIG "llvm-config" PATHS /opt/llvm/bin )
if( LLVM_CONFIG )
	message( STATUS "Found LLVM_CONFIG as ${LLVM_CONFIG}" )
	execute_process(
			COMMAND ${LLVM_CONFIG} "--libdir" "--includedir" "--libs" core bitreader option mcparser instrumentation
			RESULT_VARIABLE HAD_ERROR
			OUTPUT_VARIABLE CONFIG_OUTPUT )
	if( NOT HAD_ERROR )
		string( REGEX REPLACE
				"[ \t]*[\r\n]+[ \t]*" ";"
				CONFIG_OUTPUT ${CONFIG_OUTPUT} )
	else()
		string( REPLACE ";" " " CONFIG_COMMAND_STR "${CONFIG_COMMAND}" )
		message( STATUS "${CONFIG_COMMAND_STR}" )
		message( FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}" )
	endif()
else()
	message( FATAL_ERROR "llvm-config not found -- ${LLVM_CONFIG}" )
endif()

list( GET CONFIG_OUTPUT 0 LIBRARY_DIR )
list( GET CONFIG_OUTPUT 1 INCLUDE_DIR )
list( GET CONFIG_OUTPUT 2 LIBS_LIST )

include_directories( "${INCLUDE_DIR}" )
link_directories( "${LIBRARY_DIR}" )

add_definitions( -std=c++11 -fno-rtti -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

add_executable ( swiftpp
					src/CodeTemplate.cpp
					src/CodeTemplate.h
					src/SwiftppASTConsumer.cpp
					src/SwiftppASTConsumer.h
					src/SwiftppASTVisitor.cpp
					src/SwiftppASTVisitor.h
					src/SwiftppData.cpp
					src/SwiftppData.h
					src/SwiftppObjcOutput.cpp
					src/SwiftppObjcOutput.h
					src/SwiftppObjcTemplates.h
					src/SwiftppOutput.cpp
					src/SwiftppOutput.h
					src/Version.h
					src/main.cpp
					src/substringref.cpp
					src/substringref.h )

target_link_libraries( swiftpp ${LIBS_LIST} -lclangBasic -lclangDriver -lclangEdit -lclangFrontend -lclangLex -lclangParse -lclangAST -lclangASTMatchers -lclangAnalysis -lclangSema -lclangSerialization -lclangTooling -lclangToolingCore -lz -lcurses )
