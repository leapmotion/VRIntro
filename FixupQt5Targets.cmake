#fixup_qt5_targets
#makes Qt5::Gui depend on Qt5::QWindowsIntegrationPlugin on windows, and sets
#the INSTALL_SUBDIR property on QWindowsIntegrationPlugin so that target_imported_libraries
#will copy it to the correct location.  Must be called after qt5 is found
function(fixup_qt5_targets)
  if(WIN32)
  	set_property(TARGET Qt5::QWindowsIntegrationPlugin PROPERTY INSTALL_SUBDIR platforms)
  	set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_LINK_MODULES Qt5::QWindowsIntegrationPlugin)
  endif()
endfunction()