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
#ifndef TREEPROJECT_H
#define TREEPROJECT_H

#include <QTreeWidget>
#include <QPointer>
//
class ProjectManager;
//
class TreeProject : public QTreeWidget
{
	friend class Delegator;
Q_OBJECT
public:
	TreeProject(QWidget * parent = 0);
	~TreeProject();
	void setProjectManager( QPointer<ProjectManager> g ) { m_projectManager = g; };
	bool showAsKrawek() { return krawekItemDelegate != 0; };
private:
	void setup();
protected:
	void mousePressEvent( QMouseEvent * event );
	void dragEnterEvent(QDragEnterEvent *);
	void drawBranches ( QPainter * painter, const QRect & rect, const QModelIndex & index ) const;
	void resizeEvent ( QResizeEvent * event );
	void keyPressEvent( QKeyEvent * event );
private:
	QPointer<ProjectManager> m_projectManager;
	QTreeWidgetItem *m_itemClicked;
	QAbstractItemDelegate *normalItemDelegate, *krawekItemDelegate;
private slots:
	void slotAddNewItem();
	void slotAddNewClass();
	void slotAddExistingFiles();
	void slotAddScope();
	void slotRenameItem();
	void slotDeleteItem();
	void slotAddSubProject();
	void slotProjectPropertie();
	void slotlupdate();
	void slotlrelease();
	void slotOpen();
	void slotSort();
	void slotSubclassing();
	void slotShowAsNormal();
	void slotPreviewForm();
public slots:
	void slotShowAsKrawek();
signals:
	void addNewItem(QTreeWidgetItem *, QString);
	void addExistingsFiles(QTreeWidgetItem *);
	void addNewClass(QTreeWidgetItem *);
	void deleteItem(QTreeWidgetItem *);
	void renameItem(QTreeWidgetItem *);
	void addScope(QTreeWidgetItem *);
	void addSubProject(QTreeWidgetItem *);
	void projectPropertie(QTreeWidgetItem *);
	void open(QTreeWidgetItem *, int);
	void lupdate(QTreeWidgetItem *);	
	void lrelease(QTreeWidgetItem *);
	void subclassing(QTreeWidgetItem *);	
	void previewForm(QTreeWidgetItem *);
	void sort();
};

#endif
