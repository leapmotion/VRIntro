# primary function of this macro: strip symbols at link time on Mac and Linux
# secondary function: avoid MSVC warnings in Debug by specifying /nodefaultlib
# tertiary function: avoid MSVC warnings about combining /incremental with /ltcg
macro(TARGET_STRIP Target)
  set(STRIPPED_EXE_LINKER_FLAGS "-Xlinker -unexported_symbol -Xlinker \"*\" -Xlinker -dead_strip -Xlinker -dead_strip_dylibs")
  if(MSVC)
    if(CMAKE_CXX_FLAGS_RELEASE)
      set_target_properties(${Target} PROPERTIES LINK_FLAGS_DEBUG "/INCREMENTAL:NO /NODEFAULTLIB:MSVCRT")
    else()
      set_target_properties(${Target} PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT")
    endif()
  else()
    set_target_properties(${Target} PROPERTIES LINK_FLAGS_RELEASE ${STRIPPED_EXE_LINKER_FLAGS})
  endif()
endmacro()
