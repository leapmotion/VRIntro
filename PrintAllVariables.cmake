# A simple script that prints all variables currently declared in this execution of cmake.
# Taken from sakra's answer at http://stackoverflow.com/questions/9298278/cmake-print-out-all-accessible-variables-in-a-script

get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

