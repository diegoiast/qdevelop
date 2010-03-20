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
#ifndef DEBOGAGE_H
#define DEBOGAGE_H

#include <QThread>
#include <QStringList>
#include <QPair>
#include <QProcess>
#include "projectmanager.h"
//
struct Variable;
class BlockUserData;
class RegistersImpl;
//
class Debug : public QThread
{
Q_OBJECT
public:
	Debug(QObject * parent, RegistersImpl *registersImpl, QString gdbName, Parameters p, QString exe, bool exeOnly=false);
    void run();
	enum Request { None, Registers, InfoSources, InfoScope, Local, OtherArgs, Whatis, Address, Length, Value};
private:
	// Methods
	void writeMessagesToDebugger();
	void launchDebug();
	void executeWithoutDebug();
	void configureGdb();
	void setEnvironment(QProcess *process);
	// Variables
	QString executableName;
	QString m_gdbName;
	QStringList messagesToDebugger;
	QProcess *processDebug;
	int m_pid;
	bool m_executeWithoutDebug;
	bool m_infoSources;
	Parameters m_parameters;
	Request m_request;
	QList<Variable> listVariables, listVariablesToSend;
	QStringList m_otherVariables;
	QObject * m_parent;
	RegistersImpl *m_registersImpl;
signals:
	void message(QString);
	void endDebug();
	void onPause();
	void debugVariables( QList<Variable> );
public slots:
	void slotBreakpoint(QString filename, unsigned int line, BlockUserData *);
protected slots:
	void slotMessagesDebug();
	void slotMessagesError();
	void slotDebugCommand(QString text);
	void slotStopDebug();
	void slotPauseDebug();
	void slotOtherVariables(QStringList list);
};
//
struct Variable
{
	Debug::Request kind;
	QString name;
	QString type;
	int length;
	QString content;
	QString address;
	int pos;
};

#endif
