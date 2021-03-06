#ifndef PREFERENCES_H
#define PREFERENCES_H

/*
 *	Local includes
 */

/*
 *	Qt includes
 */
#include <QObject>
#include <QString>
#include <QByteArray>

/**
 * @brief The Preferences class. Class to hold the preferences.
 */
class Preferences : public QObject
{
    Q_OBJECT

    public:

        /*
         *  Icon types
         */
        enum IconType {
            PREF_BLANK_ICON = 0,
            PREF_NEWMAIL_ICON,
            PREF_CUSTOM_ICON
        };

    public:

        /**
         * @brief Preferences.  Constructor. Destructor.
         */
        Preferences( QObject *parent = nullptr );

        /**
         * @brief getAppPrefChanged. Control for sending changes to the add-on.
         *
         * @return      The state
         */
        bool getAppPrefChanged() const;

        /**
         * @brief setAppPrefChanged. Control for sending changes to the add-on.
         *
         *  @param state    The state
         */
        void setAppPrefChanged( bool state );

        /**
         * @brief getIconType. Get the icon type.
         *
         * @return      The icon type.
         */
        IconType getIconType() const;

        /**
         * @brief setIconType. Set the icon type.
         *
         * @param      The icon type.
         */
        void setIconType( IconType icon_type );

        /**
         * @brief getIconMime. Get the icon mime.
         *
         * @return      The icon mime.
         */
        const QString& getIconMime() const;

        /**
         * @brief setIconMime. Set the icon mime.
         *
         * @param      The icon mime.
         */
        void setIconMime( const QString& icon_mime );

        /**
         * @brief getIconData. Get the icon data.
         *
         * @return      The icon data.
         */
        const QByteArray& getIconData() const;

        /**
         * @brief setIconData. Set the icon data.
         *
         * @param      The icon data.
         */
        void setIconData( const QByteArray& icon_data );

        /**
         * @brief getHideOnMinimize. Get the hide on minimize state.
         *
         * @return      The state.
         */
        bool getHideOnMinimize() const;

        /**
         * @brief setHideOnMinimize. Set the hide on minimize state.
         *
         * @param      The state.
         */
        void setHideOnMinimize( bool state );

        /**
         * @brief getStartMinimized. Get the start minimized state.
         *
         * @return      The state.
         */
        bool getStartMinimized() const;

        /**
         * @brief setStartMinimized. Set the start minimized state.
         *
         * @param      The state.
         */
        void setStartMinimized( bool state );

        /**
         * @brief getDebug. Get the debug windows state.
         *
         * @return      The state.
         */
        bool getDebug() const;

        /**
         * @brief setDebug. Set the debug windows state.
         *
         * @param      The state.
         */
        void setDebug( bool state );

    signals:

        /**
         * @brief signalIconTypeChange. Signal a icon type change.
         */
        void signalIconTypeChange();

        /**
         * @brief signalIconDataChange. Signal a icon data change.
         */
        void signalIconDataChange();

        /**
         * @brief signalHideOnMinimizeChange. Signal a hide on mnimize state change.
         */
        void signalHideOnMinimizeChange();

        /**
         * @brief signalStartMinimizedChange. Signal a start minimized state change.
         */
        void signalStartMinimizedChange();

        /**
         * @brief signalDebugChange. Signal a debug state change.
         */
        void signalDebugChange();

    private:

        /**
         * @brief m_app_pref_changed. Control for sending changes to the add-on.
         */
        bool m_app_pref_changed;

        /**
         * @brief m_icon_type. Selected icon type.
         */
        IconType m_icon_type;

        /**
         * @brief m_icon_mime. Selected icon mime.
         */
        QString m_icon_mime;

        /**
         * @brief m_icon_data. Binary data icon image.
         */
        QByteArray m_icon_data;

        /**
         * @brief m_hide_minimize. Hide the minimized window.
         */
        bool m_hide_minimize;

        /**
         * @brief m_start_minimized. Start TB minimized.
         */
        bool m_start_minimized;

        /**
         * @brief m_debug. Display debug window.
         */
        bool m_debug;
};

#endif // PREFERENCES_H
