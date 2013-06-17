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

#include "connection.hpp"
#include <QDebug>

Connection::Connection():
    mServer(new QTcpServer(this)),
    db(new Database())
{
    mServer->listen(QHostAddress::Any, 2000);
    connect(mServer, SIGNAL(newConnection()),this, SLOT(newClient()));
}

Connection::~Connection()
{
    delete mServer;
    delete db;
}

void Connection::newClient()
{
    Client *cli = new Client;
    cli->logged = false;
    QTcpSocket *connection = mServer->nextPendingConnection();
    clientMap.insert(connection, cli);
    connections.append(connection);
    connect(connection, SIGNAL(disconnected()), SLOT(disconn()));
    connect(connection, SIGNAL(readyRead()),	SLOT(receiveMessage()));
}

void Connection::disconn()
{
    QTcpSocket* disconn = static_cast<QTcpSocket*>(sender());
    Client *cli = clientMap.take(disconn);
    loginMap.take(cli->login);
    connections.removeAll(disconn);
    disconn->deleteLater();
    delete cli;
}

int Connection::receiveMessage()
{
    QTcpSocket* sock = static_cast<QTcpSocket*>(sender());
    Client *cli = clientMap.value(sock);
    QString received = QString(sock->readAll());
    QString okResponse("ok");
    QString ackResponse("ack");

    sock->write(ackResponse.toUtf8().data());
    sock->waitForBytesWritten();
    sock->waitForReadyRead();
    QString receivedArg = QString(sock->readAll());

    if(received==QString("log"))
        cli->login = receivedArg;
    else if(received==QString("pas"))
        cli->password = receivedArg;
    else if(received==QString("lin"))
    {
        if(db->validateLogin(cli->login, cli->password))
        {
            cli->logged = true;
            loginMap.insert(cli->login, sock);
        }
        else
            okResponse = QString("fail");
    }
    else if(received==QString("get")&&cli->logged)
    {
        if(receivedArg==QString("dan"))
        {
            Dane d = db->getDaneRecord(cli->login);

            sock->write(d.imie.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            sock->write(d.nazwisko.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            sock->write(d.stopien.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            sock->write(d.funkcja.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;
        }

        else if(receivedArg == QString("fup"))
        {
            QVector<Powiadomienie> vector = db->getAndRemoveUserNotifications(cli->login);

            QString ilosc = QString::number(vector.size());

            sock->write(ilosc.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            for(int i=0; i<vector.size(); i++)
            {
                sock->write(vector.at(i).type.toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;

                sock->write(vector.at(i).message.toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;
            }
        }

        else if(receivedArg==QString("gru"))
        {
            QVector<QPair<QString, QString> > grupy;
            grupy = db->getGroupList(cli->login);

            QString ilosc = QString::number(grupy.size());
            sock->write(ilosc.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            for(int i=0; i<grupy.size(); i++)
            {
                QPair<QString, QString> para = grupy.at(i);
                sock->write(para.second.toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;
                sock->write(para.first.toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;
            }
        }

        else if(receivedArg==QString("gdn"))
        {
            sock->write(ackResponse.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            QString grupa(sock->readAll());
            GroupInfo info = db->getGroupInfo(grupa);

            sock->write(info.grupaOpis.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            QString ilosc = QString::number(info.users.size());

            sock->write(ilosc.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            for(int i=0; i<info.users.size(); i++)
            {
                sock->write(info.users.at(i).toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;
            }

        }

        else if(receivedArg==QString("usr"))
        {
            QVector<QString> vector = db->getAllUsers();

            QString ilosc = QString::number(vector.size());

            sock->write(ilosc.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            for(int i=0; i<vector.size(); i++)
            {
                sock->write(vector.at(i).toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();

                if(QString(sock->readAll())!=ackResponse)
                    return 0;
            }
        }

        else if(receivedArg==QString("wia"))
        {
            sock->write(ackResponse.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            QString person(sock->readAll());

            QVector<Wiadomosc> vector = db->getChatLog(cli->login, person);

            QString ilosc = QString::number(vector.size());

            sock->write(ilosc.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return 0;

            for(int i=0; i<vector.size(); i++)
            {
                sock->write(vector.at(i).sender.toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;

                sock->write(vector.at(i).message.toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;

                sock->write(vector.at(i).date.toUtf8().data());
                sock->waitForBytesWritten();
                sock->waitForReadyRead();
                if(QString(sock->readAll())!=ackResponse)
                    return 0;
            }
        }
    }

    else if(received==QString("set")&&cli->logged)
    {
        if(receivedArg==QString("wia"))
        {
            sock->write(ackResponse.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            QString receiver(sock->readAll());

            sock->write(ackResponse.toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            QString message(sock->readAll());

            db->addChatMessage(cli->login, receiver, message);

            QPair<QString, QVector<QString> > para;
            para.first = QString("wia");
            para.second.push_back(cli->login);
            para.second.push_back(message);
            sendMessage(receiver, para);
        }

    }

    sock->write(okResponse.toUtf8().data());
    sock->waitForBytesWritten();
    sock->waitForReadyRead();
    QString finish = QString(sock->readAll());

    if(finish==QString("end"))
        return 1;
    return 0;
}

void Connection::sendMessage(QString user, QPair<QString, QVector<QString> > para)
{
    QString okResponse("ok");
    QString ackResponse("ack");

    if(loginMap.find(user) == loginMap.end())
        db->addNotification(QString("wia"), user, para.second.at(0));

    else
    {
        QTcpSocket *sock = loginMap.value(user);

        disconnect(sock, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

        sock->write("upd");
        sock->waitForBytesWritten();
        sock->waitForReadyRead();
        if(QString(sock->readAll())!=ackResponse)
            return;

        sock->write(para.first.toUtf8().data());
        sock->waitForBytesWritten();
        sock->waitForReadyRead();
        if(QString(sock->readAll())!=ackResponse)
            return;

        QString ilosc = QString::number(para.second.size());

        sock->write(ilosc.toUtf8().data());
        sock->waitForBytesWritten();
        sock->waitForReadyRead();
        if(QString(sock->readAll())!=ackResponse)
            return;

        for(int i=0; i<para.second.size(); i++)
        {
            sock->write(para.second.at(i).toUtf8().data());
            sock->waitForBytesWritten();
            sock->waitForReadyRead();
            if(QString(sock->readAll())!=ackResponse)
                return;
        }

        sock->write(QString("end").toUtf8().data());
        sock->waitForBytesWritten();
        sock->waitForReadyRead();
        if(QString(sock->readAll())!=okResponse)
            return;

        connect(sock, SIGNAL(readyRead()),	SLOT(receiveMessage()));
    }

}
