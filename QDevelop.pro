CONFIG += qt uitools warn_on release
DESTDIR += bin
FORMS += ui/about.ui \
 ui/addexistant.ui \
 ui/addnew.ui \
 ui/addnewclass.ui \
 ui/addnewclassmethod.ui \
 ui/addnewclassvariable.ui \
 ui/addscope.ui \
 ui/addsetget.ui \
 ui/exechoice.ui \
 ui/findfiles.ui \
 ui/findwidget.ui \
 ui/gotoline.ui \
 ui/main.ui \
 ui/newimplementation.ui \
 ui/newproject.ui \
 ui/newvariable.ui \
 ui/openfile.ui \
 ui/options.ui \
 ui/parameters.ui \
 ui/projectpropertie.ui \
 ui/replacewidget.ui \
 ui/shortcuts.ui \
 ui/subclassing.ui \
 ui/toolsControl.ui \
 ui/warning.ui \
 ui/breakpointcondition.ui \
 ui/registers.ui
HEADERS += src/InitCompletion.h \
 src/QIComplete/parse.h \
 src/QIComplete/readtags.h \
 src/QIComplete/tree.h \
 src/addexistantimpl.h \
 src/addnewclassimpl.h \
 src/addnewclassmethodimpl.h \
 src/addnewclassvariableimpl.h \
 src/addnewimpl.h \
 src/addscopeimpl.h \
 src/addsetgetimpl.h \
 src/assistant.h \
 src/build.h \
 src/cpphighlighter.h \
 src/debug.h \
 src/designer.h \
 src/editor.h \
 src/findfileimpl.h \
 src/lineedit.h \
 src/linenumbers.h \
 src/logbuild.h \
 src/mainimpl.h \
 src/misc.h \
 src/newprojectimpl.h \
 src/openfileimpl.h \
 src/optionsimpl.h \
 src/parametersimpl.h \
 src/pluginsinterfaces.h \
 src/projectmanager.h \
 src/projectpropertieimpl.h \
 src/selectionborder.h \
 src/shortcutsimpl.h \
 src/stackimpl.h \
 src/subclassingimpl.h \
 src/tabwidget.h \
 src/textEdit.h \
 src/toolscontrolimpl.h \
 src/treeclasses.h \
 src/treeproject.h \
 src/registersimpl.h
INCLUDEPATH += . src src/ui
MOC_DIR += build/moc
QT += core gui network sql
RCC_DIR += build/rcc
RC_FILE += QDevelop.rc
RESOURCES += resources/resources.qrc
SOURCES += src/InitCompletion.cpp \
 src/QIComplete/parse.cpp \
 src/QIComplete/readtags.cpp \
 src/QIComplete/tree.cpp \
 src/addexistantimpl.cpp \
 src/addnewclassimpl.cpp \
 src/addnewclassmethodimpl.cpp \
 src/addnewclassvariableimpl.cpp \
 src/addnewimpl.cpp \
 src/addscopeimpl.cpp \
 src/addsetgetimpl.cpp \
 src/assistant.cpp \
 src/build.cpp \
 src/cpphighlighter.cpp \
 src/debug.cpp \
 src/designer.cpp \
 src/editor.cpp \
 src/findfileimpl.cpp \
 src/lineedit.cpp \
 src/linenumbers.cpp \
 src/logbuild.cpp \
 src/main.cpp \
 src/mainimpl.cpp \
 src/misc.cpp \
 src/newprojectimpl.cpp \
 src/openfileimpl.cpp \
 src/optionsimpl.cpp \
 src/parametersimpl.cpp \
 src/projectmanager.cpp \
 src/projectpropertieimpl.cpp \
 src/selectionborder.cpp \
 src/shortcutsimpl.cpp \
 src/stackimpl.cpp \
 src/subclassingimpl.cpp \
 src/tabwidget.cpp \
 src/textEdit.cpp \
 src/toolscontrolimpl.cpp \
 src/treeclasses.cpp \
 src/treeproject.cpp \
 src/registersimpl.cpp
TEMPLATE = app
TRANSLATIONS += resources/translations/QDevelop_Chinese.ts \
 resources/translations/QDevelop_Czech.ts \
 resources/translations/QDevelop_Dutch.ts \
 resources/translations/QDevelop_French.ts \
 resources/translations/QDevelop_German.ts \
 resources/translations/QDevelop_Italian.ts \
 resources/translations/QDevelop_Polish.ts \
 resources/translations/QDevelop_Portuguese.ts \
 resources/translations/QDevelop_Russian.ts \
 resources/translations/QDevelop_Spanish.ts \
 resources/translations/QDevelop_Turkish.ts \
 resources/translations/QDevelop_Ukrainian.ts \
 resources/translations/QDevelop_Hungarian.ts \
 resources/translations/QDevelop_Japanese.ts \
 resources/translations/QDevelop_Vietnamese.ts
UI_DIR += build/ui
macx {
 TARGET =  QDevelop
 ICON +=  resources/images/qdevelop.icns
 OBJECTS_DIR +=  build/o/mac
}
unix {
 TARGET =  qdevelop
 OBJECTS_DIR +=  build/o/unix
 target.path +=  /usr/bin/
 INSTALLS +=  target
}
win32 {
 TARGET =  QDevelop
 OBJECTS_DIR +=  build/o/win32
 CONFIG -=  debug_and_release
}
!exists(resources/translations/QDevelop_Russian.qm) {
 error(Please run \"lrelease QDevelop.pro\" before building the project)
}
