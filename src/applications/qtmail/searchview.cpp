/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Messaging Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "searchview.h"
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include "messagelistview.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QFlags>
#include <QSpinBox>
#include <QDialog>
#include "emailfolderview.h"
#include "selectfolder.h"
#include <qmailaccount.h>
#include <qmailserviceaction.h>
#include <QStatusBar>
#include <QDateEdit>

static const int maxMessageBytes = 100000000;
static const int maxSearchTerms = 10;
static const int minSearchTerms = 1;

class SearchButton : public QPushButton
{
    Q_OBJECT
public:
    SearchButton(const QMailSearchAction* searchAction, QWidget* parent = 0);

signals:
    void startSearch();
    void stopSearch();

private slots:
    void thisClicked();
    void searchActivityChanged(QMailServiceAction::Activity a);

private:
    void updateView();

private:
    bool m_searching;
};

SearchButton::SearchButton(const QMailSearchAction* searchAction, QWidget* parent)
:
QPushButton("Search",parent),
m_searching(false)
{
    connect(this,SIGNAL(clicked(bool)),this,SLOT(thisClicked()));
    connect(searchAction,SIGNAL(activityChanged(QMailServiceAction::Activity)),
        this,SLOT(searchActivityChanged(QMailServiceAction::Activity)));
    updateView();
}

void SearchButton::thisClicked()
{
    if(m_searching)
        emit stopSearch();
    else
        emit startSearch();
}

void SearchButton::searchActivityChanged(QMailServiceAction::Activity a)
{
    m_searching = (a == QMailServiceAction::InProgress);
    updateView();
}

void SearchButton::updateView()
{
    if(m_searching)
    {
        setText("Stop");
        setIcon(QIcon(":icon/stop"));
    }
    else
    {
        setText("Search");
        setIcon(QIcon(":icon/find"));
    }
}

class BodySearchWidget : public QWidget
{
    Q_OBJECT

public:
    BodySearchWidget(QWidget* parent = 0);

    QString term() const;

public slots:
    void reset();

private:
    QCheckBox* m_checkBox;
    QLineEdit* m_term;
};

BodySearchWidget::BodySearchWidget(QWidget* parent)
:
QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    m_checkBox = new QCheckBox("Body contains:",this);
    layout->addWidget(m_checkBox);

    m_term = new QLineEdit(this);
    layout->addWidget(m_term);

    connect(m_checkBox,SIGNAL(clicked(bool)),m_term,SLOT(setEnabled(bool)));

    reset();
}

QString BodySearchWidget::term() const
{
    if(m_checkBox->isChecked())
        return m_term->text();
    else
        return QString();
}

void BodySearchWidget::reset()
{
    m_checkBox->setCheckState(Qt::Unchecked);
    m_term->clear();
    m_term->setEnabled(false);
}

class FolderSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    FolderSelectorWidget(QWidget* parent = 0);
    QMailMessageKey searchKey() const;

public slots:
    void reset();

private:
    void setupUi();

private slots:
    void selectFolder();
    void specificFolderRadioClicked();

private:
    void updateDisplay();
    bool haveSelection() const;

private:
    QRadioButton* m_localFolderRadio;
    QRadioButton* m_specificFolderRadio;
    QLineEdit* m_specificFolderDisplay;
    QToolButton* m_selectFolderButton;
    QCheckBox* m_includeSubFoldersCheckBox;

    EmailFolderModel m_model;
    QMailMessageSet* m_selectedItem;
};

FolderSelectorWidget::FolderSelectorWidget(QWidget* parent)
:
QWidget(parent),
m_model(this),
m_selectedItem(0)
{
    setupUi();
    m_model.init();
    reset();
}

QMailMessageKey FolderSelectorWidget::searchKey() const
{
    QMailMessageKey k;

    if (m_selectedItem) {
        k = m_selectedItem->messageKey();
        if (m_includeSubFoldersCheckBox->checkState() == Qt::Checked) {
            k |= m_selectedItem->descendantsMessageKey();
        }
    }

    return k;
}

