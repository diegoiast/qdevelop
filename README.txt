    QDevelop - A Development Environment for Qt4

    (C) 2006-2010 Jean-Luc Biord <jlbiord@gmail.com>

Installation
~~~~~~~~~~~~

Download is available from http://biord-software.org/qdevelop, the official website.

You need Qt (from Trolltech) at least version 4.3 to be able to compile
QDevelop. Most Linux distributions normally already package the latest
version of Qt, so it is likely that you need to worry about this. However,
you need the development package as well (sometimes named as qt-devel or
qt4-devel, it varies depends on the distributions).

The simplest way to compile QDevelop package is:

1. Extract the source zip.
2.`cd' to the directory containing the package's source code and type
     `qmake' (`qmake -recursive' with Qt 4.3.x) to generate the Makefile for your system.
3. Type `make' on Linux or `mingw32-make` on Windows to compile the package.
4. The executable file QDevelop (QDevelop.exe under Windows) is built on bin/ directory.

If you want to have code completion, install the package ctags. Under Linux,
ctags is normally installed with development packages. For Windows, ctags is accessible here : http://freefr.dl.sourceforge.net/project/ctags/ctags/5.8/ctags58.zip 
If you want to have debugging functionalities, you must install gdb. Like ctags, gdb is normally installed 
with development packages under Linux. gdb for Windows is available here : http://www.mingw.org/download.shtml/

You can also compile QDevelop using cmake, but this is not fully supported yet. For more information read the documentation
found in README.cmake.

Command line options
~~~~~~~~~~~~~~~~~~~~

QDevelop may be launched with some parameters:

QDevelop [-l translation] [file(s)] [project]

-l translation
	When QDevelop starts, it detect automatically the language of installed system. If you want to use another language, use this option.
	Available languages are English, French, German and Dutch.

[file(s)] Open the files in editors. 

[project] Open the (.pro) file in QDevelop. The other files are closed before open the project.

Documentation
~~~~~~~~~~~~~

Up to date documentation can be found at:
http://biord-software.org/qdevelop

License
~~~~~~~~~~~~~

Copyright (C) 2006-2010 Jean-Luc Biord <jlbiord@gmail.com>

QDevelop is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
