# Most of this should be script
#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#  message(STATUS "test include dir='${dir}'")
#endforeach()

package_add_test(test-die-face test-die-face.cpp lib-dicekey)
# target_link_libraries(test-die-face lib-dicekey)

package_add_test(test-dicekey test-dicekey.cpp lib-dicekey)
#target_link_libraries(test-dicekey lib-dicekey)

# package_add_test_withmain(test-dicekey-file-inputs dicekey-file-inputs.cpp lib-dicekey)
