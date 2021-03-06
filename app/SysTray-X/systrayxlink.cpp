#include "systrayxlink.h"


/*
 *	Local includes
 */
#include "preferences.h"


/*
 *  System includes
 */
#include <iostream>
#ifdef Q_OS_WIN
#include <fcntl.h>
#include <io.h>
#endif


/*
 *	Qt includes
 */
#include <QFile>
#include <QTimer>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QJsonValue>
#include <QJsonObject>


/*****************************************************************************
 *
 *  SysTrayXLinkReader Class
 *
 *****************************************************************************/


/*
 *	Constructor
 */
SysTrayXLinkReader::SysTrayXLinkReader()
{
    /*
     *  Set stdin to binary
     */
#ifdef Q_OS_WIN
    _setmode( _fileno( stdin ), _O_BINARY);
#endif

    /*
     *	Setup the timer
     */
    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, &QTimer::timeout, this, &SysTrayXLinkReader::slotWorker );
}


/*
 *	Destructor
 */
SysTrayXLinkReader::~SysTrayXLinkReader()
{
    /*
     *  Cleanup
     */
    m_timer->stop();
    delete m_timer;
}


/*
 *	Start reading
 */
void    SysTrayXLinkReader::startThread()
{
    /*
     *	Start the work
     */
    m_doWork = true;

    /*
     *	Start the worker
     */
    m_timer->start();
}


/*
 *	Stop viewing
 */
void    SysTrayXLinkReader::stopThread()
{
    /*
     *	Stop working
     */
    m_doWork = false;
}


/*
 *	Read the data
 */
void    SysTrayXLinkReader::slotWorker()
{
    int error_count = 0;

    while( m_doWork )
    {
        qint32 data_len;
        std::cin.read( reinterpret_cast< char* >( &data_len ), sizeof( qint32 ) );

        if( data_len > 0)
        {
            QByteArray data( data_len, 0 );
            std::cin.read( data.data(), data_len );

            /*
             *  Send the data to my parent
             */
            emit signalReceivedMessage( data );

            /*
             *  Send the data to my parent
             */
            if( data.at( 0 ) == '{' )
            {
                emit signalReceivedMessage( data );

                error_count = 0;
            }
            else
            {
                error_count++;

                if( error_count > 20 )
                {
                    emit signalAddOnShutdown();
                }
            }
        }
    }

    /*
     *	Quit this thread
     */
    QThread::currentThread()->quit();
}


/*****************************************************************************
 *
 *  SysTrayXLink Class
 *
 *****************************************************************************/


/*
 *	Constructor
 */
SysTrayXLink::SysTrayXLink( Preferences* pref )
{
    /*
     *  Store preferences
     */
    m_pref = pref;

    /*
     *  Open dump.txt
     */
//    m_dump = new QFile( "dump.txt", this );
//    m_dump->open( QIODevice::WriteOnly );

    /*
     *  Setup the reader thread
     */
    m_reader_thread = new QThread( this );

    SysTrayXLinkReader* reader = new SysTrayXLinkReader;
    reader->moveToThread( m_reader_thread );

    connect( m_reader_thread, &QThread::finished, reader, &QObject::deleteLater );
    connect( reader, &SysTrayXLinkReader::signalReceivedMessage, this, &SysTrayXLink::slotLinkRead );
    connect( reader, &SysTrayXLinkReader::signalAddOnShutdown, this, &SysTrayXLink::slotAddOnShutdown );

    connect( m_reader_thread, &QThread::started, reader, &SysTrayXLinkReader::startThread, Qt::QueuedConnection );
    m_reader_thread->start();
}


/*
 *	Destructor
 */
SysTrayXLink::~SysTrayXLink()
{
    /*
     *  Cleanup
     */
//    m_dump->close();
//    delete m_dump;
}


/*
 *  Write a message to the link
 */
void    SysTrayXLink::linkWrite( const QByteArray& message )
{
    qint32  msglen = message.length();
    std::cout.write( reinterpret_cast< char* >( &msglen ), sizeof( qint32 ) );
    std::cout.write( message.data(), msglen ) << std::flush;
}


/*
 *  Send the preferences to the add-on
 */
void    SysTrayXLink::sendPreferences()
{
    /*
     *  Enacode the preferences into a JSON doc
     */
    EncodePreferences( *m_pref );

    /*
     *  Send them to the add-on
     */
    linkWrite( m_pref_json_doc.toJson( QJsonDocument::Compact ) );
}


/*
 *  Send the window normal command
 */
void    SysTrayXLink::sendWindowNormal()
{
    /*
     *  Create command
     */
    QJsonObject windowObject;
    windowObject.insert( "window", "normal" );

    /*
     *  Create doc
     */
    QJsonDocument doc( windowObject );

    /*
     *  Send the command
     */
    linkWrite( doc.toJson( QJsonDocument::Compact ) );
}


/*
 *  Send the window minimize command
 */
void    SysTrayXLink::sendWindowMinimize()
{
    /*
     *  Create command
     */
    QJsonObject windowObject;
    windowObject.insert("window", "minimized" );

    /*
     *  Create doc
     */
    QJsonDocument doc( windowObject );

    /*
     *  Send the command
     */
    linkWrite( doc.toJson( QJsonDocument::Compact ) );
}


/*
 *  Decode JSON message
 */
