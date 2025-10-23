// Copyright (c) 2011-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/optionsmodel.h>

#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>

#include <init.h>
#include <validation.h>

#include <net.h>
#include <netbase.h>
#include <txdb.h>

#include <miner.h>

#include <qt/intro.h>

#ifdef ENABLE_WALLET
#include <wallet/wallet.h>
#include <wallet/walletdb.h>
#endif

#include <QNetworkProxy>
#include <QSettings>
#include <QStringList>

const char *DEFAULT_GUI_PROXY_HOST = "127.0.0.1";

OptionsModel::OptionsModel(QObject *parent, bool resetSettings)
    : QAbstractListModel(parent) {
  Init(resetSettings);
}

void OptionsModel::addOverriddenOption(const std::string &option) {
  strOverriddenByCommandLine +=
      QString::fromStdString(option) + "=" +
      QString::fromStdString(gArgs.GetArg(option, "")) + " ";
}

void OptionsModel::Init(bool resetSettings) {
  if (resetSettings)
    Reset();

  checkAndMigrate();

  QSettings settings;

  setRestartRequired(false);

  if (!settings.contains("fHideTrayIcon"))
    settings.setValue("fHideTrayIcon", false);
  fHideTrayIcon = settings.value("fHideTrayIcon").toBool();
  Q_EMIT hideTrayIconChanged(fHideTrayIcon);

  if (!settings.contains("fMinimizeToTray"))
    settings.setValue("fMinimizeToTray", false);
  fMinimizeToTray =
      settings.value("fMinimizeToTray").toBool() && !fHideTrayIcon;

  if (!settings.contains("fMinimizeOnClose"))
    settings.setValue("fMinimizeOnClose", false);
  fMinimizeOnClose = settings.value("fMinimizeOnClose").toBool();

  if (!settings.contains("nDisplayUnit"))
    settings.setValue("nDisplayUnit", BitcoinUnits::BTC);
  nDisplayUnit = settings.value("nDisplayUnit").toInt();

  if (!settings.contains("strThirdPartyTxUrls"))
    settings.setValue("strThirdPartyTxUrls", "");
  strThirdPartyTxUrls = settings.value("strThirdPartyTxUrls", "").toString();

  if (!settings.contains("fCoinControlFeatures"))
    settings.setValue("fCoinControlFeatures", false);
  fCoinControlFeatures = settings.value("fCoinControlFeatures", false).toBool();

  if (!settings.contains("nDatabaseCache"))
    settings.setValue("nDatabaseCache", (qint64)nDefaultDbCache);
  if (!gArgs.SoftSetArg(
          "-dbcache",
          settings.value("nDatabaseCache").toString().toStdString()))
    addOverriddenOption("-dbcache");

  if (!settings.contains("nThreadsScriptVerif"))
    settings.setValue("nThreadsScriptVerif", DEFAULT_SCRIPTCHECK_THREADS);
  if (!gArgs.SoftSetArg(
          "-par",
          settings.value("nThreadsScriptVerif").toString().toStdString()))
    addOverriddenOption("-par");

  if (!settings.contains("strDataDir"))
    settings.setValue("strDataDir", Intro::getDefaultDataDirectory());

#ifdef ENABLE_WALLET
  if (!settings.contains("bSpendZeroConfChange"))
    settings.setValue("bSpendZeroConfChange", true);
  if (!gArgs.SoftSetBoolArg("-spendzeroconfchange",
                            settings.value("bSpendZeroConfChange").toBool()))
    addOverriddenOption("-spendzeroconfchange");

  if (!settings.contains("nHiveCheckThreads"))
    settings.setValue("nHiveCheckThreads", (qint64)DEFAULT_HIVE_THREADS);
  if (!gArgs.SoftSetArg(
          "-hivecheckthreads",
          settings.value("nHiveCheckThreads").toString().toStdString()))
    addOverriddenOption("-hivecheckthreads");

  if (!settings.contains("nHiveCheckDelay"))
    settings.setValue("nHiveCheckDelay", (qint64)DEFAULT_HIVE_CHECK_DELAY);
  if (!gArgs.SoftSetArg(
          "-hivecheckdelay",
          settings.value("nHiveCheckDelay").toString().toStdString()))
    addOverriddenOption("-hivecheckdelay");

  if (!settings.contains("fHiveCheckEarlyOut"))
    settings.setValue("fHiveCheckEarlyOut", DEFAULT_HIVE_EARLY_OUT);
  if (!gArgs.SoftSetBoolArg("-hiveearlyout",
                            settings.value("fHiveCheckEarlyOut").toBool()))
    addOverriddenOption("-hiveearlyout");
#endif

  if (!settings.contains("fUseUPnP"))
    settings.setValue("fUseUPnP", DEFAULT_UPNP);
  if (!gArgs.SoftSetBoolArg("-upnp", settings.value("fUseUPnP").toBool()))
    addOverriddenOption("-upnp");

  if (!settings.contains("fListen"))
    settings.setValue("fListen", DEFAULT_LISTEN);
  if (!gArgs.SoftSetBoolArg("-listen", settings.value("fListen").toBool()))
    addOverriddenOption("-listen");

  if (!settings.contains("fUseProxy"))
    settings.setValue("fUseProxy", false);
  if (!settings.contains("addrProxy"))
    settings.setValue(
        "addrProxy",
        QString("%1:%2").arg(DEFAULT_GUI_PROXY_HOST, DEFAULT_GUI_PROXY_PORT));

  if (settings.value("fUseProxy").toBool() &&
      !gArgs.SoftSetArg("-proxy",
                        settings.value("addrProxy").toString().toStdString()))
    addOverriddenOption("-proxy");
  else if (!settings.value("fUseProxy").toBool() &&
           !gArgs.GetArg("-proxy", "").empty())
    addOverriddenOption("-proxy");

  if (!settings.contains("fUseSeparateProxyTor"))
    settings.setValue("fUseSeparateProxyTor", false);
  if (!settings.contains("addrSeparateProxyTor"))
    settings.setValue(
        "addrSeparateProxyTor",
        QString("%1:%2").arg(DEFAULT_GUI_PROXY_HOST, DEFAULT_GUI_PROXY_PORT));

  if (settings.value("fUseSeparateProxyTor").toBool() &&
      !gArgs.SoftSetArg(
          "-onion",
          settings.value("addrSeparateProxyTor").toString().toStdString()))
    addOverriddenOption("-onion");
  else if (!settings.value("fUseSeparateProxyTor").toBool() &&
           !gArgs.GetArg("-onion", "").empty())
    addOverriddenOption("-onion");

  if (!settings.contains("language"))
    settings.setValue("language", "");
  if (!gArgs.SoftSetArg("-lang",
                        settings.value("language").toString().toStdString()))
    addOverriddenOption("-lang");

  language = settings.value("language").toString();
}

