#pragma once
#include <QDockWidget>
#include <QLineEdit>
#include <QListView>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QUrl>
#include <QTimer>

#include "ExtensionModel.h"
#include "ExtensionDelegate.h"
#include "ExtensionFetcher.h"

// ─────────────────────────────────────────────────────────────
// ExtensionManagerDock
//   QDockWidget that provides the full extension manager UI:
//   ┌──────────────────────────────────────┐
//   │  EXTENSIONS  [⚙]                     │
//   │  ┌────────────────────────────────┐  │
//   │  │ 🔍  Search extensions...       │  │
//   │  └────────────────────────────────┘  │
//   │  Showing 15 results                   │
//   │  ┌────────────────────────────────┐  │
//   │  │ <ExtensionDelegate rows>       │  │
//   │  │ ...                            │  │
//   │  └────────────────────────────────┘  │
//   │  [████████████░░░░] Installing...    │
//   └──────────────────────────────────────┘
// ─────────────────────────────────────────────────────────────
class ExtensionManagerDock : public QWidget
{
    Q_OBJECT

public:
    explicit ExtensionManagerDock(const QUrl &serverUrl,
                                  QWidget *parent = nullptr);

public slots:
    void installExtension(const Extension &ext);
    void uninstallExtension(const Extension &ext);

signals:
    void extensionClicked(const Extension &ext);

private slots:
    void onSearchTextChanged(const QString &text);
    void onExtensionsFetched(const QList<Extension> &extensions);
    void onFetchError(const QString &errorMessage);
    void onInstallRequested(const QModelIndex &index);
    void onUninstallRequested(const QModelIndex &index);
    void onItemClicked(const QModelIndex &index);
    void onInstallProgress(const QString &extId, int percent);
    void onInstallFinished(const QString &extId, bool success);
    void onUninstallFinished(const QString &extId, bool success);

private:
    void buildUi();
    void triggerSearch(const QString &keyword);
    void setStatusMessage(const QString &msg, const QString &color = "#9d9d9d");
    void updateSplitModels();

    QUrl               m_serverUrl;

    // ── Widgets ───────────────────────────────────────────────
    QLineEdit         *m_searchEdit   = nullptr;
    QLabel            *m_statusLabel  = nullptr;
    QListView         *m_listView     = nullptr; // Original one, we might keep it or replace it
    QListView         *m_installedListView = nullptr;
    QListView         *m_recommendedListView = nullptr;
    QLabel            *m_installedHeader = nullptr;
    QLabel            *m_recommendedHeader = nullptr;
    QProgressBar      *m_progressBar  = nullptr;
    QLabel            *m_progressLabel= nullptr;

    // ── Business logic ────────────────────────────────────────
    ExtensionModel    *m_model        = nullptr;
    ExtensionModel    *m_installedModel = nullptr;
    ExtensionModel    *m_recommendedModel = nullptr;
    ExtensionDelegate *m_delegate     = nullptr;
    ExtensionFetcher  *m_fetcher      = nullptr;

    QTimer            *m_searchTimer  = nullptr;   // debounce
};
