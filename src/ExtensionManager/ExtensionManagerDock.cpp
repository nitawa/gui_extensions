#include "ExtensionManagerDock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollBar>
#include <QScrollArea>

// ─────────────────────────────────────────────────────────────

ExtensionManagerDock::ExtensionManagerDock(const QUrl &serverUrl, QWidget *parent)
    : QWidget(parent)
    , m_serverUrl(serverUrl)
{
    m_model    = new ExtensionModel(this);
    m_installedModel = new ExtensionModel(this);
    m_recommendedModel = new ExtensionModel(this);
    m_delegate = new ExtensionDelegate(this);
    m_fetcher  = new ExtensionFetcher(this);

    // Debounce timer: 350 ms after last keystroke
    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(350);

    buildUi();

    // ── Wire signals ──────────────────────────────────────────
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &ExtensionManagerDock::onSearchTextChanged);

    connect(m_searchTimer, &QTimer::timeout, this, [this]() {
        triggerSearch(m_searchEdit->text().trimmed());
    });

    connect(m_fetcher, &ExtensionFetcher::extensionsFetched,
            this, &ExtensionManagerDock::onExtensionsFetched);
    connect(m_fetcher, &ExtensionFetcher::fetchError,
            this, &ExtensionManagerDock::onFetchError);
    connect(m_fetcher, &ExtensionFetcher::installProgress,
            this, &ExtensionManagerDock::onInstallProgress);
    connect(m_fetcher, &ExtensionFetcher::installFinished,
            this, &ExtensionManagerDock::onInstallFinished);
    connect(m_fetcher, &ExtensionFetcher::uninstallFinished,
            this, &ExtensionManagerDock::onUninstallFinished);

    connect(m_delegate, &ExtensionDelegate::installRequested,
            this, &ExtensionManagerDock::onInstallRequested);
    connect(m_delegate, &ExtensionDelegate::uninstallRequested,
            this, &ExtensionManagerDock::onUninstallRequested);

    connect(m_installedListView, &QListView::clicked,
            this, &ExtensionManagerDock::onItemClicked);
    connect(m_recommendedListView, &QListView::clicked,
            this, &ExtensionManagerDock::onItemClicked);

    // Initial load: fetch all
    triggerSearch("");
}

// ── UI construction ───────────────────────────────────────────

