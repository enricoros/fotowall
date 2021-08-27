/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Original work                                                         *
 *      file             : likebackdialog.cpp                              *
 *      license          : GPL v2+                                         *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
                              likebackdialog.cpp
                             -------------------
    begin                : unknown
    imported to LB svn   : 3 june, 2009
    copyright            : (C) 2006 by Sebastien Laout
                           (C) 2008-2009 by Valerio Pilo, Sjors Gielen
    email                : sjors@kmess.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "LikeBackDialog.h"

#include <QApplication>
#include <QButtonGroup>
#include <QLocale>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPushButton>
#include <QSettings>
#include <QUrl>


// Constructor
LikeBackDialog::LikeBackDialog( QNetworkAccessManager * nam, LikeBack::Button reason,
                                const QString &initialComment, const QString &windowPath,
                                const QString &context, LikeBack *likeBack )
  : QDialog( QApplication::activeWindow() )
  , m_nam( nam )
  , m_context( context )
  , m_likeBack( likeBack )
  , m_windowPath( windowPath )
{
    // Customize UI
    setupUi( this );
    QFont font = informationLabel->font();
    font.setPointSize(font.pointSize() - 1);
    informationLabel->setFont(font);
    emailLabel->setFont(font);
    setAttribute(Qt::WA_DeleteOnClose);
#if 1
    restoreGeometry(QSettings().value("LikeBack/geometry").toByteArray());
#else
    restoreDialogSize( KGlobal::config()->group( "LikeBackDialog" ) );
#endif

    // Connect the 2 bottom buttons
    connect( buttonBox, SIGNAL( accepted() ), this, SLOT( slotSendData() ) );
    connect( buttonBox, SIGNAL( rejected() ), this, SLOT( close() ) );

    // Group the buttons together to retrieve the checked one quickly
    m_typeGroup = new QButtonGroup( this );
    m_typeGroup->addButton( likeRadio_,     LikeBack::Like    );
    m_typeGroup->addButton( dislikeRadio_,  LikeBack::Dislike );
    m_typeGroup->addButton( bugRadio_,      LikeBack::Bug     );
    m_typeGroup->addButton( featureRadio_,  LikeBack::Feature );

    // Hide unneeded buttons
    LikeBack::Button buttons = m_likeBack->buttons();
    likeRadio_   ->setVisible( buttons & LikeBack::Like    );
    dislikeRadio_->setVisible( buttons & LikeBack::Dislike );
    bugRadio_    ->setVisible( buttons & LikeBack::Bug     );
    featureRadio_->setVisible( buttons & LikeBack::Feature );

    // If no specific "reason" is provided, choose the first one:
    if ( reason == LikeBack::AllButtons || reason == LikeBack::DefaultButtons ) {
         if( buttons & LikeBack::Dislike ) reason = LikeBack::Dislike;
    else if( buttons & LikeBack::Bug     ) reason = LikeBack::Bug;
    else if( buttons & LikeBack::Feature ) reason = LikeBack::Feature;
    else                                   reason = LikeBack::Like;
    }

    // Choose which button to check
    switch( reason ) {
        case LikeBack::Like:    likeRadio_   ->setChecked( true ); break;
        case LikeBack::Dislike: dislikeRadio_->setChecked( true ); break;
        case LikeBack::Bug:     bugRadio_    ->setChecked( true ); break;
        case LikeBack::Feature: featureRadio_->setChecked( true ); break;
        default: break; // Will never arrive here
    }

    // Disable the Ok button if no comment is present
    connect( m_comment, SIGNAL( textChanged() ), this, SLOT( verifyOk() ) );

    // If no window path is provided, get the current active window path
    if ( m_windowPath.isEmpty() )
        m_windowPath = LikeBack::activeWindowPath();

    // Specify the initial comment
    m_comment->setPlainText( initialComment );
    m_comment->setFocus();

    // Provide the initial status for email address widgets if available
    emailAddressEdit_->setText( m_likeBack->emailAddress() );
    specifyEmailCheck->setChecked( true );

    // The introduction message is long and will require a new minimum dialog size
    informationLabel->setText( introductionText() );
    setMinimumSize( minimumSizeHint() );

    // Initially verify the widgets status
    verifyOk();
}



