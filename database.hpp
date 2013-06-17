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

#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QVector>

struct Dane
{
    QString imie;
    QString nazwisko;
    QString stopien;
    QString funkcja;
};

struct GroupInfo
{
    QString grupaOpis;
    QVector<QString> users;
    QVector<QString> ogloszenia;
    QHash<QString, QString> tematyOgloszen;
    QHash<QString, QString> tekstyOgloszen;
};

struct Wiadomosc
{
    QString sender;
    QString message;
    QString date;
};

struct Powiadomienie
{
    QString type;
    QString message;
};

class Database : public QObject
{
    Q_OBJECT

public:
    Database();
    ~Database();
    bool validateLogin(QString login, QString password);
    Dane getDaneRecord(QString login);
    QVector<QPair<QString, QString> > getGroupList(QString login);
    GroupInfo getGroupInfo(QString grupa);
    QVector<QString> getAllUsers();
    QVector<Wiadomosc> getChatLog(QString user, QString secondPerson);
    QVector<Powiadomienie> getAndRemoveUserNotifications(QString user);


    void addChatMessage(QString sender, QString receiver, QString wiadomosc);
    void addNotification(QString type, QString receiver, QString message);

private:
    QSqlDatabase db;
};


#endif // DATABASE_HPP
