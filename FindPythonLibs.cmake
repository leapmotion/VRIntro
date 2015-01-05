include(CreateImportTargetHelpers)

list(APPEND CMAKE_INCLUDE_PATH ${EXTERNAL_LIBRARY_DIR}/python2.7/include)
list(APPEND CMAKE_LIBRARY_PATH ${EXTERNAL_LIBRARY_DIR}/python2.7/libs)
list(APPEND CMAKE_LIBRARY_PATH ${EXTERNAL_LIBRARY_DIR}/python2.7/lib) #different name on Linux cause we're dumb
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_LIBRARY_PATH
    /Library/Frameworks/Python.framework/Versions/2.7/lib
    ${CMAKE_LIBRARY_PATH}
  )
endif()

include(${CMAKE_ROOT}/Modules/FindPythonLibs.cmake)

set(PYTHON_FOUND ${PYTHONLIBS_FOUND})
unset(PYTHON_LIBRARY CACHE)

#HACK - one of our prebuilt libraries is forcing us to link with the release
#version in all cases.
set(PYTHON_LIBRARY_DEBUG ${PYTHON_LIBRARY_RELEASE})

generate_import_target(PYTHON STATIC TARGET PythonLibs::PythonLibs)

list(REMOVE_AT CMAKE_INCLUDE_PATH -1)
list(REMOVE_AT CMAKE_LIBRARY_PATH -1)
list(REMOVE_AT CMAKE_LIBRARY_PATH -1)
