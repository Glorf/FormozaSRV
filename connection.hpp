/***********************************************************************************
 *  Copyright (C) 2013 Michał Bień.
 *
 *     This file is part of FormozaSRV.
 *
 *  FormozaSRV is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  FormozaSRV is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FormozaSRV; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ***********************************************************************************/

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QCryptographicHash>
#include <QBuffer>
#include <QHash>
#include <QList>
#include "database.hpp"

struct Client
{
    QString login;
    QString password;
    qint16 loopTimeout;
    bool logged;
};

class Connection: public QObject
{
    Q_OBJECT

private:
    QTcpServer *mServer;
    QHash<QTcpSocket*, Client*> clientMap;
    QHash<QString, QTcpSocket*> loginMap;
    QList<QTcpSocket*> connections;
    Database *db;

private slots:
    void newClient();
    void disconn();
    int receiveMessage();

public:
    explicit Connection();
    ~Connection();

    void setPort(quint16 port);
    void setAddress(QHostAddress address);
    void setName(QString name);
    void setVersion(qint16 version);
    void setMaxClients(qint16 max);
    void start();
    void sendMessage(QString user, QPair<QString, QVector<QString> > para);
};

#endif // CONNECTION_HPP
