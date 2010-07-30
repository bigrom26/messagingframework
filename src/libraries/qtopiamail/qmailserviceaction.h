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

#ifndef QMAILSERVICEACTION_H
#define QMAILSERVICEACTION_H

#include "qprivateimplementation.h"
#include "qmailglobal.h"
#include "qmailid.h"
#include "qmailmessagekey.h"
#include "qmailmessagesortkey.h"
#include "qmailmessage.h"
#include "qmailaction.h"
#include <QString>
#include <QStringList>

class QMailServiceActionPrivate;

class QTOPIAMAIL_EXPORT QMailServiceAction
    : public QObject,
      public QPrivatelyNoncopyable<QMailServiceActionPrivate>
{
    Q_OBJECT

    friend class QMailServiceActionPrivate;
    friend class QMailRetrievalActionPrivate;
    friend class QMailTransmitActionPrivate;
    friend class QMailSearchActionPrivate;

public:
    typedef QMailServiceActionPrivate ImplementationType;

    enum Connectivity {
        Offline = 0,
        Connecting,
        Connected,
        Disconnected
    };

    enum Activity {
        Pending = 0,
        InProgress,
        Successful,
        Failed
    };

    class QTOPIAMAIL_EXPORT Status 
    {
    public: 
        enum ErrorCode {
            ErrNoError = 0,
            ErrorCodeMinimum = 1024,
            ErrNotImplemented = ErrorCodeMinimum,
            ErrFrameworkFault,
            ErrSystemError,
            ErrUnknownResponse,
            ErrLoginFailed,
            ErrCancel,
            ErrFileSystemFull,
            ErrNonexistentMessage,
            ErrEnqueueFailed,
            ErrNoConnection,
            ErrConnectionInUse,
            ErrConnectionNotReady,
            ErrConfiguration,
            ErrInvalidAddress,
            ErrInvalidData,
            ErrTimeout,
            ErrInternalStateReset,
            ErrorCodeMaximum = ErrInternalStateReset
        };

        Status();
        Status(ErrorCode code, 
               const QString &text,
               const QMailAccountId &accountId, 
               const QMailFolderId &folderId,
               const QMailMessageId &messageId);

        Status(const Status &other);

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);

        ErrorCode errorCode;
        QString text;

        QMailAccountId accountId;
        QMailFolderId folderId;
        QMailMessageId messageId;
    };

    ~QMailServiceAction();

    Connectivity connectivity() const;
    Activity activity() const;
    const Status status() const;
    QPair<uint, uint> progress() const;

public slots:
    virtual void cancelOperation();

signals:
    void connectivityChanged(QMailServiceAction::Connectivity c);
    void activityChanged(QMailServiceAction::Activity a);
    void statusChanged(const QMailServiceAction::Status &s);
    void progressChanged(uint value, uint total);

protected:
    // Only allow creation by sub-types
    template<typename Subclass>
    QMailServiceAction(Subclass *p, QObject *parent);

protected:
    void setStatus(Status::ErrorCode code, const QString &text = QString());
    void setStatus(Status::ErrorCode code, const QString &text, const QMailAccountId &accountId,const QMailFolderId &folderId = QMailFolderId(), const QMailMessageId &messageId = QMailMessageId());
};

class QMailRetrievalActionPrivate;

class QTOPIAMAIL_EXPORT QMailRetrievalAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailRetrievalActionPrivate ImplementationType;

    enum RetrievalSpecification {
        Flags,
        MetaData,
        Content
    };

    QMailRetrievalAction(QObject *parent = 0);
    ~QMailRetrievalAction();

public slots:
    void retrieveFolderList(const QMailAccountId &accountId, const QMailFolderId &folderId, bool descending = true);
    void retrieveMessageList(const QMailAccountId &accountId, const QMailFolderId &folderId, uint minimum = 0, const QMailMessageSortKey &sort = QMailMessageSortKey());

    void retrieveMessages(const QMailMessageIdList &messageIds, RetrievalSpecification spec = MetaData);
    void retrieveMessagePart(const QMailMessagePart::Location &partLocation);

    void retrieveMessageRange(const QMailMessageId &messageId, uint minimum);
    void retrieveMessagePartRange(const QMailMessagePart::Location &partLocation, uint minimum);

    void retrieveAll(const QMailAccountId &accountId);
    void exportUpdates(const QMailAccountId &accountId);

    void synchronizeAll(const QMailAccountId &accountId);
};


class QMailTransmitActionPrivate;

class QTOPIAMAIL_EXPORT QMailTransmitAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailTransmitActionPrivate ImplementationType;

    QMailTransmitAction(QObject *parent = 0);
    ~QMailTransmitAction();

signals:
    void messagesTransmitted(const QMailMessageIdList &ids);
    void messagesFailedTransmission(const QMailMessageIdList &ids, QMailServiceAction::Status::ErrorCode);

public slots:
    void transmitMessages(const QMailAccountId &accountId);

};


class QMailStorageActionPrivate;

class QTOPIAMAIL_EXPORT QMailStorageAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailStorageActionPrivate ImplementationType;

    QMailStorageAction(QObject *parent = 0);
    ~QMailStorageAction();