void ExtensionManagerDock::buildUi()
{
    setStyleSheet("background-color: #252526; color: #cccccc;");

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Header ────────────────────────────────────────────────
    QFrame *header = new QFrame;
    header->setStyleSheet("background-color: #333333;");
    header->setFixedHeight(36);
    auto *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(10, 0, 10, 0);
    QLabel *title = new QLabel("EXTENSIONS");
    title->setStyleSheet("color: #bbbbbb; font-size: 11px; font-weight: bold; "
                         "letter-spacing: 1px;");
    hLayout->addWidget(title);
    hLayout->addStretch();
    rootLayout->addWidget(header);

    // ── Search bar ────────────────────────────────────────────
    QFrame *searchFrame = new QFrame;
    searchFrame->setStyleSheet("background-color: #252526;");
    auto *sLayout = new QHBoxLayout(searchFrame);
    sLayout->setContentsMargins(8, 8, 8, 4);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("  🔍  Search extensions...");
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "  background: #3c3c3c;"
        "  color: #cccccc;"
        "  border: 1px solid #555555;"
        "  border-radius: 3px;"
        "  padding: 4px 8px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid #007fd4;"
        "}");
    m_searchEdit->setClearButtonEnabled(true);
    sLayout->addWidget(m_searchEdit);
    rootLayout->addWidget(searchFrame);

    // ── Status label ─────────────────────────────────────────
    m_statusLabel = new QLabel("Fetching extensions...");
    m_statusLabel->setStyleSheet("color: #9d9d9d; font-size: 11px; "
                                 "padding: 2px 12px 4px 12px;");
    rootLayout->addWidget(m_statusLabel);

    auto* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: #252526; border: none;");
    
    auto* scrollWidget = new QWidget;
    auto* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(0);

    // ── Installed Section ─────────────────────────────────────
    m_installedHeader = new QLabel("  INSTALLED");
    m_installedHeader->setStyleSheet("color: #bbbbbb; font-size: 10px; font-weight: bold; background-color: #37373d; padding: 4px 0;");
    scrollLayout->addWidget(m_installedHeader);

    m_installedListView = new QListView;
    m_installedListView->setModel(m_installedModel);
    m_installedListView->setItemDelegate(m_delegate);
    m_installedListView->setMouseTracking(true);
    m_installedListView->setUniformItemSizes(true);
    m_installedListView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_installedListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_installedListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_installedListView->setStyleSheet("QListView { background-color: #252526; border: none; }");
    scrollLayout->addWidget(m_installedListView);

    // ── Recommended Section ───────────────────────────────────
    m_recommendedHeader = new QLabel("  RECOMMENDED");
    m_recommendedHeader->setStyleSheet("color: #bbbbbb; font-size: 10px; font-weight: bold; background-color: #37373d; padding: 4px 0;");
    scrollLayout->addWidget(m_recommendedHeader);

    m_recommendedListView = new QListView;
    m_recommendedListView->setModel(m_recommendedModel);
    m_recommendedListView->setItemDelegate(m_delegate);
    m_recommendedListView->setMouseTracking(true);
    m_recommendedListView->setUniformItemSizes(true);
    m_recommendedListView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_recommendedListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recommendedListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recommendedListView->setStyleSheet("QListView { background-color: #252526; border: none; }");
    scrollLayout->addWidget(m_recommendedListView);
    
    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    rootLayout->addWidget(scrollArea, 1);

    // ── Progress area ─────────────────────────────────────────
    QFrame *progressFrame = new QFrame;
    progressFrame->setStyleSheet("background-color: #1e1e1e;");
    auto *pLayout = new QVBoxLayout(progressFrame);
    pLayout->setContentsMargins(8, 4, 8, 4);
    pLayout->setSpacing(2);

    m_progressLabel = new QLabel;
    m_progressLabel->setStyleSheet("color: #9d9d9d; font-size: 11px;");
    m_progressLabel->hide();

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(4);
    m_progressBar->setStyleSheet(
        "QProgressBar { background: #3c3c3c; border-radius: 2px; }"
        "QProgressBar::chunk { background: #0e639c; border-radius: 2px; }");
    m_progressBar->hide();

    pLayout->addWidget(m_progressLabel);
    pLayout->addWidget(m_progressBar);
    rootLayout->addWidget(progressFrame);
}

// ── Slots ─────────────────────────────────────────────────────

void ExtensionManagerDock::onSearchTextChanged(const QString &)
{
    m_searchTimer->start();   // restart debounce
}

void ExtensionManagerDock::triggerSearch(const QString &keyword)
{
    setStatusMessage("Searching…");
    m_fetcher->search(keyword, m_serverUrl);
}

void ExtensionManagerDock::onExtensionsFetched(const QList<Extension> &extensions)
{
    m_model->setExtensions(extensions);
    updateSplitModels();
    if (extensions.isEmpty())
        setStatusMessage("No extensions found.");
    else
        setStatusMessage(QString("Showing %1 result%2")
                         .arg(extensions.size())
                         .arg(extensions.size() > 1 ? "s" : ""));
}

