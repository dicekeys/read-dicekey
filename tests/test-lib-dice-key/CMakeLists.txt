file(GLOB SRCS *.cpp)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "test include dir='${dir}'")
endforeach()

add_executable(test-lib-dice-key ${SRCS})

target_link_libraries(test-lib-dice-key
	lib-dice-key
  gtest
  gmock
)
#     gtest  gmock

install(TARGETS test-lib-dice-key  DESTINATION bin)

add_test(NAME test-lib-dice-key COMMAND test-lib-dice-key)
set_target_properties(test-lib-dice-key PROPERTIES FOLDER tests)