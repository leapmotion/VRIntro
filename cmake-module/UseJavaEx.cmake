include(UseJava)

function(add_jar_ex _TARGET_NAME)

    # In CMake < 2.8.12, add_jar used variables which were set prior to calling
    # add_jar for customizing the behavior of add_jar. In order to be backwards
    # compatible, check if any of those variables are set, and use them to
    # initialize values of the named arguments. (Giving the corresponding named
    # argument will override the value set here.)
    #
    # New features should use named arguments only.
    if(DEFINED CMAKE_JAVA_TARGET_VERSION)
        set(_add_jar_VERSION "${CMAKE_JAVA_TARGET_VERSION}")
    endif()
    if(DEFINED CMAKE_JAVA_TARGET_OUTPUT_DIR)
        set(_add_jar_OUTPUT_DIR "${CMAKE_JAVA_TARGET_OUTPUT_DIR}")
    endif()
    if(DEFINED CMAKE_JAVA_TARGET_OUTPUT_NAME)
        set(_add_jar_OUTPUT_NAME "${CMAKE_JAVA_TARGET_OUTPUT_NAME}")
        # reset
        set(CMAKE_JAVA_TARGET_OUTPUT_NAME)
    endif()
    if(DEFINED CMAKE_JAVA_JAR_ENTRY_POINT)
        set(_add_jar_ENTRY_POINT "${CMAKE_JAVA_JAR_ENTRY_POINT}")
    endif()

    cmake_parse_arguments(_add_jar
      ""
      "VERSION;OUTPUT_DIR;OUTPUT_NAME;ENTRY_POINT;MANIFEST"
      "SOURCES;FILELIST;INCLUDE_JARS"
      ${ARGN}
    )

    set(_JAVA_SOURCE_FILES ${_add_jar_SOURCES} ${_add_jar_UNPARSED_ARGUMENTS})

    if (NOT DEFINED _add_jar_OUTPUT_DIR)
        set(_add_jar_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    if (_add_jar_ENTRY_POINT)
        set(_ENTRY_POINT_OPTION e)
        set(_ENTRY_POINT_VALUE ${_add_jar_ENTRY_POINT})
    endif ()

    if (_add_jar_MANIFEST)
        set(_MANIFEST_OPTION m)
        set(_MANIFEST_VALUE ${_add_jar_MANIFEST})
    endif ()

    if (LIBRARY_OUTPUT_PATH)
        set(CMAKE_JAVA_LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH})
    else ()
        set(CMAKE_JAVA_LIBRARY_OUTPUT_PATH ${_add_jar_OUTPUT_DIR})
    endif ()

    set(CMAKE_JAVA_INCLUDE_PATH
        ${CMAKE_JAVA_INCLUDE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_JAVA_OBJECT_OUTPUT_PATH}
        ${CMAKE_JAVA_LIBRARY_OUTPUT_PATH}
    )

    if (WIN32 AND NOT CYGWIN AND CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        set(CMAKE_JAVA_INCLUDE_FLAG_SEP ";")
    else ()
        set(CMAKE_JAVA_INCLUDE_FLAG_SEP ":")
    endif()

    foreach (JAVA_INCLUDE_DIR ${CMAKE_JAVA_INCLUDE_PATH})
       set(CMAKE_JAVA_INCLUDE_PATH_FINAL "${CMAKE_JAVA_INCLUDE_PATH_FINAL}${CMAKE_JAVA_INCLUDE_FLAG_SEP}${JAVA_INCLUDE_DIR}")
    endforeach()

    set(CMAKE_JAVA_CLASS_OUTPUT_PATH "${_add_jar_OUTPUT_DIR}${CMAKE_FILES_DIRECTORY}/${_TARGET_NAME}.dir")

    set(_JAVA_TARGET_OUTPUT_NAME "${_TARGET_NAME}.jar")
    if (_add_jar_OUTPUT_NAME AND _add_jar_VERSION)
        set(_JAVA_TARGET_OUTPUT_NAME "${_add_jar_OUTPUT_NAME}-${_add_jar_VERSION}.jar")
        set(_JAVA_TARGET_OUTPUT_LINK "${_add_jar_OUTPUT_NAME}.jar")
    elseif (_add_jar_VERSION)
        set(_JAVA_TARGET_OUTPUT_NAME "${_TARGET_NAME}-${_add_jar_VERSION}.jar")
        set(_JAVA_TARGET_OUTPUT_LINK "${_TARGET_NAME}.jar")
    elseif (_add_jar_OUTPUT_NAME)
        set(_JAVA_TARGET_OUTPUT_NAME "${_add_jar_OUTPUT_NAME}.jar")
    endif ()

    set(_JAVA_CLASS_FILES)
    set(_JAVA_COMPILE_FILES)
    set(_JAVA_DEPENDS)
    set(_JAVA_COMPILE_DEPENDS)
    set(_JAVA_RESOURCE_FILES)
    foreach(_JAVA_SOURCE_FILE ${_JAVA_SOURCE_FILES})
        get_filename_component(_JAVA_EXT ${_JAVA_SOURCE_FILE} EXT)
        get_filename_component(_JAVA_FILE ${_JAVA_SOURCE_FILE} NAME_WE)
        get_filename_component(_JAVA_PATH ${_JAVA_SOURCE_FILE} PATH)
        get_filename_component(_JAVA_FULL ${_JAVA_SOURCE_FILE} ABSOLUTE)

        if (_JAVA_EXT MATCHES ".java")
            file(RELATIVE_PATH _JAVA_REL_BINARY_PATH ${_add_jar_OUTPUT_DIR} ${_JAVA_FULL})
            file(RELATIVE_PATH _JAVA_REL_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${_JAVA_FULL})
            string(LENGTH ${_JAVA_REL_BINARY_PATH} _BIN_LEN)
            string(LENGTH ${_JAVA_REL_SOURCE_PATH} _SRC_LEN)
            if (${_BIN_LEN} LESS ${_SRC_LEN})
                set(_JAVA_REL_PATH ${_JAVA_REL_BINARY_PATH})
            else ()
                set(_JAVA_REL_PATH ${_JAVA_REL_SOURCE_PATH})
            endif ()
            get_filename_component(_JAVA_REL_PATH ${_JAVA_REL_PATH} PATH)

            list(APPEND _JAVA_COMPILE_FILES ${_JAVA_SOURCE_FILE})
            set(_JAVA_CLASS_FILE "${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_REL_PATH}/${_JAVA_FILE}.class")
            set(_JAVA_CLASS_FILES ${_JAVA_CLASS_FILES} ${_JAVA_CLASS_FILE})

        elseif (_JAVA_EXT MATCHES ".jar"
                OR _JAVA_EXT MATCHES ".war"
                OR _JAVA_EXT MATCHES ".ear"
                OR _JAVA_EXT MATCHES ".sar")
            # Ignored for backward compatibility

        elseif (_JAVA_EXT STREQUAL "")
            list(APPEND CMAKE_JAVA_INCLUDE_PATH ${JAVA_JAR_TARGET_${_JAVA_SOURCE_FILE}} ${JAVA_JAR_TARGET_${_JAVA_SOURCE_FILE}_CLASSPATH})
            list(APPEND _JAVA_DEPENDS ${JAVA_JAR_TARGET_${_JAVA_SOURCE_FILE}})

        else ()
            __java_copy_file(${CMAKE_CURRENT_SOURCE_DIR}/${_JAVA_SOURCE_FILE}
                             ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_SOURCE_FILE}
                             "Copying ${_JAVA_SOURCE_FILE} to the build directory")
            list(APPEND _JAVA_RESOURCE_FILES ${_JAVA_SOURCE_FILE})
        endif ()
    endforeach()

    foreach(_JAVA_INCLUDE_JAR ${_add_jar_INCLUDE_JARS})
        if (TARGET ${_JAVA_INCLUDE_JAR})
            get_target_property(_JAVA_JAR_PATH ${_JAVA_INCLUDE_JAR} JAR_FILE)
            if (_JAVA_JAR_PATH)
                set(CMAKE_JAVA_INCLUDE_PATH_FINAL "${CMAKE_JAVA_INCLUDE_PATH_FINAL}${CMAKE_JAVA_INCLUDE_FLAG_SEP}${_JAVA_JAR_PATH}")
                list(APPEND CMAKE_JAVA_INCLUDE_PATH ${_JAVA_JAR_PATH})
                list(APPEND _JAVA_DEPENDS ${_JAVA_INCLUDE_JAR})
                list(APPEND _JAVA_COMPILE_DEPENDS ${_JAVA_INCLUDE_JAR})
            else ()
                message(SEND_ERROR "add_jar: INCLUDE_JARS target ${_JAVA_INCLUDE_JAR} is not a jar")
            endif ()
        else ()
            set(CMAKE_JAVA_INCLUDE_PATH_FINAL "${CMAKE_JAVA_INCLUDE_PATH_FINAL}${CMAKE_JAVA_INCLUDE_FLAG_SEP}${_JAVA_INCLUDE_JAR}")
            list(APPEND CMAKE_JAVA_INCLUDE_PATH "${_JAVA_INCLUDE_JAR}")
            list(APPEND _JAVA_DEPENDS "${_JAVA_INCLUDE_JAR}")
            list(APPEND _JAVA_COMPILE_DEPENDS "${_JAVA_INCLUDE_JAR}")
        endif ()
    endforeach()

    # create an empty java_class_filelist
    if (NOT EXISTS ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist)
        file(WRITE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist "")
    endif()

    if (_JAVA_COMPILE_FILES OR _add_jar_FILELIST)
        # Create the list of files to compile.
        set(_JAVA_SOURCES_FILE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_sources)
        if (_JAVA_COMPILE_FILES)
          string(REPLACE ";" "\"\n\"" _JAVA_COMPILE_STRING "\"${_JAVA_COMPILE_FILES}\"")
        else()
          set(_JAVA_COMPILE_STRING "")
        endif()
        if(EXISTS ${_JAVA_SOURCES_FILE})
          file(READ ${_JAVA_SOURCES_FILE} _JAVA_COMPILE_STRING_CACHED)
          if(NOT _JAVA_COMPILE_STRING_CACHED STREQUAL _JAVA_COMPILE_STRING)
            # Source files changed
            file(STRINGS ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist _JAVA_CLASS_FILELIST)
            foreach(_JAVA_CLASS_FILE ${_JAVA_CLASS_FILELIST})
              file(REMOVE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/${_JAVA_CLASS_FILE})
            endforeach()
            file(REMOVE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_compiled_${_TARGET_NAME})
            file(WRITE ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist "")
          endif()
        endif()

        file(WRITE ${_JAVA_SOURCES_FILE} ${_JAVA_COMPILE_STRING})
        set(_JAVA_COMPILE_ADDITIONAL_FILES)
        if (_add_jar_FILELIST)
          set(_JAVA_COMPILE_ADDITIONAL_FILES @${_add_jar_FILELIST})
        endif()

        # Compile the java files and create a list of class files
        add_custom_command(
            # NOTE: this command generates an artificial dependency file
            OUTPUT ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_compiled_${_TARGET_NAME}
            COMMAND ${Java_JAVAC_EXECUTABLE}
                ${CMAKE_JAVA_COMPILE_FLAGS}
                -classpath "${CMAKE_JAVA_INCLUDE_PATH_FINAL}"
                -d ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
                @${_JAVA_SOURCES_FILE} ${_JAVA_COMPILE_ADDITIONAL_FILES}
            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_compiled_${_TARGET_NAME}
            DEPENDS ${_JAVA_COMPILE_FILES} ${_JAVA_COMPILE_DEPENDS} ${_add_jar_FILELIST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building Java objects for ${_JAVA_TARGET_OUTPUT_NAME}"
        )
        add_custom_command(
            OUTPUT ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist
            COMMAND ${CMAKE_COMMAND}
                -DCMAKE_JAVA_CLASS_OUTPUT_PATH=${CMAKE_JAVA_CLASS_OUTPUT_PATH}
                -DCMAKE_JAR_CLASSES_PREFIX="${CMAKE_JAR_CLASSES_PREFIX}"
                -P ${_JAVA_CLASS_FILELIST_SCRIPT}
            DEPENDS ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_compiled_${_TARGET_NAME}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif ()

    # create the jar file
    set(_JAVA_JAR_OUTPUT_PATH
      ${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_NAME})
    if (CMAKE_JNI_TARGET)
        add_custom_command(
            OUTPUT ${_JAVA_JAR_OUTPUT_PATH}
            COMMAND ${Java_JAR_EXECUTABLE}
                -cf${_ENTRY_POINT_OPTION}${_MANIFEST_OPTION} ${_JAVA_JAR_OUTPUT_PATH} ${_ENTRY_POINT_VALUE} ${_MANIFEST_VALUE}
                ${_JAVA_RESOURCE_FILES} @java_class_filelist
            COMMAND ${CMAKE_COMMAND}
                -D_JAVA_TARGET_DIR=${_add_jar_OUTPUT_DIR}
                -D_JAVA_TARGET_OUTPUT_NAME=${_JAVA_TARGET_OUTPUT_NAME}
                -D_JAVA_TARGET_OUTPUT_LINK=${_JAVA_TARGET_OUTPUT_LINK}
                -P ${_JAVA_SYMLINK_SCRIPT}
            COMMAND ${CMAKE_COMMAND}
                -D_JAVA_TARGET_DIR=${_add_jar_OUTPUT_DIR}
                -D_JAVA_TARGET_OUTPUT_NAME=${_JAVA_JAR_OUTPUT_PATH}
                -D_JAVA_TARGET_OUTPUT_LINK=${_JAVA_TARGET_OUTPUT_LINK}
                -P ${_JAVA_SYMLINK_SCRIPT}
            DEPENDS ${_JAVA_RESOURCE_FILES} ${_JAVA_DEPENDS} ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist
            WORKING_DIRECTORY ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
            COMMENT "Creating Java archive ${_JAVA_TARGET_OUTPUT_NAME}"
        )
    else ()
        add_custom_command(
            OUTPUT ${_JAVA_JAR_OUTPUT_PATH}
            COMMAND ${Java_JAR_EXECUTABLE}
                -cf${_ENTRY_POINT_OPTION}${_MANIFEST_OPTION} ${_JAVA_JAR_OUTPUT_PATH} ${_ENTRY_POINT_VALUE} ${_MANIFEST_VALUE}
                ${_JAVA_RESOURCE_FILES} @java_class_filelist
            COMMAND ${CMAKE_COMMAND}
                -D_JAVA_TARGET_DIR=${_add_jar_OUTPUT_DIR}
                -D_JAVA_TARGET_OUTPUT_NAME=${_JAVA_TARGET_OUTPUT_NAME}
                -D_JAVA_TARGET_OUTPUT_LINK=${_JAVA_TARGET_OUTPUT_LINK}
                -P ${_JAVA_SYMLINK_SCRIPT}
            WORKING_DIRECTORY ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
            DEPENDS ${_JAVA_RESOURCE_FILES} ${_JAVA_DEPENDS} ${CMAKE_JAVA_CLASS_OUTPUT_PATH}/java_class_filelist
            COMMENT "Creating Java archive ${_JAVA_TARGET_OUTPUT_NAME}"
        )
    endif ()

    # Add the target and make sure we have the latest resource files.
    add_custom_target(${_TARGET_NAME} ALL DEPENDS ${_JAVA_JAR_OUTPUT_PATH})

    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            INSTALL_FILES
                ${_JAVA_JAR_OUTPUT_PATH}
    )

    if (_JAVA_TARGET_OUTPUT_LINK)
        set_property(
            TARGET
                ${_TARGET_NAME}
            PROPERTY
                INSTALL_FILES
                    ${_JAVA_JAR_OUTPUT_PATH}
                    ${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_LINK}
        )

        if (CMAKE_JNI_TARGET)
            set_property(
                TARGET
                    ${_TARGET_NAME}
                PROPERTY
                    JNI_SYMLINK
                        ${_add_jar_OUTPUT_DIR}/${_JAVA_TARGET_OUTPUT_LINK}
            )
        endif ()
    endif ()

    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            JAR_FILE
                ${_JAVA_JAR_OUTPUT_PATH}
    )

    set_property(
        TARGET
            ${_TARGET_NAME}
        PROPERTY
            CLASSDIR
                ${CMAKE_JAVA_CLASS_OUTPUT_PATH}
    )

endfunction()
