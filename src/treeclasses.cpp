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

#include "treeclasses.h"
#include "projectmanager.h"
#include "mainimpl.h"
#include "misc.h"
#include "ui_addnewclassvariable.h"
#include "addnewclassmethodimpl.h"
#include "addnewclassvariableimpl.h"
#include "addsetgetimpl.h"

#include <QMenu>
#include <QMouseEvent>
#include <QDir>
#include <QProcess>
#include <QVariant>
#include <QHeaderView>
#include <QMessageBox>
#include <QTreeWidgetItemIterator>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>


#include <QDebug>
//
TreeClasses::TreeClasses(QWidget * parent)
        : QTreeWidget(parent)
{
    header()->hide();
    m_treeClassesItems = new QList<ParsedItem>();
}
//
TreeClasses::~TreeClasses()
{
    delete m_treeClassesItems;
}
//
void TreeClasses::clear()
{
    QTreeWidget::clear();
}
//
void TreeClasses::updateClasses(QString filename, QString buffer, QStringList parents, QString ext)
{
    if ( !m_ctagsPresent || !QFile::exists(m_ctagsName) )
        return;
    while ( tempProcessMap.count() > 10 )
        qApp->processEvents();
    QProcess *process = new QProcess();
    connect(process, SIGNAL(finished(int , QProcess::ExitStatus)), this, SLOT(slotParseCtags()) );
    tempProcessMap[process].ext = ext;
    tempProcessMap[process].numTempFile = 0;
    tempProcessMap[process].filename = filename;
    QString f;
    do
    {
        tempProcessMap[process].numTempFile++;
        f = QDir::tempPath()+"/qdevelop_"+QString::number(tempProcessMap[process].numTempFile)+ext;
    }
    while ( QFile::exists( f ) );

    QFile tempFileRead( QDir::tempPath()+"/qdevelop_"+QString::number( tempProcessMap[process].numTempFile )+ext );
    if (!tempFileRead.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    tempFileRead.write( buffer.toLocal8Bit() );
    tempProcessMap[process].parents = parents;
    tempFileRead.close();
    //
    process->start(m_ctagsName, QStringList()<<"-f " + QDir::tempPath()+"/qdevelop_ctags"+ext+"."+QString::number(tempProcessMap[process].numTempFile) << "--fields=+z+k+a+S+l+n" << "--c++-kinds=+p"<< QDir::tempPath()+"/qdevelop_"+QString::number( tempProcessMap[process].numTempFile )+tempProcessMap[process].ext);
    //
}
//
void TreeClasses::slotParseCtags()
{
    QProcess *process = ((QProcess*)sender());
    process->waitForFinished( -1 );
    QString read;
    QFile file( QDir::tempPath()+"/qdevelop_ctags"+tempProcessMap[process].ext+"."+QString::number(tempProcessMap[process].numTempFile) );
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        return;
    read = file.readAll();
    file.close();
    //
    QString filenameRead = QDir::tempPath()+"/qdevelop_"+QString::number( tempProcessMap[process].numTempFile )+tempProcessMap[process].ext;
    if ( !QFile( filenameRead ).remove() )
        qDebug()<<"Unable to remove"<<QDir::tempPath()+"/qdevelop_"+QString::number( tempProcessMap[process].numTempFile )+tempProcessMap[process].ext;
    QString filenameWrite = QDir::tempPath()+"/qdevelop_ctags"+tempProcessMap[process].ext+"."+QString::number(tempProcessMap[process].numTempFile);
    if ( !QFile( filenameWrite ).remove() )
        qDebug()<<"Unable to remove" << QDir::tempPath()+"/qdevelop_ctags"+tempProcessMap[process].ext+"."+QString::number(tempProcessMap[process].numTempFile);
    QStringList parents = tempProcessMap[process].parents;
    QString filename = tempProcessMap[process].filename;
    QString ext = tempProcessMap[process].ext;
    tempProcessMap.remove( process );
    process->deleteLater();
    QDir dir(filename);
    dir.makeAbsolute();
    QString absolutePath = dir.path();
    QString pathHash = getPathHash(absolutePath);
    if ( read.isEmpty() )
        return;
    if ( topLevelItem(0) )
    {
        setSortingSymbols(topLevelItem(0), true, filename, ext, parents);
    }
    foreach(QString s, read.split("\n", QString::SkipEmptyParts) )
    {
        if ( !s.isEmpty() && s.simplified().at(0) == '!' )
            continue;
        //if( s.contains("typeref:") )
        //continue;
        s += '\t';
        s.replace("__anon", pathHash);
        ParsedItem parsedItem;
        parsedItem.parents = parents;
        parsedItem.markedForDelete = false;
        parsedItem.name = s.section("\t", 0, 0).simplified();
        //parsedItem.ex_cmd = s.section("/^", -1, -1).section("$/", 0, 0).simplified();
        QString ex_cmd = s.section("/^", -1, -1).section("\"", 0, 0).simplified();
        parsedItem.ex_cmd = ex_cmd;
        QString numline = s.section("line:", -1, -1).section("\t", 0, 0).simplified();
        if ( ext == QString(".cpp") )
            parsedItem.implementation = filename + "|" + numline;
        else
            parsedItem.declaration = filename + "|" + numline;
        parsedItem.language = s.section("language:", -1, -1).section("\t", 0, 0).simplified();
        if ( s.contains("class:") )
            parsedItem.classname = s.section("class:", -1, -1).section("\t", 0, 0).simplified();
        if ( s.contains("struct:") )
            parsedItem.structname = s.section("struct:", -1, -1).section("\t", 0, 0).simplified();
        if ( s.contains("namespace:") )
            parsedItem.structname = s.section("namespace:", -1, -1).section("\t", 0, 0).simplified();
        if ( s.contains("enum:") )
            parsedItem.enumname = s.section("enum:", -1, -1).section("\t", 0, 0).simplified();
        if ( s.contains("access:") )
            parsedItem.access = s.section("access:", -1, -1).section("\t", 0, 0).simplified();
        parsedItem.signature = s.section("signature:", -1, -1).section("\t", 0, 0).simplified();
        parsedItem.kind = s.section("kind:", -1, -1).section("\t", 0, 0).simplified();
        //m_parsedItemsList.append( parsedItem );
        parse(parsedItem);
    }
    //m_parsedItemsList.clear();
    if ( topLevelItem(0) )
    {
        m_listDeletion.clear();
        deleteMarked( topLevelItem(0) );
        for (int i=0; i<m_listDeletion.count(); i++)
            delete m_listDeletion.at(i);
        m_listDeletion.clear();
        setItemExpanded(topLevelItem(0), true );
        sortItems(0, Qt::AscendingOrder);
        m_treeClassesItems->clear();
        setSortingSymbols(topLevelItem(0), false, QString(), QString(), QStringList());
    }
}
//
void TreeClasses::parse(ParsedItem parsedItem)
{
    // Begin parsing
    if ( parsedItem.name.simplified().isEmpty() )
        return;
    QTreeWidgetItem *itemParent = topLevelItem(0);
    int level = 0;
    foreach(QString s, parsedItem.parents)
    {
        itemParent = findAndCreate(itemParent, "", s, "parent:"+QString::number(level++)+":"+s, false, false, ParsedItem());
    }
    if ( parsedItem.classname.count() || parsedItem.structname.count() || parsedItem.enumname.count() )
    {
        QString text;
        QString pixname;
        if ( parsedItem.classname.count() )
        {
            text = parsedItem.classname;
            pixname = "class";
        }
        else if ( parsedItem.structname.count() )
        {
            text = parsedItem.structname;
            pixname = "struct";
        }
        else if ( parsedItem.enumname.count() )
        {
            text = parsedItem.enumname;
            pixname = "enum";
        }
        foreach(QString classname, text.split("::", QString::SkipEmptyParts) )
        {
            //itemParent = findAndCreate(itemParent, QString(),  parsedItem.classname, "class:"+classname, true, false, ParsedItem());
            itemParent = findAndCreate(itemParent, pixname,  classname, "class:"+classname, true, false, ParsedItem());
        }
    }
    if ( parsedItem.kind == "c" ) // class
    {
        findAndCreate(itemParent, "class", parsedItem.name, "class:"+parsedItem.name, true, true, parsedItem);
    }
    else if ( parsedItem.kind == "p" ) // function prototype in declaration (header)
    {
        QString pixname;
        if ( !parsedItem.access.isEmpty() )
            pixname = parsedItem.access+"_meth";
        findAndCreate(itemParent, pixname, parsedItem.name+parsedItem.signature, "function:"+parsedItem.name+signature(parsedItem.signature), false, true, parsedItem);
    }
    else if ( parsedItem.kind == "f" ) // function in implementation (sources)
    {
        QString pixname;
        if ( !parsedItem.access.isEmpty() )
            pixname = parsedItem.access+"_meth";
        else if ( parsedItem.classname.isEmpty() )
            pixname = "global_meth";
        findAndCreate(itemParent, pixname, parsedItem.name+parsedItem.signature, "function:"+parsedItem.name+signature(parsedItem.signature), false, true, parsedItem);
    }
    else if ( parsedItem.kind == "m" ) // member
    {
        findAndCreate(itemParent, parsedItem.access+"_var", parsedItem.name, "variable:"+parsedItem.name, false, true, parsedItem);
    }
    else if ( parsedItem.kind == "v" ) // variable
    {
        QString pixname;
        if ( parsedItem.classname.isEmpty() )
            pixname = "global_var";
        findAndCreate(itemParent, pixname, parsedItem.name, "variable:"+parsedItem.name, false, true, parsedItem);
    }
    else if ( parsedItem.kind == "s" ) // struct
    {
        QString pixname;
        pixname = "struct";
        findAndCreate(itemParent, pixname, parsedItem.name, "class:"+parsedItem.name, false, true, parsedItem);
    }
    else if ( parsedItem.kind == "g" ) // enum name
    {
        QString pixname;
        pixname = "public_enum";
        if ( !parsedItem.access.isEmpty() )
            pixname = parsedItem.access+"_enum";
        findAndCreate(itemParent, pixname, parsedItem.name, "class:"+parsedItem.name, false, true, parsedItem);
    }
    else if ( parsedItem.kind == "e" ) // enum value
    {
        QString pixname;
        if ( parsedItem.classname.isEmpty() )
            pixname = "public_var";
        findAndCreate(itemParent, pixname, parsedItem.name, "enum:"+parsedItem.name, false, true, parsedItem);
    }
    else if ( parsedItem.kind == "t" ) // typedef
    {
        QString pixname;
        pixname = "typedef";
        findAndCreate(itemParent, pixname, parsedItem.name, "class:"+parsedItem.name, false, true, parsedItem);
    }
    else if ( parsedItem.kind == "n" ) // namespace
    {
        QString pixname;
        pixname = "namespace";
        findAndCreate(itemParent, pixname, parsedItem.name, "class:"+parsedItem.name, false, true, parsedItem);
    }
    // End parsing

}
//
void TreeClasses::setSortingSymbols(QTreeWidgetItem *it, bool active, QString filename, QString ext, QStringList parents)
{
    ParsedItem parsedItem = it->data(0, Qt::UserRole).value<ParsedItem>();
    m_treeClassesItems->append( parsedItem );
    if ( active )
    {
        it->setText(0, markForSorting(parsedItem.kind, it->text(0)) );
        markForDeletion(it, filename, ext, parents);
    }
    else
    {
        it->setText(0, it->text(0).section('|', -1, -1) );
        if ( parsedItem.markedForDelete || it->text(0).simplified().isEmpty() )
        {
            //delete current;
            m_listDeletion.append( it );
            return;
        }
    }
    for (int i=0; i<it->childCount(); i++)
    {
        setSortingSymbols( it->child( i ), active, filename, ext, parents);
    }
}
//
void TreeClasses::markForDeletion(QTreeWidgetItem *current, QString filename, QString ext, QStringList parents )
{
    ParsedItem parsedItem = current->data(0, Qt::UserRole).value<ParsedItem>();
    parsedItem.markedForDelete = false;
    if ( ext==".cpp" && parsedItem.implementation.section("|", 0, 0) == filename && parsedItem.parents == parents)
    {
        if ( !parsedItem.declaration.toLower().endsWith(".h") )
        {
            parsedItem.markedForDelete = true;
        }
        parsedItem.implementation = "";
    }
    else if ( ext==".h" && parsedItem.declaration.section("|", 0, 0) == filename && parsedItem.parents == parents )
    {
        if ( !parsedItem.implementation.toLower().endsWith(".cpp") )
        {
            parsedItem.markedForDelete = true;
        }
        parsedItem.declaration = "";
    }
    else
        parsedItem.markedForDelete = false;
    QVariant v;
    v.setValue( parsedItem );
    current->setData(0, Qt::UserRole, v );
}
//
void TreeClasses::deleteMarked(QTreeWidgetItem *current)
{
    ParsedItem parsedItem = current->data(0, Qt::UserRole).value<ParsedItem>();
    if ( parsedItem.markedForDelete )
    {
        //delete current;
        m_listDeletion.append( current );
		//emit modifiedClasse( parsedItem.classname );
        return;
    }
    for (int i=0; i<current->childCount(); i++)
    {
        deleteMarked( current->child( i ));
    }
    return;
}
//
//
QTreeWidgetItem *TreeClasses::findAndCreate(QTreeWidgetItem *begin, QString pixname, QString text, QString key, bool recursive, bool update, ParsedItem parsedItem)
{
    QTreeWidgetItem *newItem = findItem(begin, text, key, recursive);
    if ( !newItem  )
    {
		//emit modifiedClasse( parsedItem.classname );
        if ( begin )
            newItem = new QTreeWidgetItem( begin );
        else
            newItem = new QTreeWidgetItem( this );
        parsedItem.key = key;
        parsedItem.markedForDelete = false;
        QVariant v;
        v.setValue( parsedItem );
        text = markForSorting(parsedItem.kind, text);
        newItem->setText(0, text);
        setTooltip(newItem, parsedItem);
        newItem->setData(0, Qt::UserRole, v );
        if ( !pixname.isEmpty() )
        {
            newItem->setIcon(0, QIcon(":/CV/images/CV"+pixname+".png"));
            parsedItem.icon = ":/CV/images/CV"+pixname+".png";
        }
    }
    if ( update )
    {
        ParsedItem oldParsedItem = newItem->data(0, Qt::UserRole).value<ParsedItem>();
        if ( parsedItem.declaration.isEmpty() && !oldParsedItem.declaration.isEmpty() )
            parsedItem.declaration = oldParsedItem.declaration;
        if ( parsedItem.implementation.isEmpty()  && !oldParsedItem.implementation.isEmpty() )
            parsedItem.implementation = oldParsedItem.implementation;
        if ( parsedItem.access.isEmpty()  && !oldParsedItem.access.isEmpty() )
            parsedItem.access = oldParsedItem.access;
        parsedItem.icon = oldParsedItem.icon;
        text = markForSorting(parsedItem.kind, text);
        setTooltip(newItem, parsedItem);
        newItem->setText(0, text);
        parsedItem.key = key;
        if ( !pixname.isEmpty() )
        {
            newItem->setIcon(0, QIcon(":/CV/images/CV"+pixname+".png"));
            parsedItem.icon = ":/CV/images/CV"+pixname+".png";
        }
        parsedItem.markedForDelete = false;
        QVariant v;
        v.setValue( parsedItem );
        newItem->setData(0, Qt::UserRole, v );
    }
    return newItem;
}
//
void TreeClasses::setTooltip(QTreeWidgetItem *item, ParsedItem parsedItem)
{
    if ( parsedItem.key.indexOf( "parent:" ) != 0 )
    {
        QString ex_cmd = parsedItem.ex_cmd;
        //if( parsedItem.kind == "p" || parsedItem.kind == "f" || parsedItem.kind == "c" )
        //{
        ex_cmd = ex_cmd.section(";", 0, 0).simplified();
        ex_cmd = ex_cmd.section("$", 0, 0).simplified();
        ex_cmd = ex_cmd.section("{", 0, 0).simplified();
        //}
        //else
        //ex_cmd = ex_cmd.section(";", 0, 0).simplified();
        QString tooltip;
        if ( !parsedItem.access.isEmpty() )
            tooltip = '['+parsedItem.access+']'+" ";
        tooltip += ex_cmd;
        item->setToolTip(0, tooltip);
    }
}
//
QString TreeClasses::markForSorting(QString kind, QString text )
{
    if ( text.contains( '|' ) )
        text = text.section('|', -1, -1);
    if ( kind == "c" )
        text = "A|"+text;
    else if ( kind == "f" || kind == "p" )
        text = "B|"+text;
    else if ( kind == "s" )
        text = "C|"+text;
    else if ( kind == "g" )
        text = "D|"+text;
    else if ( kind == "t" )
        text = "E|"+text;
    else if ( kind == "m" || kind == "v" || kind == "e" )
        text = "F|"+text;
    return text;
}
//
QTreeWidgetItem *TreeClasses::findItem(const QTreeWidgetItem *begin, const QString text, const QString key, const bool recursive)
{
    QTreeWidgetItem *newItem = 0;
    QTreeWidgetItem *it = (QTreeWidgetItem *)begin;
    if ( it == 0 )
        it = topLevelItem( 0 );
    if ( it )
    {
        ParsedItem parsedItem = it->data(0, Qt::UserRole).value<ParsedItem>();
        if ( parsedItem.key == key )
        {
            return it;
        }
        else
        {
            for (int i=0; i<it->childCount(); i++)
            {
                ParsedItem parsedItem = it->child( i )->data(0, Qt::UserRole).value<ParsedItem>();
                if ( parsedItem.key == key )
                    return it->child( i );
                else if ( recursive )
                    newItem = findItem(it->child( i ), text, key, recursive);
            }
        }
    }
    return newItem;
}
//
void TreeClasses::mousePressEvent( QMouseEvent * event )
{
    m_itemClicked = itemAt( event->pos() );
    if ( event->button() == Qt::RightButton )
    {
        QMenu *menu = new QMenu(this);
        if ( m_itemClicked )
        {
            setCurrentItem( m_itemClicked );
            ParsedItem parsedItem = m_itemClicked->data(0, Qt::UserRole).value<ParsedItem>();
            if ( parsedItem.key.left(10) != "parent:" )
            {
                if ( !parsedItem.declaration.isEmpty() )
                    connect(menu->addAction(QIcon(":/treeview/images/h.png"), tr("Open Declaration")), SIGNAL(triggered()), this, SLOT(slotOpenDeclaration()) );
                if ( !parsedItem.implementation.isEmpty() )
                    connect(menu->addAction(QIcon(":/treeview/images/cpp.png"), tr("Open Implementation")), SIGNAL(triggered()), this, SLOT(slotOpenImplementation()) );
            }
            if ( parsedItem.kind == "c" )
            {
                connect(menu->addAction(QIcon(""), tr("Add Method...")), SIGNAL(triggered()), this, SLOT(slotAddNewClassMethod()) );
                connect(menu->addAction(QIcon(""), tr("Add Variable...")), SIGNAL(triggered()), this, SLOT(slotAddNewClassVariable()) );
            }
            else if ( parsedItem.kind == "m" )
            {
                connect(menu->addAction(QIcon(""), tr("Add get/set Methods...")), SIGNAL(triggered()), this, SLOT(slotAddGetSetMethod()) );
            }
        }
        else
        {
            menu->addSeparator();
        }
        connect(menu->addAction(QIcon(":/toolbar/images/refresh.png"), tr("Refresh contents")), SIGNAL(triggered()), this, SLOT(slotRefresh()) );
        menu->exec(event->globalPos());
        delete menu;
    }
    QTreeWidget::mousePressEvent(event);
}
//
void TreeClasses::slotOpenImplementation()
{
    ParsedItem parsedItem = m_itemClicked->data(0, Qt::UserRole).value<ParsedItem>();
    QString s = parsedItem.implementation;
    QString filename = s.section("|", 0, 0);
    int numLine = s.section("|", -1, -1).toInt();
    m_mainImpl->openFile(QStringList(filename) , numLine, false, true);
}
//
void TreeClasses::slotOpenDeclaration()
{
    ParsedItem parsedItem = m_itemClicked->data(0, Qt::UserRole).value<ParsedItem>();
    QString s = parsedItem.declaration;
    QString filename = s.section("|", 0, 0);
    int numLine = s.section("|", -1, -1).toInt();
    m_mainImpl->openFile(QStringList(filename) , numLine, false, true);
}
//
void TreeClasses::slotRefresh()
{
    if ( m_projectManager )
        m_projectManager->parseTreeClasses(true);
}
//
void TreeClasses::toDB(QString projectDirectory)
{
    if ( !topLevelItem(0) )
        return;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QSqlQuery query( QSqlDatabase::database( projectDirectory + "/" + SETTINGS_FILE_NAME ));
    QString queryString = "delete from classesbrowser where 1";
    if (!query.exec(queryString))
    {
        qDebug() << "Failed to execute" << queryString;
        return;
    }
    queryString = "delete from checksums where 1";
    if (!query.exec(queryString))
    {
        qDebug() << "Failed to execute" << queryString;
        return;
    }
    query.exec("BEGIN TRANSACTION;");
    writeItemsInDB(topLevelItem(0), QString(), query, projectDirectory);
    /** Here, for each file, a checksum is computed with his contents then 
    this checksum is saved in database. When the database is reloaded in fromDB() function, 
    the checksum is computed again. If the new checksum and the checksum in database are different, 
    the file was modified between the recording and the reloading (certainly by an external editor). 
    A new parsing for this file is required to populate correctly the classes browser.   
    */
    QStringList files;
    m_projectManager->sources( 0, files );
    m_projectManager->headers( 0, files );
    foreach(QString filename, files)
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QByteArray array( file.readAll() );
        if ( !array.count() )
            continue;
        char *ptr = array.data();
        quint16 check = qChecksum(ptr, array.length());
        queryString = "insert into checksums values(";
        queryString = queryString
                      + "'" + filename + "', "
                      + "'" + QString::number(check) + "')";
        bool rc = query.exec(queryString);
        if (rc == false)
        {
            qDebug() << "Failed to insert record to db" << query.lastError();
            qDebug() << queryString;
            exit(0);
        }
        file.close();
    }
    query.exec("END TRANSACTION;");
    QApplication::restoreOverrideCursor();
}
//
void TreeClasses::writeItemsInDB(const QTreeWidgetItem *it, QString parents, QSqlQuery query, QString projectDirectory)
{
    //
    ParsedItem parsedItem = it->data(0, Qt::UserRole).value<ParsedItem>();
    QString queryString = "insert into classesbrowser values(";
    queryString = queryString
                  + "'" + it->text(0).replace("'", "$") + "', "
                  + "'" + it->toolTip(0).replace("'", "$") + "', "
                  + "'" + parsedItem.icon.replace("'", "$") + "', "
                  + "'" + parsedItem.key.replace("'", "$") + "', "
                  + "'" + parents.replace("'", "$") + "', "
                  + "'" + parsedItem.name.replace("'", "$") + "', "
                  + "'" + QDir( projectDirectory ).relativeFilePath( QDir::cleanPath( parsedItem.implementation ) ).replace("'", "$") + "', "
                  + "'" + QDir( projectDirectory ).relativeFilePath( QDir::cleanPath( parsedItem.declaration ) ).replace("'", "$") + "', "
                  + "'" + parsedItem.ex_cmd.replace("'", "$") + "', "
                  + "'" + parsedItem.language.replace("'", "$") + "', "
                  + "'" + parsedItem.classname.replace("'", "$") + "', "
                  + "'" + parsedItem.structname.replace("'", "$") + "', "
                  + "'" + parsedItem.enumname.replace("'", "$") + "', "
                  + "'" + parsedItem.access.replace("'", "$") + "', "
                  + "'" + parsedItem.signature.replace("'", "$") + "', "
                  + "'" + parsedItem.kind.replace("'", "$") + "')";
    bool rc = query.exec(queryString);
    if (rc == false)
    {
        qDebug() << "Failed to insert record to db" << query.lastError();
        qDebug() << queryString;
        exit(0);
    }
    //
    for (int i=0; i<it->childCount(); i++)
    {
        writeItemsInDB( it->child( i ), parents+":"+it->text(0), query, projectDirectory);
    }
}
//
void TreeClasses::fromDB(QString projectDirectory)
{
    m_treeClassesItems->clear();
    connectDB(projectDirectory + "/" + SETTINGS_FILE_NAME);
    QSqlQuery query( QSqlDatabase::database( projectDirectory + "/" + SETTINGS_FILE_NAME ));
    query.exec("BEGIN TRANSACTION;");
    QString queryString = QString()
                          + "select * from classesbrowser where 1";
    query.exec(queryString);
    while (query.next())
    {
        ParsedItem parsedItem;
        QString text = query.value(0).toString().replace("$", "'");
        QString tooltip = query.value(1).toString().replace("$", "'");
        parsedItem.icon = query.value(2).toString().replace("$", "'");
        parsedItem.key = query.value(3).toString().replace("$", "'");
        QString parents = query.value(4).toString().replace("$", "'");
        parsedItem.name = query.value(5).toString().replace("$", "'");
        parsedItem.implementation = query.value(6).toString().replace("$", "'");
        parsedItem.implementation = QDir( projectDirectory ).absoluteFilePath(parsedItem.implementation  );
        parsedItem.declaration = query.value(7).toString().replace("$", "'");
        parsedItem.declaration = QDir( projectDirectory ).absoluteFilePath(parsedItem.declaration  );
        parsedItem.ex_cmd = query.value(8).toString().replace("$", "'");
        parsedItem.language = query.value(9).toString().replace("$", "'");
        parsedItem.classname = query.value(10).toString().replace("$", "'");
        parsedItem.structname = query.value(11).toString().replace("$", "'");
        parsedItem.enumname = query.value(12).toString().replace("$", "'");
        parsedItem.access = query.value(13).toString().replace("$", "'");
        parsedItem.signature = query.value(14).toString().replace("$", "'");
        parsedItem.kind = query.value(15).toString().replace("$", "'");
        createItemFromDB(topLevelItem(0), text, tooltip, parents, parsedItem);
        m_treeClassesItems->append( parsedItem );
    }
    queryString = QString()
                  + "select * from checksums where 1";
    query.exec(queryString);
    while (query.next())
    {
        QString filename = query.value(0).toString();
        quint16 checksum = query.value(1).toInt();
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QByteArray array( file.readAll() );
        if ( !array.count() )
            return;
        char *ptr = array.data();
        quint16 check = qChecksum(ptr, array.length());
        if ( check != checksum )
        {
            m_mainImpl->slotUpdateClasses(filename, QString(array));
        }
        file.close();
    }
    query.exec("END TRANSACTION;");
}
//
//
void TreeClasses::createItemFromDB(QTreeWidgetItem *parent, QString text, QString /*tooltip*/, QString parents, ParsedItem parsedItem)
{
    if ( !parent )
    {
        QTreeWidgetItem *it = new QTreeWidgetItem( this );
        it->setText(0, text);
        setTooltip(it, parsedItem);
        it->setIcon(0, QIcon(parsedItem.icon));
        QVariant v;
        v.setValue( parsedItem );
        it->setData(0, Qt::UserRole, v );
        setItemExpanded( it, true );
        return;
    }
    parents = parents.section(":", 2);
    if ( !parents.isEmpty() )
    {
        foreach( QString p, parents.split(":", QString::SkipEmptyParts) )
        {
// 			QTreeWidgetItem *find;
            for (int i=0; i<parent->childCount(); i++)
            {
                if ( parent->child( i )->text( 0 ) == p )
                    parent = parent->child( i );
            }
        }
    }
    QTreeWidgetItem *it = new QTreeWidgetItem( parent );
    it->setText(0, text);
    setTooltip(it, parsedItem);
    it->setIcon(0, QIcon(parsedItem.icon));
    QVariant v;
    v.setValue( parsedItem );
    it->setData(0, Qt::UserRole, v );
}
//
void TreeClasses::mouseDoubleClickEvent ( QMouseEvent * event )
{
    m_itemClicked = itemAt( event->pos() );
    if ( !m_itemClicked )
        return;
    ParsedItem parsedItem = m_itemClicked->data(0, Qt::UserRole).value<ParsedItem>();
    if ( parsedItem.key.left(10) != "parent:" )
    {
        if ( !parsedItem.implementation.isEmpty() && (parsedItem.kind!="c" || parsedItem.declaration.isEmpty() ))
            slotOpenImplementation();
        else if ( !parsedItem.declaration.isEmpty() )
            slotOpenDeclaration();
    }
    else
        setItemExpanded( m_itemClicked, !isItemExpanded(m_itemClicked) );
}
//
QString TreeClasses::signature(QString line)
{
    QString params = line.mid( line.indexOf("(")+1 );
    params = params.left( params.lastIndexOf(")") ) .simplified();
    QString formattedParams;
    foreach(QString param, params.split(",") )
    {
        param = param.simplified();
        if ( param.contains("&") )
            param = param.simplified().left( param.simplified().lastIndexOf("&")+1 );
        else if ( param.contains("*") )
            param = param.simplified().left( param.simplified().lastIndexOf("*")+1 );
        else if ( param.simplified().contains(" ") )
            param = param.simplified().left( param.simplified().indexOf(" ") );
        else if ( param.simplified().contains("=") )
            param = param.simplified().left( param.simplified().lastIndexOf("=") );
        formattedParams += param + ",";
    }
    formattedParams = formattedParams.simplified().left( formattedParams.lastIndexOf(",") );
    formattedParams.remove( " " );
    QString s ="(" + formattedParams + ")";
    return s;
}
//
QString TreeClasses::getPathHash(QString const& pathName)
{
    unsigned intHash = qHash(pathName);
    return QString().sprintf("_%x_", intHash);
}
//
//
void TreeClasses::slotAddNewClassMethod()
{
    ParsedItem parsedItem = m_itemClicked->data(0, Qt::UserRole).value<ParsedItem>();
    QString implementation = parsedItem.implementation;
    QString declaration = parsedItem.declaration;
    QString classname = parsedItem.name;
    if ( !QFileInfo(implementation.section("|", 0, 0)).isFile() )
    {
        for (int i=0; i<m_itemClicked->childCount(); i++)
        {
            ParsedItem childItem = m_itemClicked->child( i )->data(0, Qt::UserRole).value<ParsedItem>();
            if ( QFileInfo(childItem.implementation.section("|", 0, 0)).isFile() )
            {
                implementation = childItem.implementation;
                break;
            }
        }

    }
    AddNewClassMethodImpl *dialog = new AddNewClassMethodImpl(m_mainImpl, this, implementation, declaration, classname);
    dialog->exec();
    delete dialog;
    //
}
//
void TreeClasses::slotAddNewClassVariable()
{
    ParsedItem parsedItem = m_itemClicked->data(0, Qt::UserRole).value<ParsedItem>();
    QString implementation = parsedItem.implementation;
    QString declaration = parsedItem.declaration;
    QString classname = parsedItem.name;
    if ( !QFileInfo(implementation.section("|", 0, 0)).isFile() )
    {
        for (int i=0; i<m_itemClicked->childCount(); i++)
        {
            ParsedItem childItem = m_itemClicked->child( i )->data(0, Qt::UserRole).value<ParsedItem>();
            if ( QFileInfo(childItem.implementation.section("|", 0, 0)).isFile() )
            {
                implementation = childItem.implementation;
                break;
            }
        }

    }
    AddNewClassVariableImpl *dialog = new AddNewClassVariableImpl(m_mainImpl, this, m_itemClicked, declaration, implementation, classname);
    dialog->exec();
    delete dialog;
//
}
//
void TreeClasses::slotAddGetSetMethod()
{
    ParsedItem parsedItem = m_itemClicked->data(0, Qt::UserRole).value<ParsedItem>();
    ParsedItem parsedItemParent = m_itemClicked->parent()->data(0, Qt::UserRole).value<ParsedItem>();
    QTreeWidgetItem *m_itemClickedParent = m_itemClicked->parent();
    QString implementation = parsedItemParent.implementation;
    QString declaration = parsedItemParent.declaration;
    QString classname = parsedItemParent.name;
    if ( !QFileInfo(implementation.section("|", 0, 0)).isFile() )
    {
        for (int i=0; i<m_itemClickedParent->childCount(); i++)
        {
            ParsedItem childItem = m_itemClickedParent->child( i )->data(0, Qt::UserRole).value<ParsedItem>();
            if ( QFileInfo(childItem.implementation.section("|", 0, 0)).isFile() )
            {
                implementation = childItem.implementation;
                break;
            }
        }

    }
    QString type = parsedItem.ex_cmd.left( parsedItem.ex_cmd.lastIndexOf(" ") );
    AddSetGetImpl *addSetGetImpl = new AddSetGetImpl(m_mainImpl, this, declaration, implementation, classname, type, parsedItem.name);
    addSetGetImpl->exec();
    delete addSetGetImpl;
}
//
QStringList TreeClasses::methods(QString filename, QString /*classname*/)
{
    QStringList listMethods;
    const QList<ParsedItem> *list;
    list = treeClassesItems();
    //foreach( ParsedItem parsedItem, list )
    for (int i = 0; i < list->size(); ++i)
    {
        ParsedItem parsedItem = list->at( i );
        if ( ( parsedItem.kind == "p" || parsedItem.kind == "f" )
                && parsedItem.declaration.section("|", 0, 0) == filename )
        {
            QString returnName = parsedItem.ex_cmd;
            if ( returnName.left( returnName.indexOf("::") ).indexOf("\t") != -1 )
                returnName = returnName.left( returnName.indexOf("::") ).section("\t", 0, 0);
            else if ( returnName.left( returnName.indexOf("::") ).indexOf(" ") != -1 )
                returnName = returnName.left( returnName.indexOf("::") ).section(" ", 0, 0);
            else
                returnName = "";
            QString add = returnName+" " +parsedItem.name+parsedItem.signature;
            listMethods << add.simplified();
        }
    }
    return listMethods;
}
//
/*const QList<ParsedItem> *TreeClasses::treeClassesItems(QString classname)
{
	QTreeWidgetItem *parent = findItem(0, classname, "class:"+classname, false);
	if( parent )
	{
        ParsedItem parsedItem = parent->data(0, Qt::UserRole).value<ParsedItem>();
		list->append( parsedItem );
		for(int i=0; i<parent->childCount(); i++)
		{
			QTreeWidgetItem *childItem = parent->child( i );
	        ParsedItem parsedItem = childItem->data(0, Qt::UserRole).value<ParsedItem>();
			list->append( parsedItem );
		}
	}
	return list;
}*/
//
