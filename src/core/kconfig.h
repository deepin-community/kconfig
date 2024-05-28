/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIG_H
#define KCONFIG_H

#include "kconfigbase.h"

#include <kconfigcore_export.h>

#include <QByteArray>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QVariant>

class KConfigGroup;
class KEntryMap;
class KConfigPrivate;

/**
 * \class KConfig kconfig.h <KConfig>
 *
 * \brief The central class of the KDE configuration data system.
 *
 * Quickstart:
 *
 * Get the default application config object via KSharedConfig::openConfig().
 *
 * Load a specific configuration file:
 * \code
 * KConfig config( "/etc/kderc", KConfig::SimpleConfig );
 * \endcode
 *
 * Load the configuration of a specific component:
 * \code
 * KConfig config( "pluginrc" );
 * \endcode
 *
 * In general it is recommended to use KSharedConfig instead of
 * creating multiple instances of KConfig to avoid the overhead of
 * separate objects or concerns about synchronizing writes to disk
 * even if the configuration object is updated from multiple code paths.
 * KSharedConfig provides a set of open methods as counterparts for the
 * KConfig constructors.
 *
 * \sa KSharedConfig, KConfigGroup, <a href="https://techbase.kde.org/index.php?title=Development/Tutorials/KConfig">the techbase HOWTO on KConfig</a>.
 */
class KCONFIGCORE_EXPORT KConfig : public KConfigBase
{
public:
    /**
     * Determines how the system-wide and user's global settings will affect
     * the reading of the configuration.
     *
     * If CascadeConfig is selected, system-wide configuration sources are used
     * to provide defaults for the settings accessed through this object, or
     * possibly to override those settings in certain cases.
     *
     * If IncludeGlobals is selected, the kdeglobals configuration is used
     * as additional configuration sources to provide defaults. Additionally
     * selecting CascadeConfig will result in the system-wide kdeglobals sources
     * also getting included.
     *
     * Note that the main configuration source overrides the cascaded sources,
     * which override those provided to addConfigSources(), which override the
     * global sources.  The exception is that if a key or group is marked as
     * being immutable, it will not be overridden.
     *
     * Note that all values other than IncludeGlobals and CascadeConfig are
     * convenience definitions for the basic mode.
     * Do @em not combine them with anything.
     * @see OpenFlags
     */
    enum OpenFlag {
        IncludeGlobals = 0x01, ///< Blend kdeglobals into the config object.
        CascadeConfig = 0x02, ///< Cascade to system-wide config files.

        SimpleConfig = 0x00, ///< Just a single config file.
        NoCascade = IncludeGlobals, ///< Include user's globals, but omit system settings.
        NoGlobals = CascadeConfig, ///< Cascade to system settings, but omit user's globals.
        FullConfig = IncludeGlobals | CascadeConfig, ///< Fully-fledged config, including globals and cascading to system settings
    };
    /**
     * Stores a combination of #OpenFlag values.
     */
    Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

    /**
     * Creates a KConfig object to manipulate a configuration file for the
     * current application.
     *
     * If an absolute path is specified for @p file, that file will be used
     * as the store for the configuration settings.  If a non-absolute path
     * is provided, the file will be looked for in the standard directory
     * specified by type.  If no path is provided, a default
     * configuration file will be used based on the name of the main
     * application component.
     *
     * @p mode determines whether the user or global settings will be allowed
     * to influence the values returned by this object.  See OpenFlags for
     * more details.
     *
     * @note You probably want to use KSharedConfig::openConfig instead.
     *
     * @param file         the name of the file. If an empty string is passed in
     *                     and SimpleConfig is passed in for the OpenFlags, then an in-memory
     *                     KConfig object is created which will not write out to file nor which
     *                     requires any file in the filesystem at all.
     * @param mode         how global settings should affect the configuration
     *                     options exposed by this KConfig object
     * @param type         The standard directory to look for the configuration
     *                     file in
     *
     * @sa KSharedConfig::openConfig(const QString&, OpenFlags, QStandardPaths::StandardLocation)
     */
    explicit KConfig(const QString &file = QString(),
                     OpenFlags mode = FullConfig,
                     QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation);

