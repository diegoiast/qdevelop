# qdevelop CMakeLists.txt
# 
# To build qdevelop, you will need cmake 2.4.3 (should also work with 2.4.0 and above
# but not tested). To build the application run this commands:
# 
#	mkdir cbuild
#	cd cbuild
#	cmake ../
#	make
# 
# Please indent this file using tabs, and set the tabs to 8 spaces :)
# 
# If you don't konw cmake, here are some links for you:
# 	http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
# 	http://qtnode.net/wiki/Qt_with_cmake#cmake_for_Qt_4

CMAKE_MINIMUM_REQUIRED(VERSION 3.20.0)
PROJECT(QDevelop)
SET(QDEV	"qdevelop"	CACHE STRING "Name of the executeable")


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOUIC ON)

# SET( QT_USE_QTASSISTANT )
# SET( QT_USE_QTDESIGNER )
# SET( QT_USE_QTNETWORK )
# SET( QT_USE_QTSQL )
# SET( QT_USE_QTXML )
# SET( QT_USE_QTUITOOLS )
find_package(Qt6 COMPONENTS Core Widgets PrintSupport Sql Xml Core5Compat)

SET( qdevelop_SRCS
    ui/about.ui 
    ui/addexistant.ui
    ui/addnew.ui
    ui/addnewclass.ui
    ui/addnewclassmethod.ui
    ui/addnewclassvariable.ui
    ui/addsetget.ui
    ui/addscope.ui
    ui/breakpointcondition.ui
    ui/exechoice.ui
    ui/findfiles.ui
    ui/findwidget.ui
    ui/gotoline.ui
    ui/main.ui
    ui/newimplementation.ui
    ui/newproject.ui
    ui/newvariable.ui
    ui/options.ui
    ui/openfile.ui
    ui/parameters.ui
    ui/projectpropertie.ui
    ui/registers.ui
    ui/replacewidget.ui
    ui/shortcuts.ui
    ui/subclassing.ui
    ui/toolsControl.ui
    ui/warning.ui
    
	src/InitCompletion.cpp
	src/QIComplete/parse.cpp
	src/QIComplete/readtags.cpp
	src/QIComplete/tree.cpp
	src/addexistantimpl.cpp
	src/addnewimpl.cpp
	src/addnewclassimpl.cpp
	src/addscopeimpl.cpp
	src/addsetgetimpl.cpp 
	src/addnewclassmethodimpl.cpp
	src/addnewclassvariableimpl.cpp
	src/assistant.cpp
	src/build.cpp
	src/cpphighlighter.cpp
	src/debug.cpp
	src/designer.cpp
	src/editor.cpp
	src/findfileimpl.cpp
	src/lineedit.cpp
	src/logbuild.cpp
	src/linenumbers.cpp
	src/main.cpp
	src/mainimpl.cpp
	src/misc.cpp
	src/newprojectimpl.cpp
	src/optionsimpl.cpp
	src/openfileimpl.cpp 
	src/parametersimpl.cpp
	src/projectmanager.cpp
	src/projectpropertieimpl.cpp
	src/registersimpl.cpp
	src/selectionborder.cpp
	src/shortcutsimpl.cpp
	src/stackimpl.cpp
	src/subclassingimpl.cpp
	src/tabwidget.cpp
	src/textEdit.cpp
	src/treeclasses.cpp
	src/treeproject.cpp
	src/toolscontrolimpl.cpp
	src/getlastversion.cpp
        src/InitCompletion.h
	src/addexistantimpl.h
	src/addnewimpl.h
	src/addnewclassimpl.h
	src/addnewclassmethodimpl.h
	src/addnewclassvariableimpl.h
	src/addscopeimpl.h
	src/addsetgetimpl.h
	src/assistant.h
	src/build.h
	src/debug.h
	src/editor.h
	src/findfileimpl.h
	src/linenumbers.h
	src/logbuild.h
	src/mainimpl.h
	src/newprojectimpl.h
	src/optionsimpl.h
	src/openfileimpl.h
	src/parametersimpl.h
	src/projectmanager.h
	src/projectpropertieimpl.h
	src/registersimpl.h
	src/shortcutsimpl.h
	src/stackimpl.h
	src/subclassingimpl.h
	src/tabwidget.h
	src/textEdit.h
	src/treeclasses.h
	src/treeproject.h
	src/toolscontrolimpl.h
	src/getlastversion.h
        QDevelop.rc
	resources/resources.qrc
)

