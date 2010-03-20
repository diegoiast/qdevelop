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
/********************************************************************************************************
 * Une partie de cette classe fait partie du projet Monkey Studio
 * de Azevedo Filipe aka Nox PasNox ( pasnox at gmail dot com )
 ********************************************************************************************************/
#include "linenumbers.h"
#include "textEdit.h"
#include "editor.h"
//
#include <QTextEdit>
#include <QGridLayout>
#include <QScrollBar>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QDebug>
#include <QDialog>
#include "ui_breakpointcondition.h"
//
#define QD qDebug() << __FILE__ << __LINE__ << ":"
//
LineNumbers::LineNumbers( TextEdit* edit, Editor *editor)
	: QWidget( (QWidget *)edit ), m_textEdit( edit ), m_editor( editor )
{
	setObjectName( "editorZone" );
	setAutoFillBackground( true );
	connect( m_textEdit->document()->documentLayout(), SIGNAL( update( const QRectF& ) ), this, SLOT( update() ) );
	connect( m_textEdit->verticalScrollBar(), SIGNAL( valueChanged( int ) ), this, SLOT( update() ) );
	m_executedLine = 0;
	setDefaultProperties();	
	setMouseTracking( true );
}
//
void LineNumbers::paintEvent( QPaintEvent* )
{
	int contentsY = m_textEdit->verticalScrollBar()->value();
	qreal pageBottom = contentsY + m_textEdit->viewport()->height();
	int m_lineNumber = 1;
	const QFontMetrics fm = fontMetrics();
	const int ascent = fontMetrics().ascent() +1;
	//
	QPainter p( this );
	// need a hack to only browse the viewed block for big document
	for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid(); block = block.next(), m_lineNumber++ )
	{
		QTextLayout* layout = block.layout();
		const QRectF boundingRect = layout->boundingRect();
		QPointF position = layout->position();
		if ( position.y() +boundingRect.height() < contentsY )
			continue;
		if ( position.y() > pageBottom )
			break;
		const QString txt = QString::number( m_lineNumber );
		if( m_lineNumber == m_executedLine )
		{
			int centreV = qRound( position.y() ) -contentsY + 8;
			p.setBrush( Qt::blue );
			float x = width()-1;
			const QPointF points[7] = {
        			QPointF(x, centreV),
        			QPointF(x-9, centreV-8),
        			QPointF(x-9, centreV-4),
        			QPointF(x-15, centreV-4),
        			QPointF(x-15, centreV+4),
        			QPointF(x-9, centreV+4),
        			QPointF(x-9, centreV+8),
    			};

    			p.drawPolygon(points, 7);
		}
		else
			p.drawText( width() -fm.width( txt ) - 2, qRound( position.y() ) -contentsY +ascent, txt ); // -fm.width( "0" ) is an ampty place/indent 
		BlockUserData *blockUserData = (BlockUserData*)block.userData();

		if( blockUserData && blockUserData->breakpoint )
		{
			QPixmap pixmap = QPixmap(":/divers/images/breakpoint.png");
			if( !blockUserData->breakpointCondition.isEmpty() )
				pixmap = QPixmap(":/divers/images/breakpoint2.png");
			p.drawPixmap( 1, qRound( position.y() ) -contentsY-4, pixmap);
		}
		if( blockUserData && blockUserData->bookmark )
		{
			p.drawPixmap( 3, qRound( position.y() ) -contentsY-6,QPixmap(":/divers/images/bookmark.png"));
		}
		if( blockUserData && !blockUserData->errorString.isEmpty() )
		{
			p.drawPixmap( 3, qRound( position.y() ) -contentsY-2,QPixmap(":/divers/images/error.png") );
		}
		else if( blockUserData && !blockUserData->warningString.isEmpty() )
		{
			p.drawPixmap( 3, qRound( position.y() ) -contentsY-2,QPixmap(":/divers/images/warning.png") );
		}
	}
	p.end();
}
// PROPERTIES
void LineNumbers::setDigitNumbers( int i )
{
	if ( i == mDigitNumbers )
		return;
	mDigitNumbers = i;
	setFixedWidth( fontMetrics().width( "0" ) * (i) + 22 ); // +2 = 1 empty place before and 1 empty place after 
	//setFixedWidth( fontMetrics().width( "0" ) * ( mDigitNumbers +2 ) ); // +2 = 1 empty place before and 1 empty place after
	emit digitNumbersChanged();
}
//
int LineNumbers::digitNumbers() const
{
	return mDigitNumbers;
}
//
void LineNumbers::setTextColor( const QColor& c )
{
	if ( c == mTextColor )
		return;
	mTextColor = c;
	QPalette p( palette() );
	p.setColor( foregroundRole(), mTextColor );
	setPalette( p );
	emit textColorChanged( mTextColor );
}
//
const QColor& LineNumbers::textColor() const
{
	return mTextColor;
}
//
void LineNumbers::setBackgroundColor( const QColor& c )
{
	if ( c == mBackgroundColor )
		return;
	mBackgroundColor = c;
	QPalette p( palette() );
	p.setColor( backgroundRole(), mBackgroundColor );
	setPalette( p );
	emit backgroundColorChanged( mBackgroundColor );
}
//
const QColor& LineNumbers::backgroundColor() const
{
	return mBackgroundColor;
}
// END PROPERTIES
void LineNumbers::setDefaultProperties()
{
	// Default properties
	setFont( m_textEdit->font() );
	setBackgroundColor( QColor( "#ffffd2" ) );
	setTextColor( QColor( Qt::black ) );
	//setTextColor( QColor( "#aaaaff" ) );
	setDigitNumbers( 4 );
}
void LineNumbers::mouseMoveEvent ( QMouseEvent * event )
{
	m_cursor = m_textEdit->cursorForPosition( event->pos() );
	if( m_cursor.isNull() )
	{
		setToolTip("");
		return;
	}
	BlockUserData *blockUserData = (BlockUserData*)m_cursor.block().userData();
	QString tooltip;
	if( blockUserData && !blockUserData->errorString.isEmpty() )
	{
		tooltip = blockUserData->errorString.left(blockUserData->errorString.length()-1);
	}
	else if( blockUserData && !blockUserData->warningString.isEmpty() )
	{
		tooltip = blockUserData->warningString.left(blockUserData->warningString.length()-1);
	}
	else
		tooltip = "";
	setToolTip( tooltip );
	QWidget::mouseMoveEvent( event );
}
//
void LineNumbers::mousePressEvent ( QMouseEvent * event )
{
	m_cursor = m_textEdit->cursorForPosition( event->pos() );
	if( m_cursor.isNull() )
		return;
	m_currentLine = 1;
	for ( QTextBlock block = m_textEdit->document()->begin(); block.isValid() && block != m_cursor.block(); block = block.next(), m_currentLine++ )
		;
	if( event->button() == Qt::RightButton )
	{
		QMenu *menu = new QMenu(this);
		connect(menu->addAction(QIcon(":/divers/images/bookmark.png"), tr("Toogle Bookmark")), SIGNAL(triggered()), this, SLOT(slotToggleBookmark()) );
		connect(menu->addAction(QIcon(":/divers/images/breakpoint.png"), tr("Toogle Breakpoint")), SIGNAL(triggered()), this, SLOT(slotToggleBreakpoint()) );
		//
		BlockUserData *blockUserData = (BlockUserData*)m_cursor.block().userData();
		if( blockUserData && blockUserData->breakpoint )
		{
			menu->addSeparator();
			connect(menu->addAction(QIcon(":/divers/images/breakpoint2.png"), tr("Breakpoint Condition...")), SIGNAL(triggered()), this, SLOT(slotBreakpointCondition()) );
		}
		//
		menu->exec(event->globalPos());
		delete menu;
	}
	else if ( event->button() == Qt::MidButton )
		slotToggleBreakpoint();
	else
		slotToggleBookmark();
}
//
void LineNumbers::slotToggleBreakpoint() 
{ 
	m_editor->toggleBreakpoint( m_currentLine, QString(), true );
	repaint();
}
//
void LineNumbers::slotToggleBookmark() 
{ 
	m_editor->toggleBookmark( m_currentLine );
	repaint();
}
//
void LineNumbers::setExecutedLine(int ligne) 
{ 
	m_executedLine = ligne; 
	repaint();
}
//
void LineNumbers::slotResetExecutedLine()
{
	if( m_executedLine )
	{
		m_executedLine = 0;
		update();
	}
	else
		m_executedLine = 0;
}

