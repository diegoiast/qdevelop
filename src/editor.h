/*
* This file is part of QDevelop, an open-source cross-platform IDE
* Copyright (C) 2006  Jean-Luc Biord
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
* Contact e-mail: Jean-Luc Biord <jlbiord@qtfr.org>
* Program URL   : http://qtfr.org
*
*/
#ifndef EDITOR_H
#define EDITOR_H

#include "textEdit.h"
#include "mainimpl.h"
#include "ui_findwidget.h"
#include <QTextEdit>
#include <QPointer>
#include <QTextCursor>
#include <QScrollBar>
#include <QTimer>
//
typedef struct ReplaceOptions
{
	QStringList textFind;
	QStringList textReplace;
	bool caseSensitive;
	bool backwards;
	bool wholeWords;
	bool prompt;
};
//
//typedef QMap<QString, QStringList> classesMethodsList; 
//
class QToolButton;
class TabWidget;
class QComboBox;
class QTimer;
//
class Editor : public QWidget
{
Q_OBJECT
public:
	//enum Sauvegarde { Impossible, Ok, NonRequise };
	Editor(TabWidget *parent, MainImpl *mainimpl, InitCompletion *completion, QString name="");
	~Editor();
	bool open(bool silentMode);
	bool save();
	bool close();
	void setFilename(QString name) { m_filename = name; m_textEdit->document()->setModified(true);};
	QString filename() { return m_filename; };
	QString shortFilename();
	QString directory();
	static QString shortFilename(QString nomLong);
	static QString suffixe(QString filename);
	void find();
	void findContinue();
	QStringList classes();
	QStringList methodes(QString classe);
	QList<int> breakpoints() { return m_textEdit->breakpoints(); };
	void toggleBreakpoint(bool activate, int line);
	void deleteBreakpoint(int line) { m_textEdit->deleteBreakpoint(line); };
	void setBackgroundColor( QColor c ){ m_textEdit->setBackgroundColor(c); };
	void setCurrentLineColor( QColor c ){ m_textEdit->setCurrentLineColor(c); };
	void slotToggleBreakpoint();
	void setExecutedLine(int line);
	void emitListBreakpoints();
	void copy() { m_textEdit->copy(); };
	void cut() { m_textEdit->cut(); };
	void paste() { m_textEdit->paste(); };
	void undo() { m_textEdit->undo(); };
	void redo() { m_textEdit->redo(); };
	void comment(TextEdit::ActionComment action) { m_textEdit->comment(action); };
	void selectAll() { m_textEdit->selectAll(); };
	void slotIndent() { m_textEdit->slotIndent(); };
	void slotUnindent() { m_textEdit->slotUnindent(); };
	void dialogGotoLine() { m_textEdit->dialogGotoLine(); };
	void completeCode() { m_textEdit->completeCode(); };
	QString wordUnderCursor() { return m_textEdit->wordUnderCursor(); };
	QString classNameUnderCursor() { return m_textEdit->classNameUnderCursor(); };
	int verticalScrollBar() { return m_textEdit->verticalScrollBar()->value(); };
	void setVerticalScrollBar(int s) { m_textEdit->verticalScrollBar()->setValue(s); };
	void setAutobrackets(bool b) { m_textEdit->setAutobrackets(b); };
	bool isModified() { return m_textEdit->document()->isModified(); };
	void setFocus();
	void replace();
	void updateNumLines(int currentLine, int numLines);
	void setActiveEditor(bool b);
	void setIntervalUpdatingTreeClasses(int i) { m_intervalUpdatingClasses = i*1000;};
	void setShowTreeClasses(bool s);
	//
public slots:
	void gotoLine( int line, bool moveTop);
	void slotComboClasses(QString text=QString());
	void slotModifiedEditor(bool modified);
	void setLineNumbers(bool activate) { m_textEdit->activateLineNumbers(activate); };
	void setSelectionBorder(bool activate) { m_textEdit->setSelectionBorder(activate); };
	void setFont(QFont fonte) { m_textEdit->setFont(fonte); };
	void setEndLine(MainImpl::EndLine end) { m_textEdit->setEndLine(end); };
	void setAutoIndent(bool activate) { m_textEdit->setAutoIndent(activate); };
	void setSyntaxHighlight(bool activate);
	void setTabStopWidth(int size) { m_textEdit->setTabStopWidth(size) ; };
	void setAutoCompletion(bool activate) { m_textEdit->setAutoCompletion(activate) ; };
	void setTabSpaces(bool t) { m_textEdit->setTabSpaces(t) ; };
	void setSyntaxColors(QTextCharFormat a, QTextCharFormat b, QTextCharFormat c, QTextCharFormat d, QTextCharFormat e, QTextCharFormat f, QTextCharFormat g);
	void slotClassesMethodsList();	
	void slotOtherFile();
private slots:	
	void slotComboMethods(int index);
	void slotFindWidget_textChanged(QString text="", bool fromButton=false);
    void slotFindPrevious();
    void slotFindNext();
    void slotParseCtags();
    void slotTimerUpdateClasses();
    void slotTimerCheckIfModifiedOutside();
private:
	TabWidget *m_parent;
	QPointer<LineNumbers> m_lineNumbers;
	QTextCursor m_previousCursor;
	classesMethodsList m_classesMethodsList;
	QString m_findExp;
	QTextDocument::FindFlags m_findOptions;
	QComboBox *m_comboClasses, *m_comboMethods;
	QToolButton *m_otherFileButton;
	QToolButton *m_refreshButton;
	QString m_nameOtherFile;
	MainImpl *m_mainimpl;
    QWidget *m_findWidget;
	TextEdit *m_textEdit;
	QString m_filename;
	Ui::FindWidget uiFind;
    QTimer *autoHideTimer;
    bool m_backward;
	bool inQuotations(int position, QString text);
	ReplaceOptions m_replaceOptions;
    QProcess *testCtags;
    bool m_activeEditor;
    QTimer m_timerUpdateClasses;
    QTimer m_timerCheckLastModified;
    bool m_showTreeClasses;
    int m_intervalUpdatingClasses;
    quint16 m_checksum;
    InitCompletion *m_completion;
    QDateTime m_lastModified;
protected:
signals:
	void editorModified(Editor *, bool);
	void refreshClasses(QString);
	void breakpoint(QString, QPair<bool,unsigned int>); 
	void updateClasses(QString, QString);
};

#endif