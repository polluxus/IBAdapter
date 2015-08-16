#ifndef POLLUXUSUTILITY_H
#define POLLUXUSUTILITY_H
#include <QString>
#include <QDir>

inline QString getIniFilePath()
{
    QString iniFilePath = "workspace.ini";
    iniFilePath = QDir::currentPath() + "/" + iniFilePath;
    return iniFilePath;
};

#endif // POLLUXUSUTILITY_H