public slots:
    void deleteMessages(const QMailMessageIdList &ids);
    void discardMessages(const QMailMessageIdList &ids);

    void copyMessages(const QMailMessageIdList &ids, const QMailFolderId &destinationId);
    void moveMessages(const QMailMessageIdList &ids, const QMailFolderId &destinationId);

    void flagMessages(const QMailMessageIdList &ids, quint64 setMask, quint64 unsetMask);

    void createFolder(const QString &name, const QMailAccountId &accountId, const QMailFolderId &parentId);
    void renameFolder(const QMailFolderId &folderId, const QString &name);
    void deleteFolder(const QMailFolderId &folderId);
};


class QMailSearchActionPrivate;

class QTOPIAMAIL_EXPORT QMailSearchAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailSearchActionPrivate ImplementationType;

    enum SearchSpecification {
        Local,
        Remote
    };

    QMailSearchAction(QObject *parent = 0);
    ~QMailSearchAction();

    QMailMessageIdList matchingMessageIds() const;

signals:
    void messageIdsMatched(const QMailMessageIdList &ids);

public slots:
    void searchMessages(const QMailMessageKey &filter, const QString& bodyText, SearchSpecification spec, const QMailMessageSortKey &sort = QMailMessageSortKey());
    void cancelOperation();
};

class QMailActionInfoPrivate;

class QMailActionInfo : public QMailServiceAction {
    Q_OBJECT
//    If needed properties can be enabled by uncommenting this code
//    and won't break api/abi

//    Q_PROPERTY(QMailServerRequestType requestType READ requestType)
//    Q_ENUMS(QMailServerRequestType)
//    Q_PROPERTY(QMailServiceAction::Activity activity READ activity NOTIFY activityChanged)
//    Q_PROPERTY(QMailActionId id READ id)
//    Q_PROPERTY(float totalProgress READ totalProgress NOTIFY totalprogressChanged)
public:
    typedef Status::ErrorCode StatusErrorCode; // moc hack
// private:
//    Q_PROPERTY(StatusErrorCode statusErrorCode READ statusErrorCode NOTIFY statusErrorCodeChanged)
//    Q_ENUMS(StatusErrorCode)

//    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
//    Q_PROPERTY(QMailAccountId statusAccountId READ statusAccountId NOTIFY statusAccountIdChanged)
//    Q_PROPERTY(QMailFolderId statusFolderId READ statusFolderId NOTIFY statusFolderActionChanged)
//    Q_PROPERTY(QMailMessageId statusMessageId READ statusMessageId NOTIFY statusMessageIdChanged)
// public:
    typedef QMailActionInfoPrivate ImplementationType;

    QMailActionId id() const;
    QMailServerRequestType requestType() const;
    float totalProgress() const;
    StatusErrorCode statusErrorCode() const;
    QString statusText() const;
    QMailAccountId statusAccountId() const;
    QMailFolderId statusFolderId() const;
    QMailMessageId statusMessageId() const;

signals:
    void statusErrorCodeChanged(QMailActionInfo::StatusErrorCode newError);
    void statusTextChanged(const QString &newText);
    void statusAccountIdChanged(const QMailAccountId &newAccountId);
    void statusFolderIdChanged(const QMailFolderId &newFolderId);
    void statusMessageIdChanged(const QMailMessageId &newMessageId);
    void totalProgressChanged(float progress);
protected:
    friend class QMailActionObserverPrivate;
    QMailActionInfo(quint64 action, QMailServerRequestType description);
};

class QMailActionObserverPrivate;

class QTOPIAMAIL_EXPORT QMailActionObserver : public QMailServiceAction
{
    Q_OBJECT
//  Q_PROPERTY(QList< QSharedPointer<QMailActionInfo> > actions READ actions NOTIFY actionsChanged)
public:   
    typedef QMailActionObserverPrivate ImplementationType;

    QMailActionObserver(QObject *parent = 0);
    virtual ~QMailActionObserver();

    QList< QSharedPointer<QMailActionInfo> > actions() const;
signals:
    void actionsChanged(const QList< QSharedPointer<QMailActionInfo> > &newActions);
};

class QMailProtocolActionPrivate;

class QTOPIAMAIL_EXPORT QMailProtocolAction : public QMailServiceAction
{
    Q_OBJECT

public:
    typedef QMailProtocolActionPrivate ImplementationType;

    QMailProtocolAction(QObject *parent = 0);
    ~QMailProtocolAction();

signals:
    void protocolResponse(const QString &response, const QVariant &data);

public slots:
    void protocolRequest(const QMailAccountId &accountId, const QString &request, const QVariant &data);
};


Q_DECLARE_USER_METATYPE(QMailServiceAction::Status)

Q_DECLARE_USER_METATYPE_ENUM(QMailServiceAction::Connectivity)
Q_DECLARE_USER_METATYPE_ENUM(QMailServiceAction::Activity)
Q_DECLARE_USER_METATYPE_ENUM(QMailServiceAction::Status::ErrorCode)

Q_DECLARE_USER_METATYPE_ENUM(QMailRetrievalAction::RetrievalSpecification)

Q_DECLARE_USER_METATYPE_ENUM(QMailSearchAction::SearchSpecification)

#endif
