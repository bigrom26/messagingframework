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

#ifndef SERVICEHANDLER_H
#define SERVICEHANDLER_H

#include <QByteArray>
#include <QFile>
#include <QLinkedList>
#include <QList>
#include <qmailmessageserver.h>
#include <qmailmessageservice.h>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QPointer>

class QMailServiceConfiguration;

class ServiceHandler : public QObject
{
    Q_OBJECT

public:
    ServiceHandler(QObject* parent);
    ~ServiceHandler();

public slots:
    void transmitMessages(quint64 action, const QMailAccountId &accountId);
    void retrieveFolderList(quint64, const QMailAccountId &accountId, const QMailFolderId &folderId, bool descending);
    void retrieveMessageList(quint64, const QMailAccountId &accountId, const QMailFolderId &folderId, uint minimum, const QMailMessageSortKey &sort);
    void retrieveMessages(quint64, const QMailMessageIdList &messageIds, QMailRetrievalAction::RetrievalSpecification spec);
    void retrieveMessagePart(quint64, const QMailMessagePart::Location &partLocation);
    void retrieveMessageRange(quint64, const QMailMessageId &messageId, uint minimum);
    void retrieveMessagePartRange(quint64, const QMailMessagePart::Location &partLocation, uint minimum);
    void retrieveAll(quint64, const QMailAccountId &accountId);
    void exportUpdates(quint64, const QMailAccountId &accountId);
    void synchronize(quint64, const QMailAccountId &accountId);
    void deleteMessages(quint64 action, const QMailMessageIdList& mailList, QMailStore::MessageRemovalOption);
    void copyMessages(quint64 action, const QMailMessageIdList& mailList, const QMailFolderId &destination);
    void moveMessages(quint64 action, const QMailMessageIdList& mailList, const QMailFolderId &destination);
    void flagMessages(quint64 action, const QMailMessageIdList& mailList, quint64 setMask, quint64 unsetMask);
    void createFolder(quint64 action, const QString &name, const QMailAccountId &accountId, const QMailFolderId &parentId);
    void renameFolder(quint64 action, const QMailFolderId &folderId, const QString &name);
    void deleteFolder(quint64 action, const QMailFolderId &folderId);
    void cancelTransfer(quint64 action);
    void searchMessages(quint64 action, const QMailMessageKey &filter, const QString &bodyText, QMailSearchAction::SearchSpecification spec, const QMailMessageSortKey &sort);
    void cancelSearch(quint64 action);
    void shutdown();
    void listActions();
    void protocolRequest(quint64 action, const QMailAccountId &accountId, const QString &request, const QVariant &data);

signals:
    void actionStarted(QMailActionData action);

    void statusChanged(quint64 action, const QMailServiceAction::Status status);
    void availabilityChanged(quint64 action, bool available);
    void connectivityChanged(quint64 action, QMailServiceAction::Connectivity connectivity);
    void activityChanged(quint64 action, QMailServiceAction::Activity activity);
    void progressChanged(quint64 action, uint progress, uint total);

    void retrievalCompleted(quint64 action);

    void messagesTransmitted(quint64 action, const QMailMessageIdList&);
    void messagesFailedTransmission(quint64 action, const QMailMessageIdList&, QMailServiceAction::Status::ErrorCode);
    void transmissionCompleted(quint64 action);

    void messagesDeleted(quint64 action, const QMailMessageIdList&);
    void messagesCopied(quint64 action, const QMailMessageIdList&);
    void messagesMoved(quint64 action, const QMailMessageIdList&);
    void messagesFlagged(quint64 action, const QMailMessageIdList&);

    void folderCreated(quint64 action, const QMailFolderId&);
    void folderRenamed(quint64 action, const QMailFolderId&);
    void folderDeleted(quint64 action, const QMailFolderId&); 

    void storageActionCompleted(quint64 action);

    void matchingMessageIds(quint64 action, const QMailMessageIdList&);
    void remoteSearchCompleted(quint64 action);
    void searchCompleted(quint64 action);

    void actionsListed(const QMailActionDataList &actions);