void FolderSelectorWidget::setupUi()
{
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0,0,0,0);

    m_localFolderRadio = new QRadioButton("Search in all local folders:",this);
    connect(m_localFolderRadio,SIGNAL(clicked()),this,SLOT(reset()));
    vlayout->addWidget(m_localFolderRadio);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);

    m_specificFolderRadio = new QRadioButton("Search only in:",this);
    connect(m_specificFolderRadio,SIGNAL(clicked()),this,SLOT(specificFolderRadioClicked()));
    layout->addWidget(m_specificFolderRadio);

    m_specificFolderDisplay = new QLineEdit(this);
    m_specificFolderDisplay->setEnabled(false);
    layout->addWidget(m_specificFolderDisplay);

    m_selectFolderButton = new QToolButton(this);
    m_selectFolderButton->setIcon(QIcon(":icon/folder"));
    layout->addWidget(m_selectFolderButton);
    connect(m_selectFolderButton,SIGNAL(clicked()),this,SLOT(selectFolder()));

    m_includeSubFoldersCheckBox = new QCheckBox("Include sub folders",this);
    layout->addWidget(m_includeSubFoldersCheckBox);

    vlayout->addLayout(layout);
}

void FolderSelectorWidget::selectFolder()
{
    SelectFolderDialog sfd(&m_model, this);
    if (sfd.exec() == QDialog::Accepted) {
        m_selectedItem = sfd.selectedItem();

        m_specificFolderRadio->setChecked(true);
    } else if (!haveSelection()) {
        reset();
    }

    updateDisplay();
}

void FolderSelectorWidget::reset()
{
    m_selectedItem = 0;
    m_localFolderRadio->setChecked(true);
    m_includeSubFoldersCheckBox->setChecked(false);
    updateDisplay();
}

void FolderSelectorWidget::specificFolderRadioClicked()
{
    if (!haveSelection())
        selectFolder();
}

void FolderSelectorWidget::updateDisplay()
{
    if (m_selectedItem) {
        m_specificFolderDisplay->setText(m_selectedItem->displayName());
    } else {
        m_specificFolderDisplay->clear();
    }

    m_includeSubFoldersCheckBox->setEnabled(!m_specificFolderDisplay->text().isEmpty());
}

bool FolderSelectorWidget::haveSelection() const
{
    return m_selectedItem;
}

class SearchTermWidget : public QWidget
{
    Q_OBJECT

public:
    enum TermFormat { TextTerm, NumericTerm, MessageFlagsTerm , DateTerm, NoTerm};

    enum Comparator{ Contains =  1,
                     DoesNotContain = 1 << 1,
                     Equal = 1 << 2,
                     NotEqual = 1 << 3,
                     Is = 1 << 6,
                     IsNot = 1 << 7,
                     GreaterThan = 1 << 8,
                     GreaterThanEqual = 1 << 9,
                     LessThan = 1 << 10,
                     LessThanEqual = 1 << 11 };

    Q_DECLARE_FLAGS(Comparators,Comparator);

    static Comparators textComparators(){ return (QFlags<Comparator>(Contains) | DoesNotContain | Equal | NotEqual);};
    static Comparators numericComparators(){ return QFlags<Comparator>(Equal) | NotEqual | GreaterThan | GreaterThanEqual | LessThan | LessThanEqual;}
    static Comparators booleanComparators(){ return QFlags<Comparator>(Is) | IsNot;};

    static QMap<Comparator,QString> comparatorMap()
    {
        static QMap<Comparator,QString> s;
        if(s.isEmpty())
        {
            s.insert(Contains,"Contains");
            s.insert(DoesNotContain,"Does not contain");
            s.insert(Equal,"Equal");
            s.insert(NotEqual,"Not equal");
            s.insert(Is,"Is");
            s.insert(IsNot,"Is not");
            s.insert(GreaterThan,"Greater than");
            s.insert(GreaterThanEqual,"Greater than or equal");
            s.insert(LessThan,"Less than");
            s.insert(LessThanEqual,"Less than or equal");
        }
        return s;
    }

    enum Property{ AllRecipients,
                   SizeInBytes,
                   MessageStatus,
                   Subject,
                   From,
                   ReceptionDate };

    static QMap<Property,QString> propertyMap()
    {
        static QMap<Property,QString> s;
        if(s.isEmpty())
        {
            s.insert(AllRecipients,"All recipients");
            s.insert(SizeInBytes,"Size in bytes");
            s.insert(MessageStatus,"Message status");
            s.insert(Subject,"Subject");
            s.insert(From,"From");
            s.insert(ReceptionDate,"Reception date");
        }
        return s;
    }

public:
    SearchTermWidget(QWidget* parent = 0);
    void reset();
    QMailMessageKey searchKey() const;

private slots:
    void propertyChanged();

private:
    void setupUi();
    void setComparators(Comparators c);
    void setTerm(TermFormat f);
    inline Property property() const;
    inline Comparator comparator() const;
    QVariant term() const;

private:
    QComboBox* m_property;
    QComboBox* m_comparator;
    QLineEdit* m_textTerm;
    QComboBox* m_messageFlagsTerm;
    QSpinBox* m_numericTerm;
    QDateEdit* m_dateTerm;

private:
    friend class SearchTermsComposer;
};

class SearchKey : public QMailMessageKey
{
public:
    SearchKey(SearchTermWidget::Property p, SearchTermWidget::Comparator c, QVariant value);
};

SearchKey::SearchKey(SearchTermWidget::Property property, SearchTermWidget::Comparator comparator, QVariant value)
:
QMailMessageKey()
{
    QMailDataComparator::EqualityComparator ec;
    QMailDataComparator::InclusionComparator ic;
    QMailDataComparator::RelationComparator rc;

    enum compartorType{Equality,Inclusion,Relation,Presence,None} ct = None;

    switch(comparator)
    {
        case SearchTermWidget::Contains:
        {
            ct = Inclusion; ic = QMailDataComparator::Includes;
        } break;
        case SearchTermWidget::DoesNotContain:
        {
            ct = Inclusion; ic = QMailDataComparator::Excludes;
        } break;
        case SearchTermWidget::Equal:
        {
            ct = Equality; ec = QMailDataComparator::Equal;
        } break;
        case SearchTermWidget::NotEqual:
        {
            ct = Equality; ec = QMailDataComparator::NotEqual;
        } break;
        case SearchTermWidget::Is:
        {
            ct = Inclusion; ic = QMailDataComparator::Includes;
        } break;
        case SearchTermWidget::IsNot:
        {
            ct = Inclusion; ic = QMailDataComparator::Excludes;
        } break;
        case SearchTermWidget::GreaterThan:
        {
            ct = Relation; rc = QMailDataComparator::GreaterThan;
        } break;
        case SearchTermWidget::GreaterThanEqual:
        {
            ct = Relation; rc = QMailDataComparator::GreaterThanEqual;
        } break;
        case SearchTermWidget::LessThan:
        {
            ct = Relation; rc = QMailDataComparator::LessThan;
        } break;
        case SearchTermWidget::LessThanEqual:
        {
            ct = Relation; rc = QMailDataComparator::LessThanEqual;
        } break;
    }

    switch(property)
    {
        case SearchTermWidget::AllRecipients:
        {
            if(ct == Equality)
                QMailMessageKey::operator=(recipients(value.value<QString>(),ec));
            else
                QMailMessageKey::operator=(recipients(value.value<QString>(),ic));
        }break;
        case SearchTermWidget::SizeInBytes:
        {
            if(ct == Equality)
                QMailMessageKey::operator=(size(value.value<int>(),ec));
            else
                QMailMessageKey::operator=(size(value.value<int>(),rc));
        }break;
        case SearchTermWidget::MessageStatus:
        {
            QMailMessageKey::operator=(status(value.value<quint64>(),ic));
        }break;
        case SearchTermWidget::Subject:
        {
            if(ct == Equality)
                QMailMessageKey::operator=(subject(value.value<QString>(),ec));
            else
                QMailMessageKey::operator=(subject(value.value<QString>(),ic));
        }break;
        case SearchTermWidget::From:
        {
            if(ct == Equality)
                QMailMessageKey::operator=(sender(value.value<QString>(),ec));
            else
                QMailMessageKey::operator=(sender(value.value<QString>(),ic));
        }break;
        case SearchTermWidget::ReceptionDate:
        {
            //beacuse the storage system uses the more fine grained QDateTime, we need to construct keys that
            //consider time range as well

            if(ct == Equality)
            {
                QMailMessageKey startRange = receptionTimeStamp(QDateTime(value.value<QDate>()),QMailDataComparator::GreaterThanEqual);
                QMailMessageKey endRange = receptionTimeStamp(QDateTime(value.value<QDate>().addDays(1)),QMailDataComparator::LessThan);

                if(ec == QMailDataComparator::Equal)
                   QMailMessageKey::operator=(startRange & endRange);
                else
                    QMailMessageKey::operator=(~(startRange & endRange));
            }
            else
            {
                if(rc == QMailDataComparator::GreaterThan)
                    QMailMessageKey::operator=(receptionTimeStamp(QDateTime(value.value<QDate>().addDays(1)),QMailDataComparator::GreaterThanEqual));
                else if(rc == QMailDataComparator::LessThanEqual)
                    QMailMessageKey::operator=(receptionTimeStamp(QDateTime(value.value<QDate>()).addDays(1),QMailDataComparator::LessThan));
                else
                    QMailMessageKey::operator=(receptionTimeStamp(QDateTime(value.value<QDate>()),rc));
            }
        }break;
    }
}

SearchTermWidget::SearchTermWidget(QWidget* parent)
:
QWidget(parent)
{
    setupUi();
    m_property->setCurrentIndex(1);
    m_property->setCurrentIndex(0);
    reset();
}

void SearchTermWidget::reset()
{
    m_textTerm->clear();
    m_messageFlagsTerm->setCurrentIndex(0);
    m_numericTerm->setValue(m_numericTerm->minimum());
    m_property->setCurrentIndex(1);
    m_property->setCurrentIndex(0);
    m_dateTerm->setDate(QDate::currentDate());
}

QMailMessageKey SearchTermWidget::searchKey() const
{
    if(term().isValid())
        return SearchKey(property(),comparator(),term());
    else
        return QMailMessageKey();
}

void SearchTermWidget::propertyChanged()
{
    switch(property())
    {
        case SizeInBytes:
            setComparators(numericComparators());
            setTerm(NumericTerm);
        break;
        case MessageStatus:
            setComparators(booleanComparators());
            setTerm(MessageFlagsTerm);
        break;
        case ReceptionDate:
            setComparators(numericComparators());
            setTerm(DateTerm);
        break;
        default:
            setComparators(textComparators());
            setTerm(TextTerm);
        break;
    }
}

void SearchTermWidget::setupUi()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    m_property = new QComboBox(this);
    QMap<Property,QString>::const_iterator itr = propertyMap().constBegin();
    for(;itr != propertyMap().constEnd() ; itr++)
        m_property->addItem(*itr,itr.key());

    layout->addWidget(m_property);

    m_comparator = new QComboBox(this);
    layout->addWidget(m_comparator);

    m_textTerm = new QLineEdit(this);
    layout->addWidget(m_textTerm);

    m_messageFlagsTerm = new QComboBox(this);
    layout->addWidget(m_messageFlagsTerm);
    m_messageFlagsTerm->addItem(QIcon(":icon/new"),"New",QMailMessage::New);
    m_messageFlagsTerm->addItem(QIcon(":icon/mail_generic"),"Read",QMailMessage::Read);
    m_messageFlagsTerm->addItem(QIcon(":icon/mail_reply"),"Replied",QMailMessage::Replied);
    m_messageFlagsTerm->addItem(QIcon(":icon/mail_forward"),"Forwarded",QMailMessage::Forwarded);
    m_messageFlagsTerm->addItem(QIcon(":icon/sent"),"Sent",QMailMessage::Sent);
    m_messageFlagsTerm->addItem(QIcon(":icon/attach"),"Has attachment(s)",QMailMessage::HasAttachments);
    m_messageFlagsTerm->setVisible(false);

    m_numericTerm = new QSpinBox(this);
    m_numericTerm->setMaximum(maxMessageBytes);
    layout->addWidget(m_numericTerm);
    m_numericTerm->setVisible(false);

    m_dateTerm = new QDateEdit(this);
    layout->addWidget(m_dateTerm);
    m_dateTerm->setVisible(false);
    m_dateTerm->setCalendarPopup(true);

    connect(m_property,SIGNAL(currentIndexChanged(int)),this,SLOT(propertyChanged()));
}

void SearchTermWidget::setComparators(Comparators c)
{
    m_comparator->clear();
    QMap<Comparator,QString>::const_iterator itr = comparatorMap().constBegin();
    for(;itr != comparatorMap().constEnd();itr++)
    {
        if(c & itr.key())
            m_comparator->addItem(*itr,itr.key());
    }
}

void SearchTermWidget::setTerm(TermFormat t)
{
    m_textTerm->hide();
    m_numericTerm->hide();
    m_messageFlagsTerm->hide();
    m_dateTerm->hide();
    QWidget* w = 0;
    switch(t)
    {
        case TextTerm:
        w = m_textTerm;
        break;
        case NumericTerm:
        w = m_numericTerm;
        break;
        case MessageFlagsTerm:
        w = m_messageFlagsTerm;
        break;
        case DateTerm:
        w = m_dateTerm;
        break;
        case NoTerm:
        break;
    }
    if(w) w->show();
}

SearchTermWidget::Property SearchTermWidget::property() const
{
    Property p = static_cast<Property>(m_property->itemData(m_property->currentIndex()).toInt());
    return p;
}

SearchTermWidget::Comparator SearchTermWidget::comparator() const
{
    Comparator c = static_cast<Comparator>(m_comparator->itemData(m_comparator->currentIndex()).toInt());
    return c;
}

QVariant SearchTermWidget::term() const
{
    TermFormat f = NoTerm;
    if(m_textTerm->isVisible())
        f = TextTerm;
    else if(m_numericTerm->isVisible())
        f = NumericTerm;
    else if(m_messageFlagsTerm->isVisible())
        f = MessageFlagsTerm;
    else if(m_dateTerm->isVisible())
        f = DateTerm;

    QVariant keyValue;
    switch(f)
    {
        case TextTerm:
            if(!m_textTerm->text().isEmpty())
                keyValue = m_textTerm->text();
            break;
        case NumericTerm:
            keyValue = m_numericTerm->value();
            break;
        case DateTerm:
            keyValue = m_dateTerm->dateTime();
            break;
        case MessageFlagsTerm:
            keyValue = m_messageFlagsTerm->itemData(m_messageFlagsTerm->currentIndex());
            break;
        case NoTerm:
        break;
    }
    return keyValue;
}

class SearchTermsComposer : public QWidget
{
    Q_OBJECT

public:
    SearchTermsComposer(QWidget* parent = 0);
    QMailMessageKey searchKey() const;

public slots:
    void reset();

private slots:
    void moreButtonClicked();
    void lessButtonClicked();

private:
    void addSearchTerm();
    void removeSearchTerm();

private:
    QWidget* m_termsListWidget;
    QList<SearchTermWidget*> m_terms;
    QRadioButton* m_matchAllButton;
    QRadioButton* m_matchAnyButton;
    QPushButton* m_moreButton;
    QPushButton* m_lessButton;
    QVBoxLayout* m_termsLayout;
};

SearchTermsComposer::SearchTermsComposer(QWidget* parent)
:
QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    m_matchAllButton = new QRadioButton("Match all of the following",this);
    layout->addWidget(m_matchAllButton);
    m_matchAllButton->setChecked(true);

    m_matchAnyButton = new QRadioButton("Match any of the following",this);
    layout->addWidget(m_matchAnyButton);

    m_termsListWidget = new QWidget(this);
    m_termsLayout = new QVBoxLayout(m_termsListWidget);
    m_termsLayout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_termsListWidget);

    QHBoxLayout* controlButtonsLayout = new QHBoxLayout;

    m_moreButton = new QPushButton("More",this);
    m_moreButton->setIcon(QIcon(":icon/tick"));
    connect(m_moreButton,SIGNAL(clicked(bool)),this,SLOT(moreButtonClicked()));
    controlButtonsLayout->addWidget(m_moreButton);

    m_lessButton = new QPushButton("Less",this);
    m_lessButton->setIcon(QIcon(":icon/cross"));
    connect(m_lessButton,SIGNAL(clicked(bool)),this,SLOT(lessButtonClicked()));
    controlButtonsLayout->addWidget(m_lessButton);

    controlButtonsLayout->addStretch();

    layout->addLayout(controlButtonsLayout);
    for(int i = 0; i < minSearchTerms; ++i)
        addSearchTerm();
}

QMailMessageKey SearchTermsComposer::searchKey() const
{
    QList<SearchTermWidget*>::const_iterator itr = m_terms.begin();

    QMailMessageKey key = (*itr)->searchKey();
    itr++;

    if(m_matchAllButton->isChecked())
    {
        for(;itr != m_terms.end(); ++itr)
            key &= (*itr)->searchKey();
    }
    else if(m_matchAnyButton->isChecked())
    {
        for(;itr != m_terms.end(); ++itr)
            key |= (*itr)->searchKey();
    }
    return key;
}

void SearchTermsComposer::reset()
{
    setUpdatesEnabled(false);
    foreach(SearchTermWidget* stw, m_terms)
        stw->deleteLater();
    m_terms = QList<SearchTermWidget*>();
    for(int i = 0 ; i < minSearchTerms; ++i)
        addSearchTerm();
    updateGeometry();
    setUpdatesEnabled(true);
}

void SearchTermsComposer::moreButtonClicked()
{
    if(m_terms.count() < maxSearchTerms )
        addSearchTerm();
}

void SearchTermsComposer::lessButtonClicked()
{
    if(m_terms.count() > minSearchTerms )
        removeSearchTerm();

}

void SearchTermsComposer::addSearchTerm()
{
    setUpdatesEnabled(false);
    SearchTermWidget* stw = new SearchTermWidget(this);
    m_termsLayout->addWidget(stw);
    m_terms.append(stw);
    updateGeometry();
    setUpdatesEnabled(true);
    m_lessButton->setVisible(m_terms.count() > minSearchTerms);
    m_moreButton->setVisible(m_terms.count() != maxSearchTerms);
}

void SearchTermsComposer::removeSearchTerm()
{
    setUpdatesEnabled(false);
    SearchTermWidget* lastTerm = m_terms.last();
    m_terms.removeLast();
    m_termsLayout->removeWidget(lastTerm);
    lastTerm->deleteLater();
    updateGeometry();
    setUpdatesEnabled(true);
    m_lessButton->setVisible(m_terms.count() > minSearchTerms);
    m_moreButton->setVisible(m_terms.count() != maxSearchTerms);
}

SearchView::SearchView(QWidget* parent, Qt::WindowFlags f)
:
QMainWindow(parent,f),
m_searchAction(new QMailSearchAction())
{
    setupUi();
    setGeometry(0,0,600,400);
}

SearchView::~SearchView()
{
    delete m_searchAction; m_searchAction = 0;
}

void SearchView::setVisible(bool visible)
{
    if (visible) {
        //center the window on the parent
        QWidget* w = qobject_cast<QWidget*>(parent());
        QPoint p;
        if (w) {
            // Use mapToGlobal rather than geometry() in case w might
            // be embedded in another application
            QPoint pp = w->mapToGlobal(QPoint(0,0));
            p = QPoint(pp.x() + w->width()/2,
                    pp.y() + w->height()/ 2);
        }
        p = QPoint(p.x()-width()/2, p.y()-height()/2);
        move(p);
    }
    QMainWindow::setVisible(visible);
}

void SearchView::reset()
{
    m_folderSelectorWidget->reset();
    m_searchTermsComposer->reset();
    m_searchResults->reset();
    m_bodySearchWidget->reset();
}

