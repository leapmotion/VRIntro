#fixup_qt5_targets
#makes Qt5::Gui depend on Qt5::QWindowsIntegrationPlugin on windows, and sets
#the INSTALL_SUBDIR property on QWindowsIntegrationPlugin so that target_imported_libraries
#will copy it to the correct location.  Must be called after qt5 is found
function(fixup_qt5_targets)
  if(WIN32)
  	set_property(TARGET Qt5::QWindowsIntegrationPlugin PROPERTY INSTALL_SUBDIR platforms)
  	set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_LINK_MODULES Qt5::QWindowsIntegrationPlugin)

    # TODO: link libicu statically with Qt to remove three DLLs
    set(ICU_VERSION "52.1")
    set(ICU_DIR ${EXTERNAL_LIBRARY_DIR}/icu-${ICU_VERSION}${ALTERNATE_LIBRARY})
    foreach(icu_dll icudt52 icuuc52 icuin52)
    	add_library(ICU::${icu_dll} MODULE IMPORTED)
    	set_property(TARGET ICU::${icu_dll} PROPERTY IMPORTED_LOCATION ${ICU_DIR}/bin/${icu_dll}${SHARED_LIBRARY_SUFFIX})
    	set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_LINK_MODULES ICU::${icu_dll})
    endforeach()
  endif()

endfunction()