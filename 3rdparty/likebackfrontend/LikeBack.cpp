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
 *      file             : likeback.cpp                                    *
 *      license          : GPL v2+                                         *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
                                likeback.cpp
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

#include "LikeBack.h"
#include "LikeBackDialog.h"
#include "LikeBack_p.h"

#include <QApplication>
#include <QSettings>


// Constructor
LikeBack::LikeBack( Button buttons, bool showBarByDefault, QObject * parent )
  : QObject( parent )
  , d( new LikeBackPrivate )
{
  // Initialize properties
  d->buttons          = buttons;
  d->showBarByDefault = showBarByDefault;
  d->showBar = userWantsToShowBar();

  /// TODO Initialize the button-bars

  // Show the information message if it is the first time, and if the button-bar is shown:
  showInformationMessage();
}



// Destructor
LikeBack::~LikeBack()
{
  delete d;
}



// Set the windows listing flag
void LikeBack::setWindowNamesListing(WindowListing windowListing)
{
  d->windowListing = windowListing;
}



// Return the windows listing flag
LikeBack::WindowListing LikeBack::windowNamesListing()
{
  return d->windowListing;
}



// Set which languages are accepted by the developers for the comments
void LikeBack::setAcceptedLanguages( const QStringList &locales )
{
  d->acceptedLocales          = locales;
}



// Return the accepted languages for the comments
QStringList LikeBack::acceptedLocales()
{
  return d->acceptedLocales;
}



// Set the site address where to send feedback
void LikeBack::setServer(const QString &hostName, const QString &remotePath, quint16 hostPort)
{
  d->hostName   = hostName;
  d->remotePath = remotePath;
  d->hostPort   = hostPort;
}



// Get the developers site hostname
QString LikeBack::hostName()
{
  return d->hostName;
}



// Get the path on the developers site
QString LikeBack::remotePath()
{
  return d->remotePath;
}



// Get the developers site port
quint16 LikeBack::hostPort()
{
  return d->hostPort;
}



// Disable the LikeBack Bar
void LikeBack::disableBar()
{
  d->disabledCount++;
#if 0
  d->bar->setBarVisible( d->bar && d->disabledCount > 0 );
#endif
}



// Enable the LikeBack Bar
void LikeBack::enableBar()
{
  d->disabledCount--;

#ifdef DEBUG_LIKEBACK
  if( d->disabledCount < 0 )
    qWarning() << "Enabled more times than it was disabled. Please refer to the disableBar() documentation for more information and hints.";
#endif

#if 0
  d->bar->setBarVisible( d->bar && d->disabledCount <= 0 );
#endif
}



// Get whether the bar is enabled or not
bool LikeBack::enabledBar()
{
  return d->disabledCount <= 0;
}



// Display the Send Comments dialog
void LikeBack::execCommentDialog( Button type, const QString &initialComment, const QString &windowPath, const QString &context )
{
  LikeBackDialog *dialog = new LikeBackDialog( d->nam, type, initialComment, windowPath, context, this );

  if ( userWantsToShowBar() ) {
    disableBar();
    connect( dialog, SIGNAL( destroyed(QObject*) ), this, SLOT( enableBar() ) );
  }

  dialog->show();
}



// Display the Send Comments dialog
void LikeBack::execCommentDialogFromHelp()
{
  execCommentDialog( AllButtons, /*initialComment=*/"", /*windowPath=*/"HelpMenuAction" );
}



// Retrieve which feedback buttons are active
LikeBack::Button LikeBack::buttons()
{
  return d->buttons;
}


#if 0
// Get the KAboutData stored object
const KAboutData* LikeBack::aboutData()
{
  return d->aboutData;
}


// Get the KDE config stored object
KConfig *LikeBack::config()
{
  return d->config.config();
}



// Create the menu actions
void LikeBack::createActions( KActionCollection *parent )
{
  if( d->sendAction == 0 )
  {
    d->sendAction = new KAction( KIcon("mail-message-new"), i18n("&Send a Comment to the Developers"), this );
    connect( d->sendAction, SIGNAL(         triggered(bool) ),
             this,          SLOT  ( execCommentDialog()     ) );

    parent->addAction( "likeBackSendComment", d->sendAction );
  }

  if( d->showBarAction == 0 )
  {
    d->showBarAction = new KToggleAction( i18n("Show &Feedback Icons"), this );
    d->showBarAction->setChecked( userWantsToShowBar() );
    connect( d->showBarAction, SIGNAL(             triggered(bool) ),
             this,             SLOT  ( setUserWantsToShowBar(bool) ) );

    parent->addAction( "likeBackShowIcons", d->showBarAction );
  }
}
#endif