static void CopySettings(QSettings &dst, const QSettings &src) {
  for (const QString &key : src.allKeys()) {
    dst.setValue(key, src.value(key));
  }
}

static void BackupSettings(const fs::path &filename, const QSettings &src) {
  qWarning() << "Backing up GUI settings to"
             << GUIUtil::boostPathToQString(filename);
  QSettings dst(GUIUtil::boostPathToQString(filename), QSettings::IniFormat);
  dst.clear();
  CopySettings(dst, src);
}

void OptionsModel::Reset() {
  QSettings settings;

  BackupSettings(GetDataDir(true) / "guisettings.ini.bak", settings);

  QString dataDir = Intro::getDefaultDataDirectory();
  dataDir = settings.value("strDataDir", dataDir).toString();

  settings.clear();

  settings.setValue("strDataDir", dataDir);

  settings.setValue("fReset", true);

  if (GUIUtil::GetStartOnSystemStartup())
    GUIUtil::SetStartOnSystemStartup(false);
}

int OptionsModel::rowCount(const QModelIndex &parent) const {
  return OptionIDRowCount;
}

struct ProxySetting {
  bool is_set;
  QString ip;
  QString port;
};

static ProxySetting GetProxySetting(QSettings &settings, const QString &name) {
  static const ProxySetting default_val = {
      false, DEFAULT_GUI_PROXY_HOST, QString("%1").arg(DEFAULT_GUI_PROXY_PORT)};

  if (!settings.contains(name)) {
    return default_val;
  }

  QStringList ip_port =
      settings.value(name).toString().split(":", QString::SkipEmptyParts);
  if (ip_port.size() == 2) {
    return {true, ip_port.at(0), ip_port.at(1)};
  } else {
    return default_val;
  }
}

static void SetProxySetting(QSettings &settings, const QString &name,
                            const ProxySetting &ip_port) {
  settings.setValue(name, ip_port.ip + ":" + ip_port.port);
}

QVariant OptionsModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::EditRole) {
    QSettings settings;
    switch (index.row()) {
    case StartAtStartup:
      return GUIUtil::GetStartOnSystemStartup();
    case HideTrayIcon:
      return fHideTrayIcon;
    case MinimizeToTray:
      return fMinimizeToTray;
    case MapPortUPnP:
#ifdef USE_UPNP
      return settings.value("fUseUPnP");
#else
      return false;
#endif
    case MinimizeOnClose:
      return fMinimizeOnClose;

    case ProxyUse:
      return settings.value("fUseProxy", false);
    case ProxyIP:
      return GetProxySetting(settings, "addrProxy").ip;
    case ProxyPort:
      return GetProxySetting(settings, "addrProxy").port;

    case ProxyUseTor:
      return settings.value("fUseSeparateProxyTor", false);
    case ProxyIPTor:
      return GetProxySetting(settings, "addrSeparateProxyTor").ip;
    case ProxyPortTor:
      return GetProxySetting(settings, "addrSeparateProxyTor").port;

#ifdef ENABLE_WALLET
    case SpendZeroConfChange:
      return settings.value("bSpendZeroConfChange");

    case HiveCheckThreads:
      return settings.value("nHiveCheckThreads");
    case HiveCheckDelay:
      return settings.value("nHiveCheckDelay");
    case HiveCheckEarlyOut:
      return settings.value("fHiveCheckEarlyOut");
#endif
    case DisplayUnit:
      return nDisplayUnit;
    case ThirdPartyTxUrls:
      return strThirdPartyTxUrls;
    case Language:
      return settings.value("language");
    case CoinControlFeatures:
      return fCoinControlFeatures;
    case DatabaseCache:
      return settings.value("nDatabaseCache");
    case ThreadsScriptVerif:
      return settings.value("nThreadsScriptVerif");
    case Listen:
      return settings.value("fListen");
    default:
      return QVariant();
    }
  }
  return QVariant();
}