void SearchView::close()
{
    if(m_searchAction->activity() == QMailServiceAction::InProgress)
        m_searchAction->cancelOperation();
    QMainWindow::close();
}

void SearchView::setupUi()
{
    setWindowTitle("Search");
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    QVBoxLayout* searchSettingsLayout = new QVBoxLayout;
    QVBoxLayout* controlButtonsLayout = new QVBoxLayout;

    m_searchButton = new SearchButton(m_searchAction,this);
    connect(m_searchButton,SIGNAL(startSearch()),this,SLOT(startSearch()));
    connect(m_searchButton,SIGNAL(stopSearch()),this,SLOT(stopSearch()));
    controlButtonsLayout->addWidget(m_searchButton);

    m_resetButton = new QPushButton("Reset",this);
    connect(m_resetButton,SIGNAL(clicked(bool)),this,SLOT(reset()));
    controlButtonsLayout->addWidget(m_resetButton);

    controlButtonsLayout->addStretch();

    m_closeButton = new QPushButton("Close",this);
    m_closeButton->setIcon(QIcon(":icon/close"));
    connect(m_closeButton,SIGNAL(clicked(bool)),this,SLOT(close()));
    controlButtonsLayout->addWidget(m_closeButton);

    mainLayout->addLayout(searchSettingsLayout);
    mainLayout->addLayout(controlButtonsLayout);

    m_folderSelectorWidget = new FolderSelectorWidget(this);
    searchSettingsLayout->addWidget(m_folderSelectorWidget);

    m_searchTermsComposer = new SearchTermsComposer(this);
    searchSettingsLayout->addWidget(m_searchTermsComposer);

    searchSettingsLayout->addWidget(m_bodySearchWidget = new BodySearchWidget(this));

    m_searchResults = new MessageListView(this);
    m_searchResults->showQuickSearch(false);
    connect(m_searchResults,SIGNAL(activated(const QMailMessageId&)),this,SIGNAL(searchResultSelected(const QMailMessageId&)));
    searchSettingsLayout->addWidget(m_searchResults);

    connect(m_searchAction,SIGNAL(messageIdsMatched(const QMailMessageIdList&)),this,SLOT(messageIdsMatched(const QMailMessageIdList&)));
    connect(m_searchAction,SIGNAL(activityChanged(QMailServiceAction::Activity)),this,SLOT(searchActivityChanged(QMailServiceAction::Activity)));
    connect(m_searchAction,SIGNAL(progressChanged(uint,uint)),this,SLOT(searchProgressChanged(uint,uint)));

    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);
    m_statusBar->showMessage("Ready.");
}

QMailMessageKey SearchView::searchKey() const
{
    QMailMessageKey key = m_searchTermsComposer->searchKey();
    if(!m_folderSelectorWidget->searchKey().isEmpty())
    {
        if(!key.isEmpty())
            key &= m_folderSelectorWidget->searchKey();
        else
            key = m_folderSelectorWidget->searchKey();
    }
    return key;
}

void SearchView::startSearch()
{
    QMailMessageKey key = searchKey();
    if(key.isEmpty() && m_bodySearchWidget->term().isEmpty())
        return;

    if(m_searchAction->activity() != QMailServiceAction::InProgress)
        m_searchAction->searchMessages(key,m_bodySearchWidget->term(),QMailSearchAction::Local);
}

void SearchView::stopSearch()
{
    m_searchAction->cancelOperation();
}

void SearchView::messageIdsMatched(const QMailMessageIdList& ids)
{
    m_searchResults->setKey(QMailMessageKey::id(ids));
}

void SearchView::searchActivityChanged(QMailServiceAction::Activity a)
{
    bool searching = (a == QMailServiceAction::InProgress);
    m_resetButton->setEnabled(!searching);
    m_searchResults->setEnabled(!searching);
    if(a == QMailServiceAction::Successful)
        m_statusBar->showMessage("Done.");
}

void SearchView::searchProgressChanged(uint value, uint total)
{
    if(total > 0)
        m_statusBar->showMessage(QString("Searching %1\%").arg(value/total*100));
}

#include <searchview.moc>
