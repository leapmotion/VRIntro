include(CreateImportTargetHelpers)

list(APPEND CMAKE_INCLUDE_PATH ${EXTERNAL_LIBRARY_DIR}/python2.7/include)
list(APPEND CMAKE_LIBRARY_PATH ${EXTERNAL_LIBRARY_DIR}/python2.7/libs)

include(${CMAKE_ROOT}/Modules/FindPythonLibs.cmake)

set(PYTHON_FOUND ${PYTHONLIBS_FOUND})
generate_import_target(PYTHON STATIC TARGET PythonLibs::PythonLibs)

list(REMOVE_AT CMAKE_INCLUDE_PATH -1)
list(REMOVE_AT CMAKE_LIBRARY_PATH -1)
