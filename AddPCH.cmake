#MSVC pch helper. Copied from http://pastebin.com/84dm5rXZ and modified by Walter Gray.
#Files already in SourcesVar will be marked as using a PCH, then the pch files will be
#appended to the list.

function(add_pch SourcesVar PrecompiledHeader PrecompiledSource)
  if(MSVC)
    set_source_files_properties(${PrecompiledSource}
        PROPERTIES
        COMPILE_FLAGS "/Yc${PrecompiledHeader}"
        )
    foreach( src_file ${${SourcesVar}} )
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/Yu${PrecompiledHeader}"
            )
    endforeach( src_file ${${SourcesVar}} )
    list(APPEND ${SourcesVar} ${PrecompiledHeader} ${PrecompiledSource})
    set(${SourcesVar} ${${SourcesVar}} PARENT_SCOPE)
  endif(MSVC)
endfunction(add_pch)
