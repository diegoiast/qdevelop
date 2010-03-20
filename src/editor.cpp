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
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
#include "editor.h"
#include "mainimpl.h"
#include "lineedit.h"
#include "tabwidget.h"
#include "linenumbers.h"
#include "logbuild.h"

#include <QComboBox>
#include <QTextCursor>
#include <QTextBlock>
#include <QDialog>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QGridLayout>
#include <QSpacerItem>
#include <QPair>
#include <QToolButton>
#include <QTimer>
#include <QProcess>
#include "cpphighlighter.h"
#include <QTextDocumentFragment>
#include <QFileInfo>
#include <QDir>
#include <QDebug>


Editor::Editor(TabWidget * parent, MainImpl *mainimpl, InitCompletion *completion, QString name)
        : QWidget(parent)
{
    m_parent = parent;
    m_mainimpl = mainimpl;
    m_completion = completion;
    m_filename =  name;

    int vposLayout = 0;
    m_comboClasses = m_comboMethods = 0;
    m_otherFileButton = m_refreshButton = 0;
    m_editorToolbar = 0;
    m_textEdit = new TextEdit(this, mainimpl, completion);
    m_backward = false;
    m_activeEditor = false;
    m_nameOtherFile = "";
    m_hideFindReplace = 10;
    //
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    if ( suffixe( m_filename ).toLower() == "cpp" || suffixe( m_filename ).toLower() == "cc" || suffixe( m_filename ).toLower() == "c" || 
         suffixe( m_filename ).toLower() == "h" || suffixe( m_filename ).toLower() == "hpp" 
       )
    {

        QHBoxLayout *hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setMargin(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        //
        m_otherFileButton = new QToolButton(this);
        connect(m_otherFileButton, SIGNAL(clicked()), this, SLOT(slotOtherFile()));
        hboxLayout->addWidget(m_otherFileButton);
        //
        if  ( (suffixe( m_filename ).toLower() != "h") && (suffixe( m_filename ).toLower() != "hpp") )
        {
            m_refreshButton = new QToolButton(this);
            m_refreshButton->setIcon(QIcon(":/toolbar/images/refresh.png"));
            m_refreshButton->setToolTip( tr("Refresh classes and methods lists") );
            connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(slotClassesMethodsList()) );
            hboxLayout->addWidget(m_refreshButton);
            //
            m_comboClasses = new QComboBox(this);
            m_comboClasses->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            m_comboClasses->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            m_comboClasses->setLineEdit( new LineEdit(m_comboClasses) );
            m_comboClasses->setEditable( true );
            m_comboClasses->setAutoCompletion( true );
            m_comboClasses->setObjectName(QString::fromUtf8("m_comboClasses"));
            connect(m_comboClasses, SIGNAL(activated(QString)), this, SLOT(slotComboClasses(QString)) );
            hboxLayout->addWidget(m_comboClasses);
            //
            m_comboMethods = new QComboBox(this);
            m_comboMethods->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            m_comboMethods->setSizeAdjustPolicy(QComboBox::AdjustToContents);

            m_comboMethods->setMaximumSize( 500, m_comboMethods->height());
            m_comboMethods->setLineEdit( new LineEdit(m_comboClasses) );
            m_comboMethods->setEditable( true );
            m_comboMethods->setAutoCompletion( true );
            m_comboMethods->setMaxVisibleItems( 25 );
            m_comboMethods->setObjectName(QString::fromUtf8("comboMethodes"));
            m_comboMethods->lineEdit()->setAlignment(Qt::AlignLeft);
            connect(m_comboMethods, SIGNAL(activated(int)), this, SLOT(slotComboMethods(int)) );
            hboxLayout->addWidget(m_comboMethods);
        }
        //
        QSpacerItem *spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        hboxLayout->addItem(spacerItem);
        //
        //
        updateOtherFile(m_filename); // Update "other file" button
        m_otherFileButton->setIcon(QIcon(m_otherFileIcon));
        m_editorToolbar = new QWidget( this );
        m_editorToolbar->setLayout( hboxLayout );
        gridLayout->addWidget(m_editorToolbar, vposLayout++, 0, 1, 1);
    }
    connect(m_textEdit, SIGNAL(editorModified(bool)), this, SLOT(slotModifiedEditor(bool)) );
    //
    m_findWidget = new QWidget;
    uiFind.setupUi(m_findWidget);
    connect(uiFind.toolClose, SIGNAL(clicked()), m_findWidget, SLOT(hide()) );
    connect(uiFind.editFind, SIGNAL(textChanged(QString)), this, SLOT(slotFindWidget_textChanged(QString)) );
    connect(uiFind.editFind, SIGNAL(returnPressed()), this, SLOT(slotFindNext()) );
    connect(uiFind.toolPrevious, SIGNAL(clicked()), this, SLOT(slotFindPrevious()) );
    connect(uiFind.toolNext, SIGNAL(clicked()), this, SLOT(slotFindNext()) );
    //
    autoHideTimer = new QTimer(this);
    //
    m_replaceWidget = new QWidget;
    uiReplace.setupUi(m_replaceWidget);
    uiReplace.replace->hide();
    connect(uiReplace.toolClose, SIGNAL(clicked()), m_replaceWidget, SLOT(hide()) );
    connect(uiReplace.textFind, SIGNAL(editTextChanged(QString)), uiFind.editFind, SLOT(setText(QString)) );
    connect(uiReplace.textFind, SIGNAL(editTextChanged(QString)), this, SLOT(slotFindWidget_textChanged(QString)) );
    connect(uiReplace.textReplace, SIGNAL(editTextChanged(QString)), autoHideTimer, SLOT(start()) );
    connect(uiReplace.toolPrevious, SIGNAL(clicked()), this, SLOT(slotFindPrevious()) );
    connect(uiReplace.toolNext, SIGNAL(clicked()), this, SLOT(slotFindNext()) );
    connect(uiReplace.replace, SIGNAL(clicked()), this, SLOT(slotReplace()) );
    connect(uiReplace.checkCase, SIGNAL(toggled(bool)), uiFind.checkCase, SLOT(setChecked(bool)) );
    connect(uiReplace.checkWholeWords, SIGNAL(toggled(bool)), uiFind.checkWholeWords, SLOT(setChecked(bool)) );
    //
    autoHideTimer->setInterval(10000);
    autoHideTimer->setSingleShot(true);
    connect(autoHideTimer, SIGNAL(timeout()), m_findWidget, SLOT(hide()));
    connect(autoHideTimer, SIGNAL(timeout()), m_replaceWidget, SLOT(hide()));
    //
    gridLayout->addWidget(m_textEdit, vposLayout++, 0, 1, 1);
    gridLayout->addWidget(m_findWidget, vposLayout++, 0, 1, 1);
    gridLayout->addWidget(m_replaceWidget, vposLayout++, 0, 1, 1);
    uiFind.labelWrapped->setVisible(false);
    uiReplace.labelWrapped->setVisible(false);
    m_findWidget->hide();
    m_replaceWidget->hide();
    //
    connect(&m_timerUpdateClasses, SIGNAL(timeout()), this, SLOT(slotTimerUpdateClasses()));
    connect(&m_timerCheckLastModified, SIGNAL(timeout()), this, SLOT(slotTimerCheckIfModifiedOutside()));
}
//
void Editor::updateOtherFile(QString currentFile)
{
    if ( !m_otherFileButton ) return;
    QString suffix;
    QString base = currentFile.left(currentFile.lastIndexOf(".") );
    m_nameOtherFile = "";
    if ( suffixe( currentFile ).toLower() == "h" || suffixe( currentFile ).toLower() == "hpp" )
        suffix = "cpp";
    else if ( suffixe( m_filename ).toLower() == "cpp" || suffixe( m_filename ).toLower() == "cc"  || suffixe( m_filename ).toLower() == "c")
        suffix = "h";
    // if the current file has one of the above suffixes and the other file exists, use it
    if (!suffix.isEmpty() && QFileInfo(base+"."+suffix).exists()) {
        m_nameOtherFile = base+"."+suffix;
        m_otherFileButton->setVisible(true);
        m_otherFileButton->setToolTip( tr("Open %1").arg(shortFilename(m_nameOtherFile)) );
        m_otherFileIcon = ":/treeview/images/"+suffix+".png";
        m_otherFileButton->setIcon(QIcon(m_otherFileIcon));
    }
    else // otherwise, don't display the button
        m_otherFileButton->setVisible(false);

    emit otherFileChanged();
}
//
void Editor::setActiveEditor(bool b)
{
    m_activeEditor = b;
    if ( b && m_showTreeClasses )
    {
        m_timerUpdateClasses.start(m_intervalUpdatingClasses);
        if ( m_completion )
        {
            m_completion->disconnect( m_textEdit );
            connect(m_completion, SIGNAL(completionList(TagList)), m_textEdit, SLOT(slotCompletionList(TagList)) );
            connect(m_completion, SIGNAL(completionHelpList(TagList)), m_textEdit, SLOT(slotCompletionHelpList(TagList)) );
        }
    }
    else
    {
        slotTimerUpdateClasses();
        m_timerUpdateClasses.stop();
        if ( m_completion )
            m_completion->disconnect( m_textEdit );
    }
}
//
void Editor::setShowTreeClasses(bool s)
{
    m_showTreeClasses=s;
    if ( !m_showTreeClasses )
        m_timerUpdateClasses.stop();
}
//
void Editor::slotTimerUpdateClasses()
{
    QByteArray array( m_textEdit->toPlainText().toLocal8Bit() );
    if ( !array.count() )
        return;
    char *ptr = array.data();
    quint16 check = qChecksum(ptr, array.length());
    if ( check != m_checksum )
    {
        m_checksum = check;
        emit updateClasses( filename(), m_textEdit->toPlainText());
        m_mainimpl->slotCompile(true);
    }
    if( m_textEdit->hasFocus() )
    	m_textEdit->setMouseHidden( true );
    else
    	m_textEdit->setMouseHidden( false );
}
//
void Editor::slotTimerCheckIfModifiedOutside()
{
    QFile file(m_filename);
    if ( !file.exists() )
    {
    	m_timerCheckLastModified.stop();
    	int rep = QMessageBox::question(this, "QDevelop",
                                        tr("The file \"%1\"\ndisappeared from fisk.\n\n").arg(m_filename)+
                                        tr("What do you want to do?"),
                                        tr("Recreate"), tr("Close") );
        if ( rep == 0 ) // Recreate
        {
            m_textEdit->document()->setModified( true );
            save();
            m_timerCheckLastModified.start( 5000 );
        }
        else if ( rep == 1 ) // Closing
        {
            QList<Editor *> editors = m_mainimpl->allEditors();
            for (int i = 0; i < editors.count(); i++)
            {
            	qDebug() << editors[i]->filename() << filename(); 
            	if (editors[i]->filename() == filename())
            	{
            		// TODO: Not very good idea to delete object during it's event handler!
            		m_mainimpl->closeTab(i);
            		return;
           		}
           	}
        }
   	}
   	else if ( m_lastModified != QFileInfo( file ).lastModified() )
    {
        m_timerCheckLastModified.stop();
        int rep = QMessageBox::question(this, "QDevelop",
                                        tr("The file \"%1\"\nwas modified outside editor.\n\n").arg(m_filename)+
                                        tr("What do you want to do?"),
                                        tr("Overwrite"), tr("Reload File") );
        if ( rep == 0 ) // Overwrite
        {
            m_textEdit->document()->setModified( true );
            save();
        }
        else if ( rep == 1 ) // Reload
        {
            open(false);
        }
        m_timerCheckLastModified.start( 5000 );
    }
    checkBookmarks();
}
//
void Editor::checkBookmarks()
{
    QMenu *menu = m_mainimpl->bookmarksMenu();
    QList<QAction *> actions = menu->actions();
    foreach(QAction *action, actions)
    {
        Bookmark bookmark = action->data().value<Bookmark>();
        QTextBlock block = bookmark.second;
        bool found = false;
        for ( QTextBlock b = m_textEdit->document()->begin(); b.isValid(); b = b.next() )
        {
            BlockUserData *blockUserData = (BlockUserData*)block.userData();
            if ( block==b && blockUserData && blockUserData->bookmark )
            {
                found = true;
            }
        }
        if ( !found  )
        {
            m_mainimpl->toggleBookmark(this, "", false, block);
        }
    }
}
//
void Editor::clearAllBookmarks()
{
    int line = 1;
    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), line++ )
    {
        BlockUserData *blockUserData = (BlockUserData*)block.userData();
        if ( blockUserData && blockUserData->bookmark )
            toggleBookmark( line );
    }
}
//
Editor::~Editor()
{}
//
bool Editor::open(bool silentMode)
{
    bool ret = m_textEdit->open(silentMode, m_filename, m_lastModified);
    if ( ret && (suffixe( m_filename ).toLower() == "cpp" || suffixe( m_filename ).toLower() == "cc"))
        slotClassesMethodsList();
    QByteArray array( m_textEdit->toPlainText().toLocal8Bit() );
    if ( array.count() )
    {
        char *ptr = array.data();
        quint16 check = qChecksum(ptr, array.length());
        m_checksum = check;
    }
    m_timerCheckLastModified.start( 5000 );
    return ret;
}
//
bool Editor::close()
{
    bool ret = m_textEdit->close( m_filename );
    if ( ret )
        clearAllBookmarks();
    return ret;
}
//
void Editor::setSyntaxHighlight(bool activate)
{
    if ( activate && !QString(":c:cpp:cc:h:hpp:").contains( ":"+m_filename.section(".", -1, -1).toLower()+":" ) )
        return;
    m_textEdit->setSyntaxHighlight(activate);
}
//
bool Editor::save()
{
    return m_textEdit->save( m_filename, m_lastModified);
}
//
void Editor::gotoLine( int line, bool moveTop )
{
    m_textEdit->gotoLine( line, moveTop );
}
//
void Editor::slotFindPrevious()
{
    m_backward = true;
    if ( m_textEdit->textCursor().selectedText().length() )
    {
    	QTextCursor cursor = m_textEdit->textCursor();
	    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, cursor.selectedText().length());
	    cursor.movePosition(QTextCursor::NoMove, QTextCursor::KeepAnchor);
	    m_textEdit->setTextCursor(cursor);
   	}
    slotFindWidget_textChanged(uiFind.editFind->text(), true);
}
//
void Editor::slotFindNext()
{
    m_backward = false;
    slotFindWidget_textChanged(uiFind.editFind->text(), true);
}
//
void Editor::find()
{
    autoHideTimer->stop();
    m_replaceWidget->hide();
    m_findWidget->show();
    uiFind.editFind->setFocus(Qt::ShortcutFocusReason);
    if ( m_textEdit->textCursor().selectedText().length() )
        uiFind.editFind->setText( m_textEdit->textCursor().selectedText() );
    else
        uiFind.editFind->setText( m_textEdit->wordUnderCursor() );
    uiFind.editFind->selectAll();
    if (m_hideFindReplace > 0)
    {
    	autoHideTimer->setInterval(m_hideFindReplace*1000);
    	autoHideTimer->start();
   	}
}
//
void Editor::slotReplace()
{
	bool replaced;
	int nbFound = 0;
	do
	{
		replaced = false;
		if( ( 
				m_textEdit->textCursor().selectedText() == uiReplace.textFind->currentText() 
				|| ( m_textEdit->textCursor().selectedText().toLower() == uiReplace.textFind->currentText().toLower() && !uiReplace.checkCase->isChecked() )
			)
			&& uiReplace.textFind->currentText() != uiReplace.textReplace->currentText())
		{
			m_textEdit->textCursor().removeSelectedText();
			m_textEdit->textCursor().insertText( uiReplace.textReplace->currentText() );
			replaced = true;
			nbFound++;
			if( m_backward )
				slotFindPrevious();
			else
				slotFindNext();
		}
	} while( replaced && !uiReplace.prompt->isChecked() );
    uiReplace.reached->setText(tr("%1 replacement(s) made.").arg(nbFound));
    uiReplace.labelWrapped->setVisible( true );
	if( uiReplace.textFind->findText( uiReplace.textFind->currentText() ) == -1 )
		uiReplace.textFind->addItem(  uiReplace.textFind->currentText() );
	if( uiReplace.textReplace->findText( uiReplace.textReplace->currentText() ) == -1 )
		uiReplace.textReplace->addItem(  uiReplace.textReplace->currentText() );
}
//
void Editor::replace()
{
    /*ReplaceImpl *dialog = new ReplaceImpl(this, m_textEdit, m_replaceOptions);
    dialog->exec();
    m_replaceOptions = dialog->replaceOptions();
    delete dialog;*/
    m_findWidget->hide();
    uiReplace.labelWrapped->setVisible(false);
    autoHideTimer->stop();
    m_replaceWidget->show();
    uiReplace.textFind->setFocus(Qt::ShortcutFocusReason);
    if ( m_textEdit->textCursor().selectedText().length() )
        uiReplace.textFind->setEditText( m_textEdit->textCursor().selectedText() );
    else
        uiReplace.textFind->setEditText( m_textEdit->wordUnderCursor() );
    uiReplace.textFind->lineEdit()->selectAll();
    if (m_hideFindReplace > 0)
    {
    	autoHideTimer->setInterval(m_hideFindReplace*1000);
    	autoHideTimer->start();
   	}
}

