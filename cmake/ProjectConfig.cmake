if(WIN32)
	include(WinResource)
endif()

function(create_private_config output name)
	string(REPLACE "-" "_" OSMSCOUT_PRIVATE_CONFIG_HEADER_NAME ${name})
	string(TOUPPER ${OSMSCOUT_PRIVATE_CONFIG_HEADER_NAME} OSMSCOUT_PRIVATE_CONFIG_HEADER_NAME)
	configure_file("${OSMSCOUT_BASE_DIR_SOURCE}/cmake/Config.h.cmake" ${output})
endfunction()

macro(osmscout_library_project)
	set(_name)
	set(_output)
	set(_alias)
	set(_sources)
	set(_headers)
	set(_targets)
	set(_includes
		include
		${PROJECT_SOURCE_DIR}/include
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_BINARY_DIR}/privateinclude
		${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	set(_folder "libosmscout")
	set(_includedir "osmscout")
	set(_template)
	set(_feature)
	set(_exclude)
	set(_skip_header OFF)
	set(cmd "_name")
	foreach(arg ${ARGN})
		if(arg STREQUAL "NAME")
			set(cmd "_name")
		elseif(arg STREQUAL "OUTPUT_NAME")
			set(cmd "_output")
		elseif(arg STREQUAL "ALIAS")
			set(cmd "_alias")
		elseif(arg STREQUAL "SOURCE")
			set(cmd "_sources")
		elseif(arg STREQUAL "HEADER")
			set(cmd "_headers")
		elseif(arg STREQUAL "TARGET")
			set(cmd "_targets")
		elseif(arg STREQUAL "INCLUDES")
			set(cmd "_includes")
		elseif(arg STREQUAL "FOLDER")
			set(cmd "_folder")
		elseif(arg STREQUAL "TEMPLATE")
			set(cmd "_template")
		elseif(arg STREQUAL "INCLUDEDIR")
			set(cmd "_includedir")
		elseif(arg STREQUAL "FEATURE")
			set(cmd "_feature")
		elseif(arg STREQUAL "EXCLUDE")
			set(cmd "_exclude")
		elseif(arg STREQUAL "SKIP_HEADER")
			set(_skip_header ON)
		else()
			if("${cmd}" STREQUAL "_name")
				set(_name ${arg})
			elseif("${cmd}" STREQUAL "_output")
				set(_output ${arg})
			elseif("${cmd}" STREQUAL "_alias")
				set(_alias ${arg})
			elseif("${cmd}" STREQUAL "_folder")
				set(_folder ${arg})
			elseif("${cmd}" STREQUAL "_template")
				set(_template ${arg})
			elseif("${cmd}" STREQUAL "_includedir")
				set(_includedir ${arg})
			elseif("${cmd}" STREQUAL "_feature")
				set(_feature ${arg})
			else()
				list(APPEND ${cmd} ${arg})
			endif()
		endif()
	endforeach()
	add_library(${_name} ${_sources} ${_headers})
	add_library(OSMScout::${_alias} ALIAS ${_name})
	set_target_properties(${_name} PROPERTIES
		OUTPUT_NAME "${_output}"
		VERSION ${OSMSCOUT_LIBRARY_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
		FOLDER "${_folder}"
	)
	if(EXISTS ${_template})
		if(NOT _feature)
			get_filename_component(_template_name ${_template} NAME_WE)
			cmake_path(RELATIVE_PATH _template BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ OUTPUT_VARIABLE _template_dir)
			cmake_path(GET _template_dir PARENT_PATH _template_dir)
			message("Template dir: " ${_template_dir})
			set(_feature ${CMAKE_CURRENT_BINARY_DIR}/include/${_template_dir}/${_template_name}.h)
		endif()
		configure_file(${_template} ${_feature})
		install(FILES ${_feature} DESTINATION include/${_includedir})
	endif()
	string(REPLACE "-" "_" OSMSCOUT_PRIVATE_CONFIG_HEADER_NAME ${_name})
	string(TOUPPER ${OSMSCOUT_PRIVATE_CONFIG_HEADER_NAME} OSMSCOUT_PRIVATE_CONFIG_HEADER_NAME)
	configure_file("${OSMSCOUT_BASE_DIR_SOURCE}/cmake/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/include/${_includedir}/private/Config.h")
	target_include_directories(${_name} PRIVATE ${_includes})
	target_include_directories(${_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)
	target_link_libraries(${_name} ${_targets})
	if(WIN32 AND BUILD_SHARED_LIBS)
		create_win32_dll_resource(${_name} "${_output}")
	endif()
	install(TARGETS ${_name}
			RUNTIME DESTINATION bin
			BUNDLE DESTINATION bin
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
			FRAMEWORK DESTINATION lib)
	if(NOT ${_skip_header})
		if(NOT _exclude)
			install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${_includedir} DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "private" EXCLUDE)
		else()
			set(_exclude_cmd PATTERN "private" EXCLUDE)
			foreach(ef ${EXCLUDE_HEADER})
				set(_exclude_cmd ${_exclude_cmd} PATTERN ${ef} EXCLUDE)
			endforeach()
			install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${_includedir} DESTINATION include FILES_MATCHING PATTERN "*.h" ${_exclude_cmd})
		endif()
	endif()
	if(OSMSCOUT_INSTALL_QT_DLL AND TARGET Qt::windeployqt AND BUILD_SHARED_LIBS)
		foreach(T ${_targets})
			if("${T}" STREQUAL "Qt::Core" OR "${T}" STREQUAL "Qt::Gui" OR "${T}" STREQUAL "Qt::Widgets" OR "${T}" STREQUAL "Qt::Svg")
				add_custom_command(TARGET ${_name}
					POST_BUILD
					COMMAND set PATH=%PATH%$<SEMICOLON>${qt_install_prefix}/bin
					COMMAND Qt::windeployqt --dir "${CMAKE_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:${_name}>/$<TARGET_FILE_NAME:${_name}>"
				)
				break()
			endif()
		endforeach()
	endif()
	if(MSVC)
		if(BUILD_SHARED_LIBS)
			install(FILES $<TARGET_PDB_FILE:${_name}> DESTINATION ${CMAKE_INSTALL_PREFIX}/bin CONFIGURATIONS Debug)
		endif()
	endif()
	configure_file("${PROJECT_SOURCE_DIR}/pkgconfig-template.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/${_output}.pc" @ONLY)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_output}.pc" DESTINATION lib/pkgconfig)