// Destructor
LikeBackDialog::~LikeBackDialog()
{
#if 1
    QSettings().setValue("LikeBack/geometry", saveGeometry());
#else
    KConfigGroup group = KGlobal::config()->group( "LikeBackDialog" );
    saveDialogSize( group );
#endif
}



// Construct the introductory text of the dialog
QString LikeBackDialog::introductionText()
{
    QStringList acceptedLocaleCodes = m_likeBack->acceptedLocales();
    QStringList acceptedLocales;

    // Define a list of languages which the application developers are able to understand
    if ( ! acceptedLocaleCodes.isEmpty() ) {
        foreach( const QString & locale, acceptedLocaleCodes )
            acceptedLocales << QLocale::languageToString(QLocale(locale).language());
    } else if( ! QLocale::system().name().startsWith( "en" ) )
        acceptedLocales << QLocale::languageToString(QLocale::English);

    // Put the locales list together in a readable string
    QString languagesMessage;
    if ( ! acceptedLocales.isEmpty() ) {
        // TODO: Replace the URL with a localized one:
        //QString translationTool( "http://www.google.com/language_tools?hl=" + QLocale::system().name() );
        QString translationTool = "https://translate.google.com";

        if ( acceptedLocales.count() == 1 )
            languagesMessage = tr( "Please, write it in <b>%1</b> (you may want to use an <a href=\"%3\">online translation tool</a> for this).<br/>",
                                   "Feedback dialog text, message with one accepted language for the comments" )
                                   .arg( acceptedLocales.first() )
                                   .arg( translationTool );
        else
            languagesMessage = tr( "Please, write it in <b>%1 or %2</b> (you may want to use an <a href=\"%3\">online translation tool</a> for this).<br/>",
                                   "Feedback dialog text, message with list of accepted languages for the comments" )
                                   .arg( QStringList( acceptedLocales.mid( 0, acceptedLocales.count() - 1 ) ).join( ", " ) )
                                   .arg( acceptedLocales.last() )
                                   .arg( translationTool );
    }

    // If both "I Like" and "I Dislike" buttons are shown and one is clicked:
    QString balancingMessage;
    if( m_likeBack->isLikeActive() && m_likeBack->isDislikeActive()
        && ( m_typeGroup->checkedId() == LikeBack::Like || m_typeGroup->checkedId() == LikeBack::Dislike ) )
        balancingMessage = tr( "To make the comments you send more useful in improving this application, "
                               "try to send the same amount of positive and negative comments.<br/>",
                               "Feedback dialog text, message to remind to balance the likes and dislikes" );

    // If feature requests are not allowed:
    QString noFeatureRequestsMessage;
    if ( ! m_likeBack->isFeatureActive() )
        noFeatureRequestsMessage = tr( "Please, do not ask for new features: this kind of request will be ignored.<br/>",
                                       "Feedback dialog text, text to disallow feature requests" );

    // Blend all previous messages together
    return tr( "<p>You can provide the developers a brief description of your opinions about %1.<br/>"
               "%2 " // %2: Contains the newline if present
               "%3%4</p>",
               "Feedback dialog text, %1=Application name,%2=message with list of accepted languages for the comment,"
               "%3=optional text to remind to balance the likes and dislikes,%4=optional text to disallow feature requests." )
               .arg( QCoreApplication::applicationName() )
               .arg( languagesMessage )
               .arg( balancingMessage )
               .arg( noFeatureRequestsMessage );
}



// Check if the UI should allow the user to send the comment
void LikeBackDialog::verifyOk()
{
    bool hasComment = ( ! m_comment->document()->isEmpty() );
    bool hasType    = ( m_typeGroup->checkedId() != -1 );

    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( hasComment && hasType );
}