void ExtensionManagerDock::updateSplitModels()
{
    QList<Extension> installed;
    QList<Extension> recommended;
    
    // In a real app, we might have a list of already installed IDs
    // For this mock, let's assume we use the main model as source of truth
    for (int i = 0; i < m_model->rowCount(); ++i) {
        const Extension& ext = m_model->extensionAt(i);
        if (ext.installed)
            installed << ext;
        else
            recommended << ext;
    }
    
    m_installedModel->setExtensions(installed);
    m_recommendedModel->setExtensions(recommended);
    
    // Adjust height of list views based on content (approximate)
    m_installedListView->setFixedHeight(installed.size() * 60); // 60 is delegate height
    m_recommendedListView->setFixedHeight(recommended.size() * 60);
    
    // Hide/show sections if empty
    m_installedListView->setVisible(!installed.isEmpty());
    m_installedHeader->setVisible(!installed.isEmpty());
    m_recommendedListView->setVisible(!recommended.isEmpty());
    m_recommendedHeader->setVisible(!recommended.isEmpty());
}

void ExtensionManagerDock::onFetchError(const QString &errorMessage)
{
    setStatusMessage("⚠ " + errorMessage, "#f44747");
}

void ExtensionManagerDock::onInstallRequested(const QModelIndex &index)
{
    const ExtensionModel* model = qobject_cast<const ExtensionModel*>(index.model());
    if (model) {
        installExtension(model->extensionAt(index.row()));
    }
}

void ExtensionManagerDock::onUninstallRequested(const QModelIndex &index)
{
    const ExtensionModel* model = qobject_cast<const ExtensionModel*>(index.model());
    if (model) {
        uninstallExtension(model->extensionAt(index.row()));
    }
}

void ExtensionManagerDock::installExtension(const Extension &ext)
{
    m_progressLabel->setText(QString("Installing %1 …").arg(ext.name));
    m_progressLabel->show();
    m_progressBar->setValue(0);
    m_progressBar->show();

    m_fetcher->installExtension(ext, m_serverUrl);
}

void ExtensionManagerDock::uninstallExtension(const Extension &ext)
{
    m_progressLabel->setText(QString("Uninstalling %1 …").arg(ext.name));
    m_progressLabel->show();
    m_progressBar->hide(); // Uninstallation doesn't have progress in this mock

    m_fetcher->uninstallExtension(ext, m_serverUrl);
}

void ExtensionManagerDock::onItemClicked(const QModelIndex &index)
{
    const ExtensionModel* model = qobject_cast<const ExtensionModel*>(index.model());
    if (model) {
        const Extension &ext = model->extensionAt(index.row());
        emit extensionClicked(ext);
    }
}

void ExtensionManagerDock::onInstallProgress(const QString &, int percent)
{
    m_progressBar->setValue(percent);
}

void ExtensionManagerDock::onInstallFinished(const QString &extId, bool success)
{
    m_progressBar->hide();
    if (success) {
        m_model->markInstalled(extId);
        updateSplitModels();
        m_progressLabel->setStyleSheet("color: #4ec9b0; font-size: 11px;");
        m_progressLabel->setText("✓ Installation complete");
    } else {
        m_progressLabel->setStyleSheet("color: #f44747; font-size: 11px;");
        m_progressLabel->setText("✗ Installation failed");
    }

    // Auto-hide message after 3 s
    QTimer::singleShot(3000, this, [this]() {
        m_progressLabel->hide();
        m_progressLabel->setStyleSheet("color: #9d9d9d; font-size: 11px;");
    });
}

void ExtensionManagerDock::onUninstallFinished(const QString &extId, bool success)
{
    if (success) {
        m_model->markUninstalled(extId);
        updateSplitModels();
        m_progressLabel->setStyleSheet("color: #4ec9b0; font-size: 11px;");
        m_progressLabel->setText("✓ Uninstallation complete");
    } else {
        m_progressLabel->setStyleSheet("color: #f44747; font-size: 11px;");
        m_progressLabel->setText("✗ Uninstallation failed");
    }

    // Auto-hide message after 3 s
    QTimer::singleShot(3000, this, [this]() {
        m_progressLabel->hide();
        m_progressLabel->setStyleSheet("color: #9d9d9d; font-size: 11px;");
    });
}

void ExtensionManagerDock::setStatusMessage(const QString &msg, const QString &color)
{
    m_statusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; padding: 2px 12px 4px 12px;").arg(color));
    m_statusLabel->setText(msg);
}
