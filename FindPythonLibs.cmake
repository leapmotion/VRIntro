include(CreateImportTargetHelpers)
include(${CMAKE_ROOT}/Modules/FindPythonLibs.cmake)

set(PYTHON_FOUND ${PYTHONLIBS_FOUND})
generate_import_target(PYTHON STATIC TARGET PythonLibs::PythonLibs)