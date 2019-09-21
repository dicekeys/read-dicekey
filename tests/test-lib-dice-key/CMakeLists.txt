# Most of this should be script
#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#  message(STATUS "test include dir='${dir}'")
#endforeach()

package_add_test(test-die-face test-die-face.cpp)
target_link_libraries(test-die-face lib-dice-key)

package_add_test(test-dice-key test-dice-key.cpp)
target_link_libraries(test-dice-key lib-dice-key)