//return current selected text or word under cursor
QString Editor::selection()
{
    if ( m_textEdit->textCursor().selectedText().length() )
        return ( m_textEdit->textCursor().selectedText() );
    else
        return ( m_textEdit->wordUnderCursor() );
}

//
void Editor::setFocus()
{
    m_findWidget->hide();
    m_replaceWidget->hide();
    m_textEdit->setFocus(Qt::OtherFocusReason);
}
//
void Editor::findContinue()
{
    //slotFindWidget_textChanged(uiFind.editFind->text(), true);
    slotFindNext();
}
//
void Editor::findPrevious()
{
    slotFindPrevious();
}
//
void Editor::slotFindWidget_textChanged(QString text, bool fromButton)
{
    int options = 0;
    autoHideTimer->stop();
    if ( m_backward )
        options |= QTextDocument::FindBackward;
    if ( uiFind.checkWholeWords->isChecked() )
        options |= QTextDocument::FindWholeWords;
    if ( uiFind.checkCase->isChecked() )
        options |= QTextDocument::FindCaseSensitively;
    m_textEdit->slotFind(uiFind, uiReplace, text, (QTextDocument::FindFlags)options,fromButton);
    if( m_textEdit->textCursor().selectedText().length() )
    	uiReplace.replace->setVisible( true );
    else
    	uiReplace.replace->setVisible( false );
    if (m_hideFindReplace > 0)
    {
    	autoHideTimer->setInterval(m_hideFindReplace*1000);
    	autoHideTimer->start();
   	}
}
//
QStringList Editor::classes()
{
    QStringList liste = m_classesMethodsList.keys();
    liste.sort();
    return liste;
}
//
QStringList Editor::methodes(QString classe)
{
    QStringList liste;
    foreach(QString line, m_classesMethodsList.value( classe ) )
    liste += QStringList(line.section("::", 1, 1)+"::"+ line.section("::", 0, 0));
    liste.sort();
    QStringList liste2;
    foreach(QString ligne2, liste)
    liste2 += QStringList(ligne2.section("::", 1, 1)+"::"+ ligne2.section("::", 0, 0));
    return liste2;
}
//
void Editor::slotClassesMethodsList()
{
    if ( !m_comboClasses ) return;
    int width = 0;
    const QList<ParsedItem> *list;
    list = m_mainimpl->treeClassesItems();
    if ( list->isEmpty() )
        return;
    m_comboClasses->clear();
    m_classesMethodsList.clear();
    for (int i = 0; i < list->size(); ++i)
    {
        ParsedItem parsedItem = list->at( i );
        if ( ( parsedItem.kind == "p" || parsedItem.kind == "f" )
                && parsedItem.implementation.section("|", 0, 0) == filename() )
        {
            QString numLine = parsedItem.implementation.section("|", -1, -1);
            QString returnName = parsedItem.ex_cmd;
            if ( returnName.left( returnName.indexOf("::") ).indexOf("\t") != -1 )
                returnName = returnName.left( returnName.indexOf("::") ).section("\t", 0, 0);
            else if ( returnName.left( returnName.indexOf("::") ).indexOf(" ") != -1 )
                returnName = returnName.left( returnName.indexOf("::") ).section(" ", 0, 0);
            else
                returnName = "";
            QString add = returnName+" " +parsedItem.classname+"::"+parsedItem.name+parsedItem.signature+QChar(255)+numLine;
            QStringList methodes = m_classesMethodsList.value(parsedItem.classname);
            methodes << add;
            if ( !parsedItem.classname.simplified().isEmpty() )
            {
                width = qMax(width, fontMetrics().width( parsedItem.classname ) );
            }
            m_classesMethodsList[parsedItem.classname] = methodes;
        }
    }
    m_comboClasses->setGeometry(m_comboClasses->x(), m_comboClasses->y(), qMin(350, width+30), m_comboClasses->height());
    slotComboClasses();
    return;
}
//
bool Editor::inQuotations(int position, QString text)
{
    int debutQuote = 0, finQuote;
    bool realBegin = false;
    do
    {
        do
        {

            debutQuote = text.indexOf("\"", debutQuote);
            if ( debutQuote > 0 && (text.at(debutQuote-1) == '\\' || text.at(debutQuote-1) == '\'') )
            {
                debutQuote++;
                realBegin = false;
            }
            else
            {
                realBegin = true;
            }
        }
        while (!realBegin);
        finQuote = -1;
        if ( debutQuote != -1 )
        {
            finQuote = debutQuote+1;
            bool realEnd = false;
            do
            {
                finQuote = text.indexOf("\"", finQuote);
                if ( finQuote > 0 && text.at(finQuote-1) == '\\' )
                {
                    finQuote++;
                    realEnd = false;
                }
                else
                {
                    realEnd = true;
                }
            }
            while (!realEnd);
        }
        if ( debutQuote!=-1 && finQuote!=-1 )
        {
            if ( position > debutQuote && position < finQuote )
            {
                return true;
            }
        }
        debutQuote = finQuote+1;
    }
    while ( debutQuote!=-1 && finQuote!=-1 );
    // Idem with "'"
    debutQuote = 0, finQuote=0;
    realBegin = false;
    do
    {
        do
        {

            debutQuote = text.indexOf('\'', debutQuote);
            if ( debutQuote > 0 && (text.at(debutQuote-1) == '\\' || text.at(debutQuote-1) == '\"') )
            {
                debutQuote++;
                realBegin = false;
            }
            else
            {
                realBegin = true;
            }
        }
        while (!realBegin);

        finQuote = -1;
        if ( debutQuote != -1 )
        {
            finQuote = debutQuote+1;
            bool realEnd = false;
            do
            {
                finQuote = text.indexOf('\'', finQuote);
                if ( finQuote > 0 && (text.at(finQuote-1) == '\\' || text.at(finQuote-1) == '\"') )
                {
                    finQuote++;
                    realEnd = false;
                }
                else
                {
                    realEnd = true;
                }
            }
            while (!realEnd);
        }
        if ( debutQuote!=-1 && finQuote!=-1 )
        {
            if ( position > debutQuote && position < finQuote )
            {
                return true;
            }
        }
        debutQuote = finQuote+1;
    }
    while ( debutQuote!=-1 && finQuote!=-1 );
    return false;
}
//
void Editor::toggleBookmark(int line)
{
    QTextCursor save = m_textEdit->textCursor();
    int scroll = verticalScrollBar();
    gotoLine( line, false );
    m_textEdit->textCursor().movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    m_textEdit->textCursor().movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QString s = m_textEdit->textCursor().block().text().simplified();
    //
    QTextCursor cursor = m_textEdit->textCursor();
    bool activate;
    BlockUserData *blockUserData = (BlockUserData*)cursor.block().userData();
    if ( !blockUserData )
    {
        blockUserData = new BlockUserData();
        blockUserData->breakpoint = false;
        blockUserData->bookmark = false;
        blockUserData->block = cursor.block();
    }
    blockUserData->bookmark = !blockUserData->bookmark;
    cursor.block().setUserData( blockUserData );
    activate = blockUserData->bookmark;
    m_textEdit->setTextCursor( cursor );
    //
    //emit bookmark(this, s, QPair<bool,QTextBlock>(activate, cursor.block()));
    m_mainimpl->toggleBookmark(this, s, activate, cursor.block());
    m_textEdit->setTextCursor( save );
    setVerticalScrollBar( scroll );
    if( m_textEdit->lineNumbers() )
	    m_textEdit->lineNumbers()->update();
}
//
QList<int> Editor::bookmarksList()
{
    QList<int> list;
    int line = 1;
    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), line++ )
    {
        BlockUserData *blockUserData = (BlockUserData*)block.userData();
        if ( blockUserData && blockUserData->bookmark )
            list << line;
    }
    return list;
}
//
QList<QTextBlock> Editor::breakpointsList()
{
    QList<QTextBlock> list;
    int line = 1;
    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), line++ )
    {
        BlockUserData *blockUserData = (BlockUserData*)block.userData();
        if ( blockUserData && blockUserData->breakpoint )
            list << block;
    }
    return list;
}