bool OptionsModel::setData(const QModelIndex &index, const QVariant &value,
                           int role) {
  bool successful = true;

  if (role == Qt::EditRole) {
    QSettings settings;
    switch (index.row()) {
    case StartAtStartup:
      successful = GUIUtil::SetStartOnSystemStartup(value.toBool());
      break;
    case HideTrayIcon:
      fHideTrayIcon = value.toBool();
      settings.setValue("fHideTrayIcon", fHideTrayIcon);
      Q_EMIT hideTrayIconChanged(fHideTrayIcon);
      break;
    case MinimizeToTray:
      fMinimizeToTray = value.toBool();
      settings.setValue("fMinimizeToTray", fMinimizeToTray);
      break;
    case MapPortUPnP:

      settings.setValue("fUseUPnP", value.toBool());
      MapPort(value.toBool());
      break;
    case MinimizeOnClose:
      fMinimizeOnClose = value.toBool();
      settings.setValue("fMinimizeOnClose", fMinimizeOnClose);
      break;

    case ProxyUse:
      if (settings.value("fUseProxy") != value) {
        settings.setValue("fUseProxy", value.toBool());
        setRestartRequired(true);
      }
      break;
    case ProxyIP: {
      auto ip_port = GetProxySetting(settings, "addrProxy");
      if (!ip_port.is_set || ip_port.ip != value.toString()) {
        ip_port.ip = value.toString();
        SetProxySetting(settings, "addrProxy", ip_port);
        setRestartRequired(true);
      }
    } break;
    case ProxyPort: {
      auto ip_port = GetProxySetting(settings, "addrProxy");
      if (!ip_port.is_set || ip_port.port != value.toString()) {
        ip_port.port = value.toString();
        SetProxySetting(settings, "addrProxy", ip_port);
        setRestartRequired(true);
      }
    } break;

    case ProxyUseTor:
      if (settings.value("fUseSeparateProxyTor") != value) {
        settings.setValue("fUseSeparateProxyTor", value.toBool());
        setRestartRequired(true);
      }
      break;
    case ProxyIPTor: {
      auto ip_port = GetProxySetting(settings, "addrSeparateProxyTor");
      if (!ip_port.is_set || ip_port.ip != value.toString()) {
        ip_port.ip = value.toString();
        SetProxySetting(settings, "addrSeparateProxyTor", ip_port);
        setRestartRequired(true);
      }
    } break;
    case ProxyPortTor: {
      auto ip_port = GetProxySetting(settings, "addrSeparateProxyTor");
      if (!ip_port.is_set || ip_port.port != value.toString()) {
        ip_port.port = value.toString();
        SetProxySetting(settings, "addrSeparateProxyTor", ip_port);
        setRestartRequired(true);
      }
    } break;

#ifdef ENABLE_WALLET
    case SpendZeroConfChange:
      if (settings.value("bSpendZeroConfChange") != value) {
        settings.setValue("bSpendZeroConfChange", value);
        setRestartRequired(true);
      }
      break;

    case HiveCheckDelay:
      if (settings.value("nHiveCheckDelay") != value) {
        settings.setValue("nHiveCheckDelay", value);
        gArgs.ForceSetArg(
            "-hivecheckdelay",
            settings.value("nHiveCheckDelay").toString().toStdString());
      }

      break;

    case HiveCheckThreads:
      if (settings.value("nHiveCheckThreads") != value) {
        settings.setValue("nHiveCheckThreads", value);
        gArgs.ForceSetArg(
            "-hivecheckthreads",
            settings.value("nHiveCheckThreads").toString().toStdString());
      }
      break;

    case HiveCheckEarlyOut:
      if (settings.value("fHiveCheckEarlyOut") != value) {
        settings.setValue("fHiveCheckEarlyOut", value.toBool());
        gArgs.ForceSetArg("-hiveearlyout",
                          settings.value("fHiveCheckEarlyOut").toBool() ? "1"
                                                                        : "0");
      }
      break;
#endif
    case DisplayUnit:
      setDisplayUnit(value);
      break;
    case ThirdPartyTxUrls:
      if (strThirdPartyTxUrls != value.toString()) {
        strThirdPartyTxUrls = value.toString();
        settings.setValue("strThirdPartyTxUrls", strThirdPartyTxUrls);
        setRestartRequired(true);
      }
      break;
    case Language:
      if (settings.value("language") != value) {
        settings.setValue("language", value);
        setRestartRequired(true);
      }
      break;
    case CoinControlFeatures:
      fCoinControlFeatures = value.toBool();
      settings.setValue("fCoinControlFeatures", fCoinControlFeatures);
      Q_EMIT coinControlFeaturesChanged(fCoinControlFeatures);
      break;
    case DatabaseCache:
      if (settings.value("nDatabaseCache") != value) {
        settings.setValue("nDatabaseCache", value);
        setRestartRequired(true);
      }
      break;
    case ThreadsScriptVerif:
      if (settings.value("nThreadsScriptVerif") != value) {
        settings.setValue("nThreadsScriptVerif", value);
        setRestartRequired(true);
      }
      break;
    case Listen:
      if (settings.value("fListen") != value) {
        settings.setValue("fListen", value);
        setRestartRequired(true);
      }
      break;
    default:
      break;
    }
  }

  Q_EMIT dataChanged(index, index);

  return successful;
}

void OptionsModel::setDisplayUnit(const QVariant &value) {
  if (!value.isNull()) {
    QSettings settings;
    nDisplayUnit = value.toInt();
    settings.setValue("nDisplayUnit", nDisplayUnit);
    Q_EMIT displayUnitChanged(nDisplayUnit);
  }
}

bool OptionsModel::getProxySettings(QNetworkProxy &proxy) const {
  proxyType curProxy;
  if (GetProxy(NET_IPV4, curProxy)) {
    proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName(QString::fromStdString(curProxy.proxy.ToStringIP()));
    proxy.setPort(curProxy.proxy.GetPort());

    return true;
  } else
    proxy.setType(QNetworkProxy::NoProxy);

  return false;
}

void OptionsModel::setRestartRequired(bool fRequired) {
  QSettings settings;
  return settings.setValue("fRestartRequired", fRequired);
}

bool OptionsModel::isRestartRequired() const {
  QSettings settings;
  return settings.value("fRestartRequired", false).toBool();
}

void OptionsModel::checkAndMigrate() {
  QSettings settings;
  static const char strSettingsVersionKey[] = "nSettingsVersion";
  int settingsVersion = settings.contains(strSettingsVersionKey)
                            ? settings.value(strSettingsVersionKey).toInt()
                            : 0;
  if (settingsVersion < CLIENT_VERSION) {
    if (settingsVersion < 130000 && settings.contains("nDatabaseCache") &&
        settings.value("nDatabaseCache").toLongLong() == 100)
      settings.setValue("nDatabaseCache", (qint64)nDefaultDbCache);

    settings.setValue(strSettingsVersionKey, CLIENT_VERSION);
  }
}