endmacro(osmscout_library_project)

macro(osmscout_demo_project)
	set(_name)
	set(_sources)
	set(_targets)
	set(_includes
		include
		${PROJECT_SOURCE_DIR}/include
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_BINARY_DIR}/privateinclude
		${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	set(cmd "_name")
	foreach(arg ${ARGN})
		if(arg STREQUAL "NAME")
			set(cmd "_name")
		elseif(arg STREQUAL "SOURCES")
			set(cmd "_sources")
		elseif(arg STREQUAL "TARGET")
			set(cmd "_targets")
		elseif(arg STREQUAL "INCLUDES")
			set(cmd "_includes")
		else()
			if("${cmd}" STREQUAL "_name")
				set(_name ${arg})
			else()
				list(APPEND ${cmd} ${arg})
			endif()
		endif()
	endforeach()
	add_executable(${_name} ${_sources})
	set_target_properties(${_name} PROPERTIES VERSION ${OSMSCOUT_LIBRARY_VERSION} SOVERSION ${OSMSCOUT_LIBRARY_VERSION} FOLDER "Demos")
	target_link_libraries(${_name} ${_targets})
	install(TARGETS ${_name} RUNTIME DESTINATION share/osmscout/demos BUNDLE DESTINATION share/osmscout/demos LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
	target_include_directories(${_name} PRIVATE ${_includes})
	if(WIN32 AND HAVE_WINRES_H)
		create_win32_demo_resource(${_name})
	endif()
endmacro(osmscout_demo_project)

macro(osmscout_test_project)
	set(_name)
	set(_sources)
	set(_targets OSMScout::OSMScout Catch2::Catch2WithMain)
	set(_includes
		include
		${PROJECT_SOURCE_DIR}/include
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_BINARY_DIR}/privateinclude
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	if(MARISA_FOUND)
		list(APPEND _includes ${MARISA_INCLUDE_DIRS})
	endif()

	set(_command)
	set(_skiptest OFF)
	set(cmd "_name")
	foreach(arg ${ARGN})
		if(arg STREQUAL "NAME")
			set(cmd "_name")
		elseif(arg STREQUAL "SOURCES")
			set(cmd "_sources")
		elseif(arg STREQUAL "TARGET")
			set(cmd "_targets")
		elseif(arg STREQUAL "INCLUDES")
			set(cmd "_includes")
		elseif(arg STREQUAL "COMMAND")
			set(cmd "_command")
		elseif(arg STREQUAL "SKIPTEST")
			set(_skiptest ON)
		else()
			if("${cmd}" STREQUAL "_name")
				set(_name ${arg})
			else()
				list(APPEND ${cmd} ${arg})
			endif()
		endif()
	endforeach()
	add_executable(${_name} ${_sources})
	set_target_properties(${_name} PROPERTIES VERSION ${OSMSCOUT_LIBRARY_VERSION} SOVERSION ${OSMSCOUT_LIBRARY_VERSION} FOLDER "Tests")
	target_link_libraries(${_name} ${_targets})
	install(TARGETS ${_name} RUNTIME DESTINATION share/osmscout/tests BUNDLE DESTINATION share/osmscout/tests LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
	target_include_directories(${_name} PRIVATE ${_includes})
	if(WIN32)
		create_win32_test_resource(${_name})
	endif()
	if(NOT _skiptest)
		add_test(NAME ${_name} COMMAND ${_name} ${_command} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
		if(MSVC)
			set(qt5_bin "")
			if(QT_FOUND AND MSVC AND TARGET Qt::qmake)
				get_target_property(_qt_qmake_location Qt::qmake IMPORTED_LOCATION)
				execute_process(COMMAND "${_qt_qmake_location}" -query QT_INSTALL_PREFIX RESULT_VARIABLE return_code OUTPUT_VARIABLE qt5_install_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
				file(TO_NATIVE_PATH "${qt_install_prefix}" qt5_install_prefix)
				set(qt5_bin ";${qt_install_prefix}\\bin")
			endif()
			set(envpath "PATH=$<TARGET_FILE_DIR:OSMScout>${qt5_bin}")
			get_target_property(envpath_user OSMScout BINARY_DIR)
			file(TO_NATIVE_PATH "${envpath_user}" envpath_user)
			set(envpath_user "PATH=${envpath_user}\\$(Configuration)${qt5_bin}")
			set(OSMLIBS OSMScoutMap OSMScoutMapAGG OSMScoutMapQt OSMScoutMapCairo OSMScoutImport OSMScoutGPX OSMScoutClientQt OSMScoutTest)
			foreach(tg IN LISTS OSMLIBS)
				if(TARGET ${tg})
					set(envpath "${envpath};$<TARGET_FILE_DIR:${tg}>")
					get_target_property(TBG ${tg} BINARY_DIR)
					file(TO_NATIVE_PATH "${TBG}" TBG)
					set(envpath_user "${envpath_user};${TBG}\\$(Configuration)")
				endif()
			endforeach()
			set_tests_properties(${_name} PROPERTIES ENVIRONMENT "${envpath};$ENV{PATH}")
			file(TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}" working_directory)
			file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_name}.vcxproj.user "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project ToolsVersion=\"15.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
  <PropertyGroup>
    <LocalDebuggerWorkingDirectory>${working_directory}</LocalDebuggerWorkingDirectory>
    <LocalDebuggerEnvironment>${envpath_user};$(PATH)
$(LocalDebuggerEnvironment)</LocalDebuggerEnvironment>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
</Project>")
		endif()
	endif()
endmacro(osmscout_test_project)