void Editor::displayEditorToolbar( bool b )
{
	if (m_editorToolbar)
		m_editorToolbar->setVisible( b );
}
//
void Editor::toggleBreakpoint(int line, QString breakpointCondition, bool isTrue)
{
    QTextCursor save = m_textEdit->textCursor();
    int scroll = verticalScrollBar();
    gotoLine( line, false );
    QTextCursor cursor = m_textEdit->textCursor();
    BlockUserData *blockUserData = (BlockUserData*)cursor.block().userData();
    if ( !blockUserData )
    {
        blockUserData = new BlockUserData();
        blockUserData->breakpoint = false;
        blockUserData->bookmark = false;
        blockUserData->isTrue = false;
    }
   	blockUserData->breakpoint = !blockUserData->breakpoint;
    blockUserData->breakpointCondition = breakpointCondition;
    blockUserData->isTrue = isTrue;
    cursor.block().setUserData( blockUserData );
    m_textEdit->setTextCursor( cursor );
    m_textEdit->setTextCursor( save );
    setVerticalScrollBar( scroll );
    emit breakpoint(shortFilename(), line, blockUserData);
    if( m_textEdit->lineNumbers() )
	    m_textEdit->lineNumbers()->update();
}
//
void Editor::emitListBreakpoints()
{
    int line = 1;
    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), line++ )
    {
        BlockUserData *blockUserData = (BlockUserData*)block.userData();
        if ( blockUserData && blockUserData->breakpoint )
		    emit breakpoint(shortFilename(), line, blockUserData);
    }
}
//
QString Editor::shortFilename()
{
    QString name = m_filename;
    int pos = m_filename.lastIndexOf( "/" );
    if ( pos != -1 )
        name = name.mid(pos+1);
    return name;
}
//
QString Editor::directory()
{
    return m_filename.left( m_filename.length()-shortFilename().length()-1 );
}
QString Editor::shortFilename(QString nomLong)
{
    QString name = nomLong;
    int pos = nomLong.lastIndexOf( "/" );
    if ( pos != -1 )
        name = name.mid(pos+1);
    return name;
}
//
QString Editor::suffixe(QString filename)
{
    return filename.section(".", -1, -1);
}
//
void Editor::setExecutedLine(int line)
{
    m_textEdit->setExecutedLine(line);
}
//
void Editor::slotModifiedEditor(bool modified)
{
    emit editorModified(this, modified);
}
//
//
void Editor::slotComboClasses(QString text)
{
    if ( !m_comboClasses )
        return;
    if ( text.isEmpty() )
    {
        m_comboClasses->clear();
        m_comboClasses->addItems( classes() );
        m_comboClasses->setCurrentIndex(0);
        text = m_comboClasses->currentText();
    }
    if ( !classes().count() || (m_comboClasses->count() == 1 && m_comboClasses->itemText(0).simplified().isEmpty() ) )
        m_comboClasses->setHidden( true );
    else
        m_comboClasses->setHidden( false );
    m_comboMethods->setHidden( !classes().count() );
    m_comboMethods->clear();
    int width = 0;
    foreach(QString line, methodes(text) )
    {
        int numLine = QString(line.section(QChar(255), -1)).toInt();
        QString methode = line.left( line.indexOf(QChar(255)) );
        methode.remove('\r').remove('\n');
        QString typeRetour = " : " + methode.section(" ", 0, 0);
        if ( typeRetour.contains("::") )
            typeRetour = "";
        methode = methode.section("::", 1, 1);
        int last = methode.lastIndexOf(":");
        if ( last != 1 && !inQuotations(last, methode) )
            methode = methode.left( last );
        methode += typeRetour;
        QTextBlock block;
        int n=1;
        for (block = m_textEdit->document()->begin(); block.isValid() && n != numLine; block = block.next(), n++ )
            ;
        m_comboMethods->addItem(methode, QVariant(block.text()));
        width = qMax(width, fontMetrics().width( methode ) );
    }
    m_comboMethods->setGeometry(m_comboClasses->x()+m_comboClasses->width()+6, m_comboMethods->y(), qMin(500, width+30), m_comboMethods->height());
    m_comboMethods->setCurrentIndex(0);
}
//
void Editor::slotComboMethods(int index)
{
    m_textEdit->gotoLine(1, false);
    QString s = m_comboMethods->itemData(index).toString();
    m_textEdit->find( s );
    gotoLine( m_textEdit->currentLineNumber(), true);
}
//
void Editor::setSyntaxColors(QTextCharFormat a, QTextCharFormat b, QTextCharFormat c, QTextCharFormat d, QTextCharFormat e, QTextCharFormat f, QTextCharFormat g)
{

    m_textEdit->setSyntaxColors(a, b, c, d, e, f, g);
}
//
void Editor::slotOtherFile()
{
    m_mainimpl->openFile( QStringList(m_nameOtherFile) );
}
//
void Editor::setNameOtherFile(QString oldName, QString newName)
{
    /* When a file is renamed in files browser (by a right-click the "Rename..."), this function is called on
    all editors to rename the other file.
    */
    if ( oldName != m_nameOtherFile )
        return;
    updateOtherFile(newName);
}
//
void Editor::methodsList()
{
    if ( NULL == m_comboMethods )
    {
        QApplication::beep();
    }
    else
    {
        QLineEdit* pLineEdit = m_comboMethods->lineEdit();
        pLineEdit->setSelection(0, pLineEdit->text().count());
        m_comboMethods->showPopup();
        m_comboMethods->setFocus();
    }
}
//
void Editor::setTextColor(QColor textColor)
{
	m_textEdit->setTextColor(textColor);
}
//
void Editor::slotMessagesBuild(QString list, QString directory)
{
    /*  If your language is not translated in QDevelop and if g++ display the errors and warnings in your language, 
    modify the two strings below "error" and "warning" to adapt in your language. Also have a look at logbuild.cpp */
    bool messageContainsWarnings = false;
    bool messageContainsErrors = false;
    foreach(QString message, list.split("\n"))
    {
        if ( !message.isEmpty() )
        {
        	bool error = false;
            message.remove( "\r" );
		    QString filename;
		    uint numLine;
		    
		    if ( LogBuild::containsError(message, filename, numLine) )
            {
                // Error
                error = true;
                messageContainsErrors = true;
                if( message.toLower().contains("error") )
		            message = message.section("error", 1).simplified();
		        else if( message.toLower().contains( LogBuild::tr("error").toLower() ) )
		            message = message.section(LogBuild::tr("error"), 1).simplified();
            }
            else if ( LogBuild::containsWarning(message, filename, numLine) )
            {
                // Warning
                error = false;
                messageContainsWarnings = true;
                if( message.toLower().contains("warning") )
		            message = message.section("warning", 1).simplified();
		        else if( message.toLower().contains( LogBuild::tr("warning").toLower() ) )
		            message = message.section(LogBuild::tr("warning"), 1).simplified();
            }
            QString absoluteName = QDir(directory).absoluteFilePath(filename);
		    if( absoluteName.endsWith("-qdeveloptmp.cpp") )
		    	absoluteName = absoluteName.section("-qdeveloptmp.cpp", 0, 0) + ".cpp";
		    if( absoluteName != m_filename )
		    	continue;
		    
            if( message.startsWith(":") )
            	message = message.section(":", 1).simplified();
		    uint line = 1;
		    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), line++ )
		    {
		    	if( line != numLine )
		    		continue;
		        BlockUserData *blockUserData = (BlockUserData*)block.userData();
			    if ( !blockUserData )
			    {
			        blockUserData = new BlockUserData();
			        blockUserData->breakpoint = false;
			        blockUserData->bookmark = false;
			        blockUserData->block = block;
			    }
			    if( error && !blockUserData->errorString.contains(message) )
	        		blockUserData->errorString += message + "\n";
	        	else if( !error && !blockUserData->warningString.contains(message) )
	        		blockUserData->warningString += message + "\n";
			    block.setUserData( blockUserData );
			    break;
		    }
        }
    }
    if( m_textEdit->lineNumbers() )
	    m_textEdit->lineNumbers()->update();
	int state = 0;
    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next() )
    {
	    BlockUserData *blockUserData = (BlockUserData*)block.userData();
	    if ( !blockUserData )
	    {
	        blockUserData = new BlockUserData();
	        blockUserData->breakpoint = false;
	        blockUserData->bookmark = false;
	        blockUserData->block = block;
	    }
	    if( blockUserData && !blockUserData->errorString.isEmpty() )
	    {
	    	state = 2;
	    	break;
    	}
	    else if( blockUserData && !blockUserData->warningString.isEmpty() )
	    	state = 1;
    }
   	m_mainimpl->automaticCompilationState(this, state);
}