// Return whether the user wants to enable the likeback bar or not
bool LikeBack::userWantsToShowBar()
{
#if 1
  return QSettings().value("LikeBack/userWantToShowBar", d->showBarByDefault).toBool();
#else
  // You can choose to store the button bar status per version.
  // On debug builds from SVN, where the version changes at almost every build,
  // it's very annoying to have the bar reappearing everytime.
//   return d->config.readEntry( "userWantToShowBarForVersion_" + d->aboutData->version(), d->showBarByDefault );
  return d->config.readEntry( "userWantToShowBar", d->showBarByDefault );
#endif
}



// Set whether the user wants to enable the likeback bar or not
void LikeBack::setUserWantsToShowBar( bool showBar )
{
  if( showBar == d->showBar )
    return;

  d->showBar = showBar;
#if 1
  QSettings().setValue("LikeBack/userWantToShowBar", showBar);
  // TODO: show bar
#else
  // You can choose to store the button bar status per version.
  // On debug builds from SVN, where the version changes at almost every build,
  // it's very annoying to have the bar reappearing everytime.
//   d->config.writeEntry( "userWantToShowBarForVersion_" + d->aboutData->version(), showBar );

  d->config.writeEntry( "userWantToShowBar", showBar );

  d->config.sync(); // Make sure the option is saved, even if the application crashes after that.

  d->bar->setBarVisible( showBar );
#endif
}



// Show a dialog box to introduce the user to LikeBack
void LikeBack::showInformationMessage()
{
  // don't show the message if the bar isn't enabled.
  // message doesn't make sense without the bar
  if ( ! d->showBar ) return;

#if 0
  // Load and register the images needed by the message:
  KIconLoader *loader = KIconLoader::global();
  QString likeIconPath   ( loader->iconPath( "likeback_like",    KIconLoader::Small ) );
  QString dislikeIconPath( loader->iconPath( "likeback_dislike", KIconLoader::Small ) );
  QString bugIconPath    ( loader->iconPath( "likeback_bug",     KIconLoader::Small ) );
  QString featureIconPath( loader->iconPath( "likeback_feature", KIconLoader::Small ) );

  // Show a message reflecting the allowed types of comment:
  Button buttons = d->buttons;
  int nbButtons = ( buttons & Like    ? 1 : 0 ) +
                  ( buttons & Dislike ? 1 : 0 ) +
                  ( buttons & Bug     ? 1 : 0 ) +
                  ( buttons & Feature ? 1 : 0 );

  // Construct the welcome phrase
  QString welcomePhrase;
  if( isDevelopmentVersion( d->aboutData->version() ) )
  {
    welcomePhrase = i18nc( "Welcome dialog text, header text for test apps",
                           "Welcome to this testing version of %1.", d->aboutData->programName() );
  }
  else
  {
    welcomePhrase = i18nc( "Welcome dialog text, header text for released apps",
                           "Welcome to %1.", d->aboutData->programName() );
  }

  // Construct the like and dislike explanation
  QString likeAndDislikePhrase;
  if( ( buttons & LikeBack::Like ) && ( buttons & LikeBack::Dislike ) )
  {
    likeAndDislikePhrase = i18nc( "Welcome dialog text, explanation for both the like and dislike buttons",
                                  "Each time you have a great or frustrating experience, "
                                  "please click on the appropriate face below the window title-bar, "
                                  "briefly describe what you like or dislike and click on 'Send'." );
  }
  else if( buttons & LikeBack::Like )
  {
    likeAndDislikePhrase = i18nc( "Welcome dialog text, explanation for the like button alone",
                                  "Each time you have a great experience, "
                                  "please click on the smiling face below the window title-bar, "
                                  "briefly describe what you like and click on 'Send'." );
  }
  else
  {
    likeAndDislikePhrase = i18nc( "Welcome dialog text, explanation for the dislike button alone",
                                  "Each time you have a frustrating experience, "
                                  "please click on the frowning face below the window title-bar, "
                                  "briefly describe what you dislike and click on 'Send'." );
  }

  // Construct the bug report explanation
  QString bugPhrase;
  if( buttons & LikeBack::Bug )
  {
    bugPhrase = i18nc( "Welcome dialog text, explanation for the bug button",
                       "If you experience an improper behavior in the application, just click on "
                       "the broken-object icon in the top-right corner of the window, describe the "
                       "behavior and click on 'Send'." );
  }

  // Construct the usage examples
  QString examplesBlocks;
  if( buttons & LikeBack::Like )
  {
    examplesBlocks += "<img src=\"" + likeIconPath + "\"/> &nbsp;"
                      "<span>" +
                      i18nc( "Welcome dialog text, usage example",
                             "<b>I like</b> the new artwork. Very refreshing." ) +
                      "</span><br/>";
  }
  if( buttons & LikeBack::Dislike )
  {
    examplesBlocks += "<img src=\"" + dislikeIconPath + "\"/> &nbsp;"
                      "<span>" +
                      i18nc( "Welcome dialog text, usage example",
                             "<b>I dislike</b> the welcome page of this assistant. Too time consuming." ) +
                      "</span><br/>";
  }
  if( buttons & LikeBack::Bug )
  {
    examplesBlocks += "<img src=\"" + bugIconPath + "\"/> &nbsp;"
                      "<span>" +
                      i18nc( "Welcome dialog text, usage example",
                             "<b>The application shows an improper behaviour</b> when clicking the Add button. Nothing happens." ) +
                      "</span><br/>";
  }
  if( buttons & LikeBack::Feature )
  {
    examplesBlocks += "<img src=\"" + featureIconPath + "\"/> &nbsp;"
                      "<span>" +
                      i18nc( "Welcome dialog text, usage example",
                             "<b>I desire a new feature</b> allowing me to send my work by email." ) +
                      "</span>";
  }

  // Finally, merge all the strings together
  QString dialogText( "<html><h3>%1</h3>"
                      "<p>%2</p>"
                      "<p>%3</p>"
                      "<p>%4</p>"
                      "<h3>%5:</h3>"
                      "<p>%6</p></html>" );
  dialogText = dialogText.arg( welcomePhrase )
                         .arg( i18nc( "Welcome dialog text, us=the developers, it=the application",
                                      "To help us improve it, your comments are important." ) )
                         .arg( likeAndDislikePhrase )
                         .arg( bugPhrase )
                         .arg( i18ncp( "Welcome dialog text, header for the examples", "Example", "Examples", nbButtons ) )
                         .arg( examplesBlocks );

  // And show them
  KMessageBox::information( 0,
                            dialogText,
                            i18nc( "Welcome dialog title", "Help Improve the Application" ),
                            "LikeBack_starting_information",
                            KMessageBox::Notify );
#endif
}