void    SysTrayXLink::DecodeMessage( const QByteArray& message )
{
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson( message, &jsonError );

    if( jsonError.error == QJsonParseError::NoError )
    {
        QJsonObject jsonObject = jsonResponse.object();

        if( jsonObject.contains( "unreadMail" ) && jsonObject[ "unreadMail" ].isDouble() )
        {
            int unreadMail = jsonObject[ "unreadMail" ].toInt();
            emit signalUnreadMail( unreadMail );
        }

        if( jsonObject.contains( "title" ) && jsonObject[ "title" ].isString() )
        {
            QString title = jsonObject[ "title" ].toString();
            emit signalTitle( title );
        }

        if( jsonObject.contains( "shutdown" ) && jsonObject[ "shutdown" ].isString() )
        {
            emit signalAddOnShutdown();
        }

        if( jsonObject.contains( "window" ) && jsonObject[ "window" ].isString() )
        {
            QString window_state = jsonObject[ "window" ].toString();
            emit signalWindowState( window_state );
        }

        if( jsonObject.contains( "preferences" ) && jsonObject[ "preferences" ].isObject() )
        {
            DecodePreferences( jsonObject[ "preferences" ].toObject() );
        }
    }
}


/*
 *  Decode preferences from JSON message
 */
void    SysTrayXLink::DecodePreferences( const QJsonObject& pref )
{ 
    /*
     *  Check the received object
     */
    if( pref.contains( "iconType" ) && pref[ "iconType" ].isString() )
    {
        Preferences::IconType icon_type = static_cast< Preferences::IconType >( pref[ "iconType" ].toString().toInt() );

        /*
         *  Store the new icon type
         */
        m_pref->setIconType( icon_type );
    }

    if( pref.contains( "iconMime" ) && pref[ "iconMime" ].isString() )
    {
        QString icon_mime = pref[ "iconMime" ].toString();

        /*
         *  Store the new icon mime
         */
        m_pref->setIconMime( icon_mime );
    }

    if( pref.contains( "icon" ) && pref[ "icon" ].isString() )
    {
        QString icon_base64 = pref[ "icon" ].toString();

        /*
         *  Store the new icon data
         */
        m_pref->setIconData( QByteArray::fromBase64( icon_base64.toUtf8() ) );
    }

    if( pref.contains( "hideOnMinimize" ) && pref[ "hideOnMinimize" ].isString() )
    {
        bool hide_minimize = pref[ "hideOnMinimize" ].toString() == "true";

        /*
         *  Store the new hide on minimize state
         */
        m_pref->setHideOnMinimize( hide_minimize );
    }

    if( pref.contains( "startMinimized" ) && pref[ "startMinimized" ].isString() )
    {
        bool start_minimized = pref[ "startMinimized" ].toString() == "true";

        /*
         *  Store the new start minimized state
         */
        m_pref->setStartMinimized( start_minimized );
    }

    if( pref.contains( "debug" ) && pref[ "debug" ].isString() )
    {
        bool debug = pref[ "debug" ].toString() == "true";

        /*
         *  Store the new debug state
         */
        m_pref->setDebug( debug );
    }
}


/*
 *  Encode preferences to JSON message
 */
void    SysTrayXLink::EncodePreferences( const Preferences& pref )
{
    /*
     *  Setup the preferences JSON
     */
    QJsonObject prefObject;
    prefObject.insert("debug", QJsonValue::fromVariant( QString( pref.getDebug() ? "true" : "false" ) ) );
    prefObject.insert("hideOnMinimize", QJsonValue::fromVariant( QString( pref.getHideOnMinimize() ? "true" : "false" ) ) );
    prefObject.insert("startMinimized", QJsonValue::fromVariant( QString( pref.getStartMinimized() ? "true" : "false" ) ) );
    prefObject.insert("iconType", QJsonValue::fromVariant( QString::number( pref.getIconType() ) ) );
    prefObject.insert("iconMime", QJsonValue::fromVariant( pref.getIconMime() ) );
    prefObject.insert("icon", QJsonValue::fromVariant( QString( pref.getIconData().toBase64() ) ) );

    QJsonObject preferencesObject;
    preferencesObject.insert("preferences", prefObject );

    /*
     *  Store the new document
     */
    m_pref_json_doc = QJsonDocument( preferencesObject );
}


/*
 *  Relay shutdown signal
 */
void    SysTrayXLink::slotAddOnShutdown()
{
    emit signalAddOnShutdown();
}


/*
 *  Read the input
 */
void    SysTrayXLink::slotLinkRead( QByteArray message )
{
    /*
     *  Decode the message
     */
    DecodeMessage( message );
}


/*
 *  Handle a debug state change signal
 */
void    SysTrayXLink::slotDebugChange()
{
    if( m_pref->getAppPrefChanged() )
    {
        sendPreferences();
    }
}


/*
 *  Handle a hide on minimize state change signal
 */
void    SysTrayXLink::slotHideOnMinimizeChange()
{
    if( m_pref->getAppPrefChanged() )
    {
        sendPreferences();
    }
}


/*
 *  Handle a start minimized state change signal
 */
void    SysTrayXLink::slotStartMinimizedChange()
{
    if( m_pref->getAppPrefChanged() )
    {
        sendPreferences();
    }
}


/*
 *  Handle the icon type change signal
 */
void    SysTrayXLink::slotIconTypeChange()
{
    if( m_pref->getAppPrefChanged() )
    {
        sendPreferences();
    }
}


/*
 *  Handle the icon data change signal
 */
void    SysTrayXLink::slotIconDataChange()
{
    if( m_pref->getAppPrefChanged() )
    {
        sendPreferences();
    }
}


/*
 *  Handle the window normal signal
 */
void    SysTrayXLink::slotWindowNormal()
{
    sendWindowNormal();
}


/*
 *  Handle the window minimize signal
 */
void    SysTrayXLink::slotWindowMinimize()
{
    sendWindowMinimize();
}
