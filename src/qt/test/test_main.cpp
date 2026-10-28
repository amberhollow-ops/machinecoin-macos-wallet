// Copyright (c) 2009-2017 The Machinecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/machinecoin-config.h>
#endif

#include <chainparams.h>
#include <qt/test/rpcnestedtests.h>
#include <util.h>
#include <qt/test/uritests.h>
#include <qt/test/compattests.h>

#ifdef ENABLE_WALLET
#include <qt/test/addressbooktests.h>
#include <qt/test/paymentservertests.h>
#include <qt/test/wallettests.h>
#endif

#include <QApplication>
#include <QObject>
#include <QTest>
#include <QDir>
#include <QSettings>

#include <openssl/ssl.h>

#if defined(QT_STATICPLUGIN)
#include <QtPlugin>
#if defined(QT_QPA_PLATFORM_MINIMAL)
Q_IMPORT_PLUGIN(QMinimalIntegrationPlugin);
#endif
#if defined(QT_QPA_PLATFORM_XCB)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_WINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#elif defined(QT_QPA_PLATFORM_COCOA)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin);
#endif
#endif

extern void noui_connect();

// Windows GUI-subsystem executables may not expose stdout to the test runner.
// Allow explicit per-suite reports without changing which assertions execute.
static int RunQtTest(QObject* test)
{
    const QString logDir = QString::fromLocal8Bit(qgetenv("MACHINECOIN_QT_TEST_LOGDIR"));
    if (logDir.isEmpty()) return QTest::qExec(test);
    if (!QDir(logDir).isAbsolute() || !QDir().mkpath(logDir)) return 1;
    const QString report = QDir(logDir).filePath(QString::fromLatin1(test->metaObject()->className()) + ".txt");
    return QTest::qExec(test, QStringList() << "machinecoin-qt-test" << "-o" << (report + ",txt"));
}

// This is all you need to run all the tests
int main(int argc, char *argv[])
{
    SetupEnvironment();
    SetupNetworking();
    SelectParams(CBaseChainParams::MAIN);
    noui_connect();
    ClearDatadirCache();
    fs::path pathTemp = fs::temp_directory_path() / strprintf("test_machinecoin-qt_%lu_%i", (unsigned long)GetTime(), (int)GetRand(100000));
    fs::create_directories(pathTemp);
    gArgs.ForceSetArg("-datadir", pathTemp.string());

    bool fInvalid = false;

    // Prefer the "minimal" platform for the test instead of the normal default
    // platform ("xcb", "windows", or "cocoa") so tests can't unintentionally
    // interfere with any background GUIs and don't require extra resources.
    #if defined(WIN32)
        _putenv_s("QT_QPA_PLATFORM", "minimal");
    #else
        setenv("QT_QPA_PLATFORM", "minimal", 0);
    #endif

    // Don't remove this, it's needed to access
    // QApplication:: and QCoreApplication:: in the tests
    QApplication app(argc, argv);
    app.setApplicationName("Machinecoin-Qt-test");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    const QString settingsPath = QString::fromStdString(pathTemp.string());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsPath);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, settingsPath);

    SSL_library_init();

    URITests test1;
    if (RunQtTest(&test1) != 0) {
        fInvalid = true;
    }
#ifdef ENABLE_WALLET
    PaymentServerTests test2;
    if (RunQtTest(&test2) != 0) {
        fInvalid = true;
    }
#endif
    RPCNestedTests test3;
    if (RunQtTest(&test3) != 0) {
        fInvalid = true;
    }
    CompatTests test4;
    if (RunQtTest(&test4) != 0) {
        fInvalid = true;
    }
#ifdef ENABLE_WALLET
    WalletTests test5;
    if (RunQtTest(&test5) != 0) {
        fInvalid = true;
    }
    AddressBookTests test6;
    if (RunQtTest(&test6) != 0) {
        fInvalid = true;
    }
#endif

    fs::remove_all(pathTemp);

    return fInvalid;
}