void Editor::clearErrorsAndWarnings()
{
    int line = 1;
    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), line++ )
    {
        BlockUserData *blockUserData = (BlockUserData*)block.userData();
        if ( blockUserData  )
        {
        	blockUserData->errorString = "";
        	blockUserData->warningString = "";
		    block.setUserData( blockUserData );
       	}
    }
    if( m_textEdit->lineNumbers() )
    	m_textEdit->lineNumbers()->update();
}


void Editor::slotEndBuild()
{
	QFile::remove( tempFilename() );
}


void Editor::nextWarningError()
{
    int line = 1;
    for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), line++ )
    {
        BlockUserData *blockUserData = (BlockUserData*)block.userData();
        if ( line>currentLineNumber() && blockUserData && (!blockUserData->errorString.isEmpty() || !blockUserData->warningString.isEmpty()) )
        {
        	gotoLine(line, true);
        	break;
       	}
    }
}


void Editor::previousWarningError()
{
    int line = currentLineNumber();
    for ( QTextBlock block = m_textEdit->textCursor().block(); block.isValid(); block = block.previous(), line-- )
    {
        BlockUserData *blockUserData = (BlockUserData*)block.userData();
        if ( line<currentLineNumber() && blockUserData && (!blockUserData->errorString.isEmpty() || !blockUserData->warningString.isEmpty()) )
        {
        	gotoLine(line, true);
        	break;
       	}
    }
}
//