SET( qdevelop_TS
	resources/translations/QDevelop_Chinese.ts
	resources/translations/QDevelop_Czech.ts
	resources/translations/QDevelop_Dutch.ts
	resources/translations/QDevelop_French.ts
	resources/translations/QDevelop_German.ts
	resources/translations/QDevelop_Hungarian.ts
	resources/translations/QDevelop_Italian.ts
	resources/translations/QDevelop_Japanese.ts
	resources/translations/QDevelop_Polish.ts
	resources/translations/QDevelop_Portuguese.ts
	resources/translations/QDevelop_Russian.ts
	resources/translations/QDevelop_Spanish.ts
	resources/translations/QDevelop_Turkish.ts
	resources/translations/QDevelop_Ukrainian.ts
	resources/translations/QDevelop_Vietnamese.ts

)
add_executable(qdevelop ${qdevelop_SRCS})
target_link_libraries(qdevelop Qt::Widgets Qt::Sql Qt::Core5Compat)


SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(
    ${CMAKE_BINARY_DIR}
	src
	src/ui 
)

# Plugins support

# IF (AUTOPLUGINS)
# 	FILE (GLOB PLUGINS ${QDevelop_SOURCE_DIR}/plugins/*)
# 	FOREACH (PLG ${PLUGINS})
# 		STRING (REGEX MATCH \\.svn FAIL ${PLG})
# 		STRING (COMPARE EQUAL "${FAIL}" "" IS_EMPTY)
# 		IF (IS_EMPTY)
# 			SET (PLG_HXX "")
# 			SET (PLG_CXX "")
# 			SET (PLG_UI "")
# 			SET (PLG_MOC "")
# 			SET (PLG_UIC "")
# 			SET (PLG_TS "")
# 			SET (PLG_QM "")
		
# 			STRING (REPLACE "${QDevelop_SOURCE_DIR}/plugins/" "" PLG_NAME ${PLG})
# 			MESSAGE (STATUS "Preparing plugin ${PLG_NAME} for building")
		
# 			FILE (GLOB PLG_HXX ${PLG}/*.h)
# 			FILE (GLOB PLG_CXX ${PLG}/*.cpp)
# 			FILE (GLOB PLG_UI ${PLG}/*.ui)
# 			FILE (GLOB PLG_TS ${PLG}/translations/*.ts)
# 			QT4_ADD_TRANSLATION (PLG_QM ${PLG_TS})
		
# 			QT4_WRAP_CPP (PLG_MOC ${PLG_HXX})
# 			QT4_WRAP_UI (PLG_UIC ${PLG_UI})
		
# 			ADD_LIBRARY (${PLG_NAME} SHARED ${PLG_CXX} ${PLG_MOC} ${PLG_UIC} ${PLG_QM})
# 			TARGET_LINK_LIBRARIES (${PLG_NAME} ${QT_UITOOLSLIB} 
# 				${QT_QTSQL_LIBRARY}
# 				${QT_LIBRARIES}
# 				${QT_QTXML_LIBRARY}
# 				${QT_QTNETWORK_LIBRARY})
# 			SET_TARGET_PROPERTIES (${PLG_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY plugins LIBRARY_OUTPUT_DIRECTORY plugins)
# 			ADD_DEPENDENCIES (${PLG_NAME} ${QDEV})
		
# 			IF (WIN32)
# 				INSTALL (TARGETS ${PLG_NAME} DESTINATION bin/plugins)
# 				INSTALL (FILES ${PLG_QM} DESTINATION translations/${PLG_NAME})
# 			ELSE (NOT WIN32)
# 				INSTALL (TARGETS ${PLG_NAME} DESTINATION lib/qdevelop/plugins)
# 				INSTALL (FILES ${PLG_QM} DESTINATION lib/qdevelop/translations/${PLG_NAME})
# 			ENDIF (WIN32)
# 		ENDIF (IS_EMPTY)
# 	ENDFOREACH (PLG)
# ENDIF (AUTOPLUGINS)

# INSTALL (TARGETS ${QDEV} RUNTIME DESTINATION bin)
# IF (WIN32)
# 	INSTALL (FILES ${qdevelop_QM} DESTINATION translations)
# ELSE (NOT WIN32)
# 	INSTALL (FILES ${qdevelop_QM} DESTINATION lib/qdevelop/translations)
# ENDIF (WIN32)