void LineNumbers::slotBreakpointCondition()
{
	BlockUserData *blockUserData = (BlockUserData*)m_cursor.block().userData();
	if( m_cursor.isNull() )
		return;
	if( blockUserData && blockUserData->breakpoint )
	{
	    QDialog *condition = new QDialog;
	    Ui::BreakpointCondition ui;
	    ui.setupUi(condition);
	    ui.condition->setText( blockUserData->breakpointCondition );
	    ui.checkboxCondition->setChecked( !blockUserData->breakpointCondition.isEmpty() );
	    if( blockUserData->isTrue )
	        ui.isTrue->setChecked( true );
	    else
	        ui.hasChanged->setChecked( true );
	    if( condition->exec() == QDialog::Accepted )
	    {
			blockUserData->breakpointCondition = ui.condition->text();
			blockUserData->isTrue = ui.isTrue->isChecked();
		    m_cursor.block().setUserData( blockUserData );
			m_editor->toggleBreakpoint( m_currentLine, blockUserData->breakpointCondition, blockUserData->isTrue); // Disable breakpoint
			m_editor->toggleBreakpoint( m_currentLine, blockUserData->breakpointCondition, blockUserData->isTrue ); // Enable breakpoint
			repaint();
	   	}
	   	delete condition;
	}
}

