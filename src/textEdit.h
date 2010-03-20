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
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QPointer>
#include <QTextCursor>
#include "mainimpl.h"
#include "InitCompletion.h"
#include "ui_findwidget.h"
#include "ui_replacewidget.h"
//
typedef QMap<QString, QStringList> classesMethodsList;
//
class LineNumbers;
class SelectionBorder;
class Editor;
class FindImpl;
class CppHighlighter;
class TextEditInterface;
class QPixmap;

//
class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
	QString tempFilename();
	void saveAsTemp();
    TextEdit(Editor * parent, MainImpl *mainimpl, InitCompletion *completion);
    ~TextEdit();
    bool open(bool silentMode, QString filename, QDateTime &lastModified);
    bool save(QString filename, QDateTime &lastModified);
    bool close(QString filename);
    QTextCursor getLineCursor( int line ) const;
    void setLineNumbers( LineNumbers* );
    LineNumbers* lineNumbers();
    void setSelectionBorder( SelectionBorder* );
    SelectionBorder* selectionBorder();
    void findText();
    void setExecutedLine(int line);
    void selectLines(int debut, int fin);
    QString wordUnderCursor(const QPoint & pos=QPoint(), bool select=false);
    QString wordUnderCursor(const QString text);
    QString firstWordUnderCursor();
    QString classNameUnderCursor(const QPoint & pos=QPoint(), bool addThis=true);
    void activateLineNumbers(bool activate);
    void setSelectionBorder(bool activate);
    void setSyntaxHighlight(bool activate );
    void setAutoIndent( bool activate )
    {
        m_autoindent = activate;
    };
    void setAutoCompletion( bool activate )
    {
        m_autoCompletion = activate;
    };
    void setTabStopWidth(int taille);
    void setSyntaxColors(QTextCharFormat a, QTextCharFormat b, QTextCharFormat c, QTextCharFormat d, QTextCharFormat e, QTextCharFormat f, QTextCharFormat g);
    void setEndLine(MainImpl::EndLine end)
    {
        m_endLine = end;
    };
    void setTabSpaces(bool t)
    {
        m_tabSpaces = t;
    };
    void setAutobrackets(bool b)
    {
        m_autobrackets = b;
    };
    // Divius: comments autoclose
    void setAutocomments(bool b)
    {
        m_autocomments = b;
    };
    //
    void setMatch(bool b)
    {
        m_match = b;
    };
    void setHighlightCurrentLine(bool b)
    {
        m_highlightCurrentLine = b;
    }
    void setBackgroundColor( QColor c );
	void setTextColor(QColor c);
    void setCurrentLineColor( QColor c );
    void setMatchingColor( QColor c )
    {
        m_matchingColor = c;
    };
    void setShowWhiteSpaces( bool b )
    {
        m_showWhiteSpaces = b;
    }
    void setRightMarginColumn( int c )
    {
        m_rightMarginColumn = c;
    }
    void setWordWrap( bool w )
    {
    	if (w) setLineWrapMode(QTextEdit::WidgetWidth); else setLineWrapMode(QTextEdit::NoWrap);
   	}
    int currentLineNumber(QTextCursor cursor=QTextCursor());
    int currentLineNumber(QTextBlock block);
    int linesCount();
    void dialogGotoLine();
    void completeCode();
    void setFocus(Qt::FocusReason reason);
    void gotoMatchingBracket();
    void textPlugin(TextEditInterface *iTextEdit);
    void print();
    void printWhiteSpacesAndMatching( QPainter &p );

    enum ActionComment
    {
        Toggle, Comment, Uncomment
    };
    void insertText(QString text, int insertAfterLine);
    void setMouseHidden( bool hidden );
public slots:
    void gotoLine( int line, bool moveTop );
    void slotFind(Ui::FindWidget uiFind, Ui::ReplaceWidget uiReplace, QString ttf=0,  QTextDocument::FindFlags options=0, bool fromButton=false);
    void slotIndent(bool indent=true);
    void slotUnindent();
    void comment(ActionComment action);
    void slotGotoImplementation();
    void slotGotoDeclaration();
private slots:
    void slotAdjustSize();
    void slotWordCompletion(QListWidgetItem *item);
    void slotCursorPositionChanged();
    void slotCompletionList(TagList tagList);
    void slotCompletionHelpList(TagList tagList);
    void slotToggleBreakpoint();
    void slotToggleBookmark();
private:
    void completionHelp();
    QString m_plainText;
    long m_matchingBegin;
    long m_matchingEnd;
    QPointer<LineNumbers> m_lineNumbers;
    QPointer<SelectionBorder> m_selectionBorder;
    CppHighlighter *cpphighlighter;
    QString m_findExp;
    int m_findOptions;
    FindImpl *m_findImpl;
    int lineNumber(QTextCursor cursor);
    int lineNumber(QTextBlock b);
    int lineNumber(QPoint point);
    void key_home(bool shift);
    void key_end(bool shift);
    QPoint mousePosition;
    Editor *m_editor;
    MainImpl *m_mainImpl;
    bool m_autoindent;
    void autoIndent();
    void autobrackets();
    // Divius: comments autoclose
    void autocomments(bool start = true);
    //
    void autoUnindent();
    void match();
    void clearMatch();
    MainImpl::EndLine m_endLine;
    bool m_tabSpaces;
    InitCompletion *m_completion;
    QListWidget *m_completionList;
    bool m_autoCompletion;
    bool m_autobrackets;
    // Divius: comments autoclose
    bool m_autocomments;
    //
    bool m_match;
    bool m_highlightCurrentLine;
    bool m_showWhiteSpaces;
    int m_rightMarginColumn;
    QColor m_backgroundColor;
    QColor m_currentLineColor;
    QColor m_matchingColor;
    QAction *actionToggleBreakpoint;
    int m_lineNumber;
    bool m_mouseHidden;
	QPixmap	m_tabPixmap;
	QPixmap m_spacePixmap;
protected:
    void resizeEvent( QResizeEvent* );
    void keyPressEvent ( QKeyEvent * event );
    void contextMenuEvent(QContextMenuEvent * e);
    void mouseDoubleClickEvent( QMouseEvent * event );
    void dropEvent( QDropEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void paintEvent ( QPaintEvent * event );
    void mouseMoveEvent( QMouseEvent * event );
signals:
    void editorModified(bool);
    void initParse(InitCompletion::Request, QString, QString, bool, bool, bool, QString);
};

#endif
