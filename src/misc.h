/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006 - 2010 Jean-Luc Biord
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Jean-Luc Biord <jlbiord@gmail.com>
* Program URL   : http://biord-software.org/qdevelop/
*
*/
#ifndef DIVERS_H
#define DIVERS_H

#include <QString>
#include <QVariant>
#include <QTreeWidgetItem>


QVariant addressToVariant(void *it );
QTreeWidgetItem *variantToItem( QVariant variant );
QAction *variantToAction( QVariant variant );
bool connectDB(QString const& dbName);
QString getQDevelopPath(void);

#ifdef Q_OS_UNIX
#define SETTINGS_FILE_NAME ".qdevelop"
#else
#define SETTINGS_FILE_NAME "qdevelop-settings.db"
#endif

#endif
