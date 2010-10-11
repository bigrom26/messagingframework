/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Messaging Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "longstream_p.h"
#include "qmaillog.h"
#include <QCoreApplication>
#include "qmailnamespace.h"
#include <QIODevice>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>

#if defined(Q_OS_SYMBIAN)
#include <f32file.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#elif defined (Q_OS_MAC)
#include <sys/statvfs.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#include <errno.h>
#endif

/*  Helper class to reduce memory usage while downloading large mails */
LongStream::LongStream()
    : mStatus(Ok)
{
    QString tmpName( LongStream::tempDir() + QLatin1String( "longstream" ) );

    len = 0;
    appendedBytes = minCheck;

    tmpFile = new QTemporaryFile( tmpName + QLatin1String( ".XXXXXX" ));
    if (tmpFile->open()) {
        tmpFile->setPermissions(QFile::ReadOwner | QFile::WriteOwner);
        ts = new QDataStream( tmpFile );
    } else {
        qWarning() << "Unable to open temporary file:" << tmpFile->fileName();
        ts = 0;
        setStatus(OutOfSpace);
    }
}

LongStream::~LongStream()
{
    tmpFile->close();
    delete ts;
    delete tmpFile;
}

void LongStream::reset()
{
    delete ts;

    tmpFile->resize( 0 );
    tmpFile->close();
    tmpFile->open();

    ts = new QDataStream( tmpFile );
    len = 0;
    appendedBytes = minCheck;

    c = QChar::Null;
    resetStatus();
}

QString LongStream::detach()
{
    QString detachedName = fileName();

    delete ts;

    tmpFile->setAutoRemove(false);
    tmpFile->close();
    delete tmpFile;

    QString tmpName( LongStream::tempDir() + QLatin1String( "longstream" ) );

    tmpFile = new QTemporaryFile( tmpName + QLatin1String( ".XXXXXX" ));
    tmpFile->open();
    tmpFile->setPermissions(QFile::ReadOwner | QFile::WriteOwner);

    ts = new QDataStream( tmpFile );
    len = 0;
    appendedBytes = minCheck;

    c = QChar::Null;
    resetStatus();

    return detachedName;
}

void LongStream::append(QString str)
{
    if (ts) {
        ts->writeRawData(str.toAscii().constData(), str.length());

        len += str.length();
        appendedBytes += str.length();
        if (appendedBytes >= minCheck) {
            appendedBytes = 0;
            updateStatus();
        }
    }
}

int LongStream::length()
{
    return len;
}

QString LongStream::fileName()
{
    return tmpFile->fileName();
}

QString LongStream::readAll()
{
    QString result;

    if (ts) {
        while (!ts->atEnd()) {
            char buffer[1024];
            int len = ts->readRawData(buffer, 1024);
            if (len == -1) {
                break;
            } else {
                result.append(QString::fromAscii(buffer, len));
            }
        }
    }

    return result;
}

LongStream::Status LongStream::status()
{
    return mStatus;
}

void LongStream::resetStatus()
{
    mStatus = Ok;
}

void LongStream::updateStatus()
{
    if (!freeSpace())
        setStatus( LongStream::OutOfSpace );
}

void LongStream::setStatus( Status status )
{
    mStatus = status;
}

bool LongStream::freeSpace( const QString &path, int min)
{
    unsigned long long boundary = minFree;
    if (min >= 0)
        boundary = min;

    QString partitionPath = tempDir();
    if (!path.isEmpty())
        partitionPath = path;
    
#if defined(Q_OS_SYMBIAN)
    bool result(false);
   
    RFs fsSession;
    TInt rv;
    if ((rv = fsSession.Connect()) != KErrNone) {
        qWarning() << "Unable to connect to FS:" << rv;
    } else {
        TParse parse;
        TPtrC name(path.utf16(), path.length());

        if ((rv = fsSession.Parse(name, parse)) != KErrNone) {
            qWarning() << "Unable to parse:" << path << rv;
        } else {
            TInt drive;
            if ((rv = fsSession.CharToDrive(parse.Drive()[0], drive)) != KErrNone) {
                qWarning() << "Unable to convert:" << QString::fromUtf16(parse.Drive().Ptr(), parse.Drive().Length()) << rv;
            } else {
                TVolumeInfo info;
                if ((rv = fsSession.Volume(info, drive)) != KErrNone) {
                    qWarning() << "Unable to volume:" << drive << rv;
                } else {
                    result = (info.iFree > boundary);
                }
            }
        }
        
        fsSession.Close();
    }
    
    return result;
#elif !defined(Q_OS_WIN)
    struct statfs stats;

    while (statfs(partitionPath.toLocal8Bit(), &stats) == -1) {
        if (errno != EINTR) {
            qWarning() << "Could not stat filesystem";
            return true;
        }
    }
    unsigned long long bavail = ((unsigned long long)stats.f_bavail);
    unsigned long long bsize = ((unsigned long long)stats.f_bsize);

    return ((bavail * bsize) > boundary);
#else
    // MS recommend the use of GetDiskFreeSpaceEx, but this is not available on early versions
    // of windows 95.  GetDiskFreeSpace is unable to report free space larger than 2GB, but we're 
    // only concerned with much smaller amounts of free space, so this is not a hindrance.
    DWORD bytesPerSector(0);
    DWORD sectorsPerCluster(0);
    DWORD freeClusters(0);
    DWORD totalClusters(0);

    if (::GetDiskFreeSpace(reinterpret_cast<const wchar_t*>(partitionPath.utf16()), &bytesPerSector, &sectorsPerCluster, &freeClusters, &totalClusters) == FALSE) {
        qWarning() << "Unable to get free disk space:" << partitionPath;
    }

    return ((bytesPerSector * sectorsPerCluster * freeClusters) > boundary);
#endif
}

QString LongStream::errorMessage( const QString &prefix )
{
    QString str = QCoreApplication::tr( "Storage for messages is full. Some new "
                                    "messages could not be retrieved." );
    if (!prefix.isEmpty())
        return prefix + str;
    return str;
}

static QString tempDirPath()
{
    QString path = QMail::tempPath();
    QDir dir;
    if (!dir.exists( path ))
        dir.mkpath( path );
    return path;
}

QString LongStream::tempDir()
{
    static QString path(tempDirPath());
    return path;
}

void LongStream::cleanupTempFiles()
{
    QDir dir( LongStream::tempDir(), "longstream.*" );
    QStringList list = dir.entryList();
    for (int i = 0; i < list.size(); ++i) {
        QFile file( LongStream::tempDir() + list.at(i) );
        if (file.exists())
            file.remove();
    }
}