    void protocolResponse(quint64 action, const QString &response, const QVariant &data);
    void protocolRequestCompleted(quint64 action);

    void newMessagesAvailable();

private slots:
    void statusChanged(const QMailServiceAction::Status);
    void availabilityChanged(bool);
    void connectivityChanged(QMailServiceAction::Connectivity);
    void activityChanged(QMailServiceAction::Activity);
    void progressChanged(uint, uint );
    void actionCompleted(bool);

    void messagesTransmitted(const QMailMessageIdList&);
    void messagesFailedTransmission(const QMailMessageIdList&, QMailServiceAction::Status::ErrorCode);

    void messagesDeleted(const QMailMessageIdList&);
    void messagesCopied(const QMailMessageIdList&);
    void messagesMoved(const QMailMessageIdList&);
    void messagesFlagged(const QMailMessageIdList&);
    void messagesPrepared(const QMailMessageIdList&);
    void matchingMessageIds(const QMailMessageIdList&);

    void protocolResponse(const QString &response, const QVariant &data);

    void accountsAdded(const QMailAccountIdList &);
    void accountsUpdated(const QMailAccountIdList &);
    void accountsRemoved(const QMailAccountIdList &);

    void continueSearch();
    void finaliseSearch(quint64 action);

    void dispatchRequest();

    void expireAction();

    void reportFailures();

private:
    QMailAccountId transmissionAccountId(const QMailAccountId &accountId) const;

    void registerAccountServices(const QMailAccountIdList &ids);
    void deregisterAccountServices(const QMailAccountIdList &ids, QMailServiceAction::Status::ErrorCode code, const QString &text);
    void removeServiceFromActions(QMailMessageService *removeService);

    void reregisterAccountServices(const QMailAccountIdList &ids, QMailServiceAction::Status::ErrorCode code, const QString &text);

    void registerAccountSource(const QMailAccountId &accountId, QMailMessageSource *source, QMailMessageService *service);
    QMailMessageSource *accountSource(const QMailAccountId &accountId) const;

    void registerAccountSink(const QMailAccountId &accountId, QMailMessageSink *sink, QMailMessageService *service);
    QMailMessageSink *accountSink(const QMailAccountId &accountId) const;

    void registerAccountService(const QMailAccountId &accountId, const QMailServiceConfiguration &svcCfg);
    QMailMessageService *createService(const QString &service, const QMailAccountId &accountId);

    struct Request;
    bool servicesAvailable(const Request &services) const;
    bool serviceAvailable(QPointer<QMailMessageService> service) const;

    QSet<QMailMessageService*> sourceServiceSet(const QMailAccountId &id) const;
    QSet<QMailMessageService*> sourceServiceSet(const QSet<QMailAccountId> &ids) const;

    QSet<QMailMessageService*> sinkServiceSet(const QMailAccountId &id) const;
    QSet<QMailMessageService*> sinkServiceSet(const QSet<QMailAccountId> &ids) const;

    quint64 sourceAction(QMailMessageSource *source) const;
    quint64 sinkAction(QMailMessageSink *sink) const;

    quint64 serviceAction(QMailMessageService *service) const;

    typedef bool (ServiceHandler::*RequestServicer)(quint64, const QByteArray &);
    typedef void (ServiceHandler::*CompletionSignal)(quint64);

    void enqueueRequest(quint64 action, const QByteArray &data, const QSet<QMailMessageService*> &services, RequestServicer servicer, CompletionSignal completion, QMailServerRequestType description, const QSet<QMailMessageService*> &preconditions = QSet<QMailMessageService*>());