// Return the current window hierarchy
QString LikeBack::activeWindowPath()
{
  // Compute the window hierarchy (from the oldest to the latest, each time prepending to the list):
  QStringList windowNames;
  QWidget *window = QApplication::activeWindow();
  while( window ) {
    QString name( window->objectName() );

    // Append the class name to the window name if it is unnamed:
    if( name == "unnamed" && window->metaObject() )
      name += QString( ":" ) + window->metaObject()->className();
    windowNames.prepend( name );

    window = dynamic_cast<QWidget*>( window->parent() );
  }

  // Return the string of windows starting by the end (from the oldest to the latest):
  return windowNames.join( " -> " );
}



// Return whether the email address was confirmed by the user
bool LikeBack::emailAddressAlreadyProvided()
{
#if 1
  return QSettings().value("LikeBack/emailAlreadyAsked", false).toBool();
#else
  return d->config.readEntry( "emailAlreadyAsked", false );
#endif
}



// Return the currently saved email address, or the account's email address, if present
QString LikeBack::emailAddress()
{
#if 1
  return QSettings().value("LikeBack/emailAddress").toString();
#else
  KEMailSettings emailSettings;
  return d->config.readEntry( "emailAddress", emailSettings.getSetting( KEMailSettings::EmailAddress ) );
#endif
}



// Change the saved email address
void LikeBack::setEmailAddress( const QString &address, bool userProvided )
{
#if 1
  QSettings s;
  s.setValue("LikeBack/emailAddress", address);
  s.setValue("LikeBack/emailAlreadyAsked", userProvided || s.value("LikeBack/emailAlreadyAsked", false).toBool());
#else
  d->config.writeEntry( "emailAddress", address );
  d->config.writeEntry( "emailAlreadyAsked", ( userProvided || emailAddressAlreadyProvided() ) );
  d->config.sync(); // Make sure the option is saved, even if the application crashes after that.
#endif
}



// FIXME: Should be moved to KAboutData? Cigogne will also need it.
bool LikeBack::isDevelopmentVersion( const QString &version )
{
    return version.indexOf( "alpha", 0, Qt::CaseInsensitive ) != -1 ||
           version.indexOf( "beta",  0, Qt::CaseInsensitive ) != -1 ||
           version.indexOf( "rc",    0, Qt::CaseInsensitive ) != -1 ||
           version.indexOf( "svn",   0, Qt::CaseInsensitive ) != -1 ||
           version.indexOf( "cvs",   0, Qt::CaseInsensitive ) != -1;
}



// Return whether the Like button is active
bool LikeBack::isLikeActive() const
{
    return ( d->buttons & Like );
}



// Return whether the Dislike button is active
bool LikeBack::isDislikeActive() const
{
    return ( d->buttons & Dislike );
}



// Return whether the Bug button is active
bool LikeBack::isBugActive() const
{
    return ( d->buttons & Bug );
}



// Return whether the Feature button is active
bool LikeBack::isFeatureActive() const
{
    return ( d->buttons & Feature );
}