    /**
     * @internal
     *
     * Creates a KConfig object using the specified backend. If the backend can not
     * be found or loaded, then the standard configuration parser is used as a fallback.
     *
     * @param file the file to be parsed
     * @param backend the backend to load
     * @param type where to look for the file if an absolute path is not provided
     *
     * @since 4.1
     */
    KConfig(const QString &file, const QString &backend, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation);

    ~KConfig() override;

    /**
     * Returns the standard location enum passed to the constructor.
     * Used by KSharedConfig.
     * @since 5.0
     */
    QStandardPaths::StandardLocation locationType() const;

    /**
     * Returns the filename used to store the configuration.
     */
    QString name() const;

    /**
     * @return the flags this object was opened with
     * @since 5.3
     */
    OpenFlags openFlags() const;

    /// @reimp
    bool sync() override;

    /// Returns true if sync has any changes to write out.
    /// @since 4.12
    bool isDirty() const;

    /// @reimp
    void markAsClean() override;

    /// @{ configuration object state
    /// @reimp
    AccessMode accessMode() const override;

    /**
     * Whether the configuration can be written to.
     *
     * If @p warnUser is true and the configuration cannot be
     * written to (ie: this method returns @c false), a warning
     * message box will be shown to the user telling them to
     * contact their system administrator to get the problem fixed.
     *
     * The most likely cause for this method returning @c false
     * is that the user does not have write permission for the
     * configuration file.
     *
     * @param warnUser whether to show a warning message to the user
     *                 if the configuration cannot be written to
     *
     * @returns true if the configuration can be written to, false
     *          if the configuration cannot be written to
     */
    bool isConfigWritable(bool warnUser);
    /// @}

    /**
     * Copies all entries from this config object to a new config
     * object that will save itself to @p file.
     *
     * The configuration will not actually be saved to @p file
     * until the returned object is destroyed, or sync() is called
     * on it.
     *
     * Do not forget to delete the returned KConfig object if
     * @p config was 0.
     *
     * @param file   the new config object will save itself to
     * @param config if not 0, copy to the given KConfig object rather
     *               than creating a new one
     *
     * @return @p config if it was set, otherwise a new KConfig object
     */
    KConfig *copyTo(const QString &file, KConfig *config = nullptr) const;

    /**
     * Ensures that the configuration file contains a certain update.
     *
     * If the configuration file does not contain the update @p id
     * as contained in @p updateFile, kconf_update is run to update
     * the configuration file.
     *
     * If you install config update files with critical fixes
     * you may wish to use this method to verify that a critical
     * update has indeed been performed to catch the case where
     * a user restores an old config file from backup that has
     * not been updated yet.
     *
     * @param id the update to check
     * @param updateFile the file containing the update
     */
    void checkUpdate(const QString &id, const QString &updateFile);

    /**
     * Updates the state of this object to match the persistent storage.
     * Note that if this object has pending changes, this method will
     * call sync() first so as not to lose those changes.
     */
    void reparseConfiguration();

    /// @{ extra config files
    /**
     * Adds the list of configuration sources to the merge stack.
     *
     * Currently only files are accepted as configuration sources.
     *
     * The first entry in @p sources is treated as the most general and will
     * be overridden by the second entry.  The settings in the final entry
     * in @p sources will override all the other sources provided in the list.
     *
     * The settings in @p sources will also be overridden by the sources
     * provided by any previous calls to addConfigSources().
     *
     * The settings in the global configuration sources will be overridden by
     * the sources provided to this method (@see IncludeGlobals).
     * All the sources provided to any call to this method will be overridden
     * by any files that cascade from the source provided to the constructor
     * (@see CascadeConfig), which will in turn be
     * overridden by the source provided to the constructor.
     *
     * Note that only the most specific file, ie: the file provided to the
     * constructor, will be written to by this object.
     *
     * The state is automatically updated by this method, so there is no need to call
     * reparseConfiguration().
     *
     * @param sources A list of extra config sources.
     */
    void addConfigSources(const QStringList &sources);

