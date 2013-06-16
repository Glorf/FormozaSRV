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

#include "database.hpp"
#include <iostream>
#include <QDateTime>

Database::Database():
    db(QSqlDatabase::addDatabase("QSQLITE"))
{
    db.setDatabaseName("/var/formoza/db.sqlite");
    db.open();
}

Database::~Database()
{}

bool Database::validateLogin(QString login, QString password)
{
    QString q(QString("SELECT haslo FROM dane WHERE dane.login='" + login + QString("'")));
    QSqlQuery query(q, db);

    if(!query.next())
        return 0;
    if(query.value(0).toString()==password)
        return 1;
    else
        return 0;
}

Dane Database::getDaneRecord(QString login)
{
    Dane d;
    QString q(QString("SELECT imie,nazwisko,stopien,funkcja FROM dane WHERE dane.login='" + login + QString("'")));
    QSqlQuery query(q, db);
    query.next();
    d.imie = query.value(0).toString();
    d.nazwisko = query.value(1).toString();
    d.stopien = query.value(2).toString();
    d.funkcja = query.value(3).toString();

    return d;
}

QVector<QPair<QString, QString> > Database::getGroupList(QString login)
{
    QString q(QString("SELECT dbname, nazwa, zamknieta FROM grupy"));
    QSqlQuery query(q, db);
    QVector<QPair<QString, QString> > vector;
    while(query.next())
    {
        QPair<QString, QString> pair;
        pair.first = query.value(0).toString();
        pair.second = query.value(1).toString();
        if(query.value(2).toString()=="0")
            vector.push_back(pair);
        else
        {
            QString q2(QString("SELECT wartosc FROM ") + pair.first + QString(" WHERE ") + pair.first + QString(".wartosc='") + login +
                       QString("' AND ") + pair.first + QString(".rekord='user'"));
            QSqlQuery query2(q2, db);
            if(query2.next())
                vector.push_back(pair);
        }
    }
    return vector;
}

GroupInfo Database::getGroupInfo(QString grupa)
{
    QString q(QString("SELECT * FROM ") + grupa);
    QSqlQuery query(q, db);
    GroupInfo group;
    while(query.next())
    {
        if(query.value(0).toString()==QString("user"))
        {
            group.users.push_back(query.value(1).toString());
        }

        else if(query.value(0).toString()==QString("oglos"))
        {
            QString q2(QString("SELECT wartosc FROM ") + grupa + QString(" WHERE ") + grupa + QString(".rekord='") +
                       query.value(1).toString() + QString("_tytul'"));
            QSqlQuery query2(q2, db);
            if(query2.next())
            {
                query2.value(1).toString();
                group.tematyOgloszen.insert(query.value(1).toString(), query2.value(1).toString());
            }

            QString q3(QString("SELECT wartosc FROM ") + grupa + QString(" WHERE ") + grupa + QString(".rekord='") +
                       query.value(1).toString() + QString("_wartosc'"));
            QSqlQuery query3(q3, db);
            if(query3.next())
            {
                query3.value(1).toString();
                group.tekstyOgloszen.insert(query.value(1).toString(), query3.value(1).toString());
            }
        }
    }

    QString q4(QString("SELECT zamknieta, dodatkoweinfo FROM grupy WHERE grupy.dbname='")+ grupa + QString("'"));
    QSqlQuery query4(q4, db);
    query4.next();
    if(query4.value(0).toString()==QString("0"))
    {
        QString q5("SELECT login FROM dane");
        QSqlQuery query5(q5, db);

        while(query5.next())
        {
            group.users.push_back(query5.value(0).toString());
        }
    }

    group.grupaOpis = query4.value(1).toString();


    return group;
}

QVector<QString> Database::getAllUsers()
{
    QVector<QString> vector;
    QString q("SELECT login FROM dane");
    QSqlQuery query(q, db);
    while(query.next())
    {
        vector.push_back(query.value(0).toString());
    }

    return vector;
}

void Database::addChatMessage(QString sender, QString receiver, QString wiadomosc)
{
    QString q(QString("INSERT INTO wiadomosci VALUES ('") + sender + QString("','") + receiver + QString("','") +
              wiadomosc + QString("','") + QString::number(QDateTime::currentMSecsSinceEpoch ()) + QString("')"));

    QSqlQuery(q, db);
}

QVector<Wiadomosc> Database::getChatLog(QString user, QString secondPerson)
{
    QString q(QString("SELECT * FROM wiadomosci WHERE (wiadomosci.nadawca='") + user + QString("' AND wiadomosci.odbiorca='") +
              secondPerson + QString("') OR (wiadomosci.nadawca='") + secondPerson + QString("' AND wiadomosci.odbiorca='") +
              user + QString("')"));
    QSqlQuery query(q, db);

    QVector<Wiadomosc> vector;

    while(query.next())
    {
        Wiadomosc wiad;
        wiad.sender = query.value(0).toString();
        wiad.message = query.value(2).toString();
        wiad.date = query.value(3).toString();
        vector.push_back(wiad);
    }

    return vector;
}