// Send the comment to the developers site
void LikeBackDialog::slotSendData()
{
    // Only send the email if the user wants it to be sent
    QString emailAddress;
    if( specifyEmailCheck->isChecked() ) {
        emailAddress = emailAddressEdit_->text();

        // lame-ass way to check if the e-mail address is valid:
        if( !emailAddress.contains( QRegExp( "^[A-Z0-9._%\\-]+@(?:[A-Z0-9\\-]+\\.)+[A-Z]{2,4}$", Qt::CaseInsensitive ) ) ) {
            QMessageBox::warning( this, tr("Wrong Email"), tr( "The email address you have entered is not valid, and cannot be used: '%1'" ).arg( emailAddress ) );
            return;
        }

        m_likeBack->setEmailAddress( emailAddress, true );
    }

    // Disable the UI while we're sending the request
    m_comment->setEnabled( false );
    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

    // Choose the type of feedback
    QString type;
    switch( m_typeGroup->checkedId() ) {
        case LikeBack::Like:    type = "Like";    break;
        case LikeBack::Dislike: type = "Dislike"; break;
        case LikeBack::Bug:     type = "Bug";     break;
        case LikeBack::Feature: type = "Feature"; break;
    }

    // Compile the feedback data
    QString data( "protocol=" + QUrl::toPercentEncoding( "1.0" )                                    + '&' +
                  "type="     + QUrl::toPercentEncoding( type )                                     + '&' +
                  "version="  + QUrl::toPercentEncoding( QCoreApplication::applicationVersion() )   + '&' +
                  "locale="   + QUrl::toPercentEncoding( QLocale::system().name() )                 + '&' +
                  "window="   + QUrl::toPercentEncoding( m_windowPath )                             + '&' +
                  "context="  + QUrl::toPercentEncoding( m_context )                                + '&' +
                  "comment="  + QUrl::toPercentEncoding( m_comment->toPlainText() )                 + '&' +
                  "email="    + QUrl::toPercentEncoding( emailAddress ) );


    // make up the URL
    QUrl remoteUrl;
    remoteUrl.setScheme(m_likeBack->hostScheme());
    remoteUrl.setHost(m_likeBack->hostName());
    remoteUrl.setPath(m_likeBack->remotePath());
    if (m_likeBack->hostPort())
        remoteUrl.setPort(m_likeBack->hostPort());

#if 0
    qDebug() << remoteUrl;
    qDebug() << data;
#endif

    // do the POST and listen for the reply
    QNetworkRequest request(remoteUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply * reply = m_nam->post(request, data.toUtf8());
    connect(reply, SIGNAL(finished()), this, SLOT(slotRequestFinished()));
}



// Display confirmation of the sending action
void LikeBackDialog::slotRequestFinished()
{
    QNetworkReply * reply = static_cast<QNetworkReply *>(sender());
    bool error = reply->error() != QNetworkReply::NoError;
    reply->deleteLater();

    m_likeBack->disableBar();

    if ( ! error ) {
        QMessageBox::information( this,
                                  tr( "Comment Sent",
                                      "Dialog box title" ),
                                  tr( "<p>Your comment has been sent successfully. "
                                      "It will help us improve the application!</p>"
                                      "<p>Thanks for your time.</p>",
                                      "Dialog box text" ) );
        hide();
        m_likeBack->enableBar();
        accept();
        return;
    }

    // TODO: Save to file if error (connection not present at the moment)
    QMessageBox::warning( this,
                          tr( "Comment Sending Error",
                              "Dialog box title" ),
                          tr( "<p>There has been an error while trying to send the comment.</p>"
                              "<p>Please, try again later.</p>",
                              "Dialog box text" ) );

    m_likeBack->enableBar();

    // Re-enable the UI
    m_comment->setEnabled( true );
    verifyOk();
}