    bool dispatchPrepareMessages(quint64 action, const QByteArray& data);
    bool dispatchTransmitMessages(quint64 action, const QByteArray& data);
    bool dispatchRetrieveFolderListAccount(quint64, const QByteArray &data);
    bool dispatchRetrieveFolderList(quint64, const QByteArray &data);
    bool dispatchRetrieveMessageList(quint64, const QByteArray &data);
    bool dispatchRetrieveMessages(quint64, const QByteArray &data);
    bool dispatchRetrieveMessagePart(quint64, const QByteArray &data);
    bool dispatchRetrieveMessageRange(quint64, const QByteArray &data);
    bool dispatchRetrieveMessagePartRange(quint64, const QByteArray &data);
    bool dispatchRetrieveAll(quint64, const QByteArray &data);
    bool dispatchExportUpdates(quint64, const QByteArray &data);
    bool dispatchSynchronize(quint64, const QByteArray &data);
    bool dispatchDiscardMessages(quint64 action, const QByteArray &data);
    bool dispatchDeleteMessages(quint64 action, const QByteArray &data);
    bool dispatchCopyMessages(quint64 action, const QByteArray &data);
    bool dispatchCopyToLocal(quint64 action, const QByteArray &data);
    bool dispatchMoveMessages(quint64 action, const QByteArray &data);
    bool dispatchFlagMessages(quint64 action, const QByteArray &data);
    bool dispatchCreateFolder(quint64 action, const QByteArray &data);
    bool dispatchDeleteFolder(quint64 action, const QByteArray &data);
    bool dispatchRenameFolder(quint64 action, const QByteArray &data);
    bool dispatchSearchMessages(quint64 action, const QByteArray &data);
    bool dispatchCancelSearch(quint64 action, const QByteArray &data);
    bool dispatchProtocolRequest(quint64 action, const QByteArray &data);

    void reportFailure(quint64, QMailServiceAction::Status::ErrorCode, const QString& = QString(), const QMailAccountId& = QMailAccountId(), const QMailFolderId& = QMailFolderId(), const QMailMessageId& = QMailMessageId());
    void reportFailure(quint64, const QMailServiceAction::Status);

    void updateAction(quint64);

    void setRetrievalInProgress(const QMailAccountId &id, bool inProgress);
    void setTransmissionInProgress(const QMailAccountId &id, bool inProgress);

    QMap<QPair<QMailAccountId, QString>, QPointer<QMailMessageService> > serviceMap;
    QMap<QMailAccountId, QMailMessageSource*> sourceMap;
    QMap<QMailAccountId, QMailMessageSink*> sinkMap;
    QMap<QMailMessageSource*, QPointer<QMailMessageService> > sourceService;
    QMap<QMailMessageSink*, QPointer<QMailMessageService> > sinkService;
    QMap<QMailAccountId, QMailAccountId> masterAccount;

    QSet<QMailMessageService*> mUnavailableServices;

    class ActionData
    {
    public:
        QMailServerRequestType description;
        QSet<QPointer<QMailMessageService> > services;
        CompletionSignal completion;
        QTime expiry;
        bool reported;
    };
    
    QMap<quint64, ActionData> mActiveActions;
    QLinkedList<quint64> mActionExpiry;

    QMap<QPointer<QMailMessageService>, quint64> mServiceAction;

    enum { ExpiryPeriod = 120 * 1000 };

    struct Request 
    {
        quint64 action;
        QByteArray data;
        QSet<QPointer<QMailMessageService> > services;
        QSet<QPointer<QMailMessageService> > preconditions;
        RequestServicer servicer;
        CompletionSignal completion;
        QMailServerRequestType description;
    };

    QList<Request> mRequests;

    class MessageSearch
    {
    public:
        MessageSearch(quint64 action, const QMailMessageIdList &ids, const QString &text);

        quint64 action() const;

        const QString &bodyText() const;

        bool pending() const;
        void inProgress();

        bool isEmpty() const;

        uint total() const;
        uint progress() const;

        QMailMessageIdList takeBatch();

    private:
        quint64 _action;
        QMailMessageIdList _ids;
        QString _text;
        bool _active;
        uint _total;
        uint _progress;
    };

    QList<MessageSearch> mSearches;
    QMailMessageIdList mMatchingIds;
    QMailMessageIdList mSentIds;

    QFile _requestsFile;
    QSet<quint64> _outstandingRequests;
    QList<quint64> _failedRequests;

    QSet<QMailAccountId> _retrievalAccountIds;
    QSet<QMailAccountId> _transmissionAccountIds;
};

#endif
