#-------------------------------------------------
#
# Project created by QtCreator 2013-05-22T23:05:36
#
#-------------------------------------------------

QT       += core network sql

QT       -= gui

TARGET = FormozaSRV
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    connection.cpp \
    database.cpp

HEADERS += \
    connection.hpp \
    database.hpp
