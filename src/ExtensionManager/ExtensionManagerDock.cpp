#include "ExtensionManagerDock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollBar>

// ─────────────────────────────────────────────────────────────

ExtensionManagerDock::ExtensionManagerDock(const QUrl &serverUrl, QWidget *parent)
    : QWidget(parent)
    , m_serverUrl(serverUrl)
{
    m_model    = new ExtensionModel(this);
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

    connect(m_delegate, &ExtensionDelegate::installRequested,
            this, &ExtensionManagerDock::onInstallRequested);

    connect(m_listView, &QListView::clicked,
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

    // ── List view ─────────────────────────────────────────────
    m_listView = new QListView;
    m_listView->setModel(m_model);
    m_listView->setItemDelegate(m_delegate);
    m_listView->setMouseTracking(true);
    m_listView->setUniformItemSizes(true);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setStyleSheet(
        "QListView {"
        "  background-color: #252526;"
        "  border: none;"
        "}"
        "QScrollBar:vertical {"
        "  background: #252526;"
        "  width: 10px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #555555;"
        "  border-radius: 5px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }");
    rootLayout->addWidget(m_listView, 1);

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
    if (extensions.isEmpty())
        setStatusMessage("No extensions found.");
    else
        setStatusMessage(QString("Showing %1 result%2")
                         .arg(extensions.size())
                         .arg(extensions.size() > 1 ? "s" : ""));
}

void ExtensionManagerDock::onFetchError(const QString &errorMessage)
{
    setStatusMessage("⚠ " + errorMessage, "#f44747");
}

void ExtensionManagerDock::onInstallRequested(const QModelIndex &index)
{
    const Extension &ext = m_model->extensionAt(index.row());
    installExtension(ext);
}

void ExtensionManagerDock::installExtension(const Extension &ext)
{
    m_progressLabel->setText(QString("Installing %1 …").arg(ext.name));
    m_progressLabel->show();
    m_progressBar->setValue(0);
    m_progressBar->show();

    m_fetcher->installExtension(ext, m_serverUrl);
}

void ExtensionManagerDock::onItemClicked(const QModelIndex &index)
{
    const Extension &ext = m_model->extensionAt(index.row());
    emit extensionClicked(ext);
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

void ExtensionManagerDock::setStatusMessage(const QString &msg, const QString &color)
{
    m_statusLabel->setStyleSheet(
        QString("color: %1; font-size: 11px; padding: 2px 12px 4px 12px;").arg(color));
    m_statusLabel->setText(msg);
}