    /**
     * Returns a list of the additional configuration sources used in this object
     */
    QStringList additionalConfigSources() const;

    /// @}
    /// @{ locales
    /**
     * Returns the current locale.
     */
    QString locale() const;
    /**
     * Sets the locale to @p aLocale.
     *
     * The global locale is used by default.
     *
     * @note If set to the empty string, @b no locale will be matched. This effectively disables
     * reading translated entries.
     *
     * @return @c true if locale was changed, @c false if the call had no
     *         effect (eg: @p aLocale was already the current locale for this
     *         object)
     */
    bool setLocale(const QString &aLocale);
    /// @}

    /// @{ defaults
    /**
     * When set, all readEntry calls return the system-wide (default) values
     * instead of the user's settings.
     *
     * This is off by default.
     *
     * @param b whether to read the system-wide defaults instead of the
     *          user's settings
     */
    void setReadDefaults(bool b);
    /**
     * @returns @c true if the system-wide defaults will be read instead of the
     *          user's settings
     */
    bool readDefaults() const;
    /// @}

    /// @{ immutability
    /// @reimp
    bool isImmutable() const override;
    /// @}

    /// @{ global
#if KCONFIGCORE_ENABLE_DEPRECATED_SINCE(4, 0)
    /**
     * Forces all following write-operations to be performed on @c kdeglobals,
     * independent of the @c Global flag in writeEntry().
     *
     * @param force true to force writing to kdeglobals
     * @see forceGlobal
     * @deprecated Since 4.0
     */
    KCONFIGCORE_DEPRECATED_VERSION(4, 0, "Not recommended")
    void setForceGlobal(bool force);
#endif

#if KCONFIGCORE_ENABLE_DEPRECATED_SINCE(4, 0)
    /**
     * Returns whether all entries are being written to @c kdeglobals.
     *
     * @return @c true if all entries are being written to @c kdeglobals
     * @see setForceGlobal
     * @deprecated Since 4.0
     */
    KCONFIGCORE_DEPRECATED_VERSION(4, 0, "Not recommended")
    bool forceGlobal() const;
#endif
    /// @}

    /// @reimp
    QStringList groupList() const override;

    /**
     * Returns a map (tree) of entries in a particular group.
     *
     * The entries are all returned as strings.
     *
     * @param aGroup The group to get entries from.
     *
     * @return A map of entries in the group specified, indexed by key.
     *         The returned map may be empty if the group is empty, or not found.
     * @see   QMap
     */
    QMap<QString, QString> entryMap(const QString &aGroup = QString()) const;

    /**
     * Sets the name of the application config file.
     * @since 5.0
     */
    static void setMainConfigName(const QString &str);

    /**
     * Get the name of application config file.
     * @since 5.93
     */
    static QString mainConfigName();

protected:
    bool hasGroupImpl(const QByteArray &group) const override;
    KConfigGroup groupImpl(const QByteArray &b) override;
    const KConfigGroup groupImpl(const QByteArray &b) const override;
    void deleteGroupImpl(const QByteArray &group, WriteConfigFlags flags = Normal) override;
    bool isGroupImmutableImpl(const QByteArray &aGroup) const override;

    friend class KConfigGroup;
    friend class KConfigGroupPrivate;
    friend class KSharedConfig;

    /** Virtual hook, used to add new "virtual" functions while maintaining
     * binary compatibility. Unused in this class.
     */
    void virtual_hook(int id, void *data) override;

    KConfigPrivate *const d_ptr;

    KCONFIGCORE_NO_EXPORT explicit KConfig(KConfigPrivate &d);

private:
    friend class KConfigTest;

    Q_DISABLE_COPY(KConfig)

    Q_DECLARE_PRIVATE(KConfig)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfig::OpenFlags)

#endif // KCONFIG_H
