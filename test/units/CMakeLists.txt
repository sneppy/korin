### Setup unit tests

set(UNITS

	"templates"
	"memory"
	"math"
	"containers"
	"regex"
)

list(LENGTH UNIT NUM_UNITS)

## Create and build all units
foreach(UNIT ${UNITS})
	
	set(PRIVATE_FILE "test_${UNIT}.cpp")
	set(PUBLIC_FILE "test_${UNIT}.h")
	
	## Create private file if necessary
	get_filename_component(PRIVATE_FILE ${PRIVATE_FILE} ABSOLUTE)
	if(NOT EXISTS ${PRIVATE_FILE})
	
		file(WRITE ${PRIVATE_FILE}
		
			"#include \"test_${UNIT}.h\"\n"
			"\n"
			"int main(int argc, char ** argv)\n"
			"{\n"
			"\ttesting::InitGoogleTest(&argc, argv);\n"
			"\treturn RUN_ALL_TESTS();\n"
			"}\n"
		)
		
	endif(NOT EXISTS ${PRIVATE_FILE})
	
	## Same for public file
	get_filename_component(PUBLIC_FILE ${PUBLIC_FILE} ABSOLUTE)
	if(NOT EXISTS ${PUBLIC_FILE})
	
		file(WRITE ${PUBLIC_FILE}
		
			"#pragma once\n"
			"\n"
			"#include \"gtest/gtest.h\"\n"
		)
		
	endif(NOT EXISTS ${PUBLIC_FILE})

	## Create executable
	set(TARGET_NAME ${PROJECT_NAME}_test-${UNIT})
	add_executable(${TARGET_NAME}
	
		${PUBLIC_FILE}
		${PRIVATE_FILE}
	)

	## Link SGL library
	target_link_libraries(${TARGET_NAME}

		${PROJECT_NAME}
		gtest_main
		${CMAKE_THREAD_LIBS_INIT}
	)

	## Create make target
	add_custom_target(test_${UNIT}

		COMMAND				$<TARGET_FILE:${TARGET_NAME}>
		DEPENDS				${TARGET_NAME}
		WORKING_DIRECTORY	${PROJECT_SOURCE_DIR}
	)

endforeach()

### Create target to run all test units
	
set(PRIVATE_FILE "test_all.gen.cpp")
set(PUBLIC_FILE "test_all.gen.h")

## Generate files that includes all unit tests
get_filename_component(PRIVATE_FILE ${PRIVATE_FILE} ABSOLUTE)
file(WRITE ${PRIVATE_FILE}

	"/*================================================\n"
	"	This file was automatically generated by CMake\n"
	"================================================*/\n"
	"\n"
	"#include \"${PUBLIC_FILE}\"\n"
	"\n"
	"int main(int argc, char ** argv)\n"
	"{\n"
	"\ttesting::InitGoogleTest(&argc, argv);\n"
	"\treturn RUN_ALL_TESTS();\n"
	"}\n"
)

get_filename_component(PUBLIC_FILE ${PUBLIC_FILE} ABSOLUTE)
file(WRITE ${PUBLIC_FILE}

	"/*================================================\n"
	"	This file was automatically generated by CMake\n"
	"================================================*/\n"
	"\n"
	"#pragma once\n"
	"\n"
	"#include \"gtest/gtest.h\"\n"
	"\n"
)

## Append all test units public files
foreach(UNIT ${UNITS})
	
	set(UNIT_PUBLIC_FILE "test_${UNIT}.h")
	list(APPEND UNIT_PUBLIC_FILES ${UNIT_PUBLIC_FILE}) # Watch for changes
	file(APPEND ${PUBLIC_FILE}
	
		"#include \"${UNIT_PUBLIC_FILE}\"\n"
	)

endforeach()

## Create executable for all units
set(TARGET_NAME ${PROJECT_NAME}_test-all)
add_executable(${TARGET_NAME}

	${PRIVATE_FILE}
	${PUBLIC_FILE}
	${UNIT_PUBLIC_FILES}
)

## Link SGL library
target_link_libraries(${TARGET_NAME}

	${PROJECT_NAME}
	gtest_main
	${CMAKE_THREAD_LIBS_INIT}
)

## Create make target
add_custom_target(test_all

	COMMAND				$<TARGET_FILE:${TARGET_NAME}>
	DEPENDS				${TARGET_NAME}
	WORKING_DIRECTORY	${PROJECT_SOURCE_DIR}
)
