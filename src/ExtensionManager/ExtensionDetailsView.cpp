#include "ExtensionDetailsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QPixmap>
#include <QPalette>

ExtensionDetailsView::ExtensionDetailsView( SUIT_Desktop* desktop )
  : SUIT_ViewWindow( desktop )
{
  buildUi();
  setWindowTitle( tr( "Extension Details" ) );
}

ExtensionDetailsView::~ExtensionDetailsView()
{
}

void ExtensionDetailsView::buildUi()
{
  QWidget* central = new QWidget( this );
  setCentralWidget( central );

  QVBoxLayout* rootLayout = new QVBoxLayout( central );
  rootLayout->setContentsMargins( 20, 20, 20, 20 );
  rootLayout->setSpacing( 15 );

  // ── Header ──
  QHBoxLayout* headerLayout = new QHBoxLayout();
  m_iconLabel = new QLabel();
  m_iconLabel->setFixedSize( 64, 64 );
  m_iconLabel->setStyleSheet( "background-color: #333; border-radius: 8px;" );
  m_iconLabel->setAlignment( Qt::AlignCenter );
  headerLayout->addWidget( m_iconLabel );

  QVBoxLayout* titleLayout = new QVBoxLayout();
  m_nameLabel = new QLabel( "Extension Name" );
  m_nameLabel->setStyleSheet( "font-size: 24px; font-weight: bold; color: #fff;" );
  titleLayout->addWidget( m_nameLabel );

  m_versionLabel = new QLabel( "v1.0.0" );
  m_versionLabel->setStyleSheet( "font-size: 14px; color: #aaa;" );
  titleLayout->addWidget( m_versionLabel );
  headerLayout->addLayout( titleLayout );
  headerLayout->addStretch();

  m_installButton = new QPushButton( tr( "INSTALL" ) );
  m_installButton->setFixedSize( 100, 32 );
  m_installButton->setCursor( Qt::PointingHandCursor );
  m_installButton->setStyleSheet(
    "QPushButton {"
    "  background-color: #007acc;"
    "  color: white;"
    "  border-radius: 4px;"
    "  font-weight: bold;"
    "}"
    "QPushButton:hover {"
    "  background-color: #005a9e;"
    "}"
    "QPushButton:disabled {"
    "  background-color: #444;"
    "  color: #888;"
    "}"
  );
  connect( m_installButton, &QPushButton::clicked, this, &ExtensionDetailsView::onInstallClicked );
  headerLayout->addWidget( m_installButton );

  rootLayout->addLayout( headerLayout );

  // ── Stats ──
  QHBoxLayout* statsLayout = new QHBoxLayout();
  m_ratingLabel = new QLabel( "⭐ 4.5" );
  m_installsLabel = new QLabel( "⬇ 1.2k installs" );
  m_authorLabel = new QLabel( "by Author" );
  
  QString statsStyle = "font-size: 13px; color: #888; margin-right: 15px;";
  m_ratingLabel->setStyleSheet( statsStyle );
  m_installsLabel->setStyleSheet( statsStyle );
  m_authorLabel->setStyleSheet( statsStyle );

  statsLayout->addWidget( m_ratingLabel );
  statsLayout->addWidget( m_installsLabel );
  statsLayout->addWidget( m_authorLabel );
  statsLayout->addStretch();
  rootLayout->addLayout( statsLayout );

  // ── Separator ──
  QFrame* line = new QFrame();
  line->setFrameShape( QFrame::HLine );
  line->setFrameShadow( QFrame::Sunken );
  line->setStyleSheet( "background-color: #444;" );
  rootLayout->addWidget( line );

  // ── Description ──
  QLabel* descTitle = new QLabel( tr( "Description" ) );
  descTitle->setStyleSheet( "font-size: 18px; font-weight: bold; color: #ddd;" );
  rootLayout->addWidget( descTitle );

  m_descriptionLabel = new QLabel();
  m_descriptionLabel->setWordWrap( true );
  m_descriptionLabel->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  m_descriptionLabel->setStyleSheet( "font-size: 14px; color: #ccc; line-height: 1.4;" );
  
  QScrollArea* scrollArea = new QScrollArea();
  scrollArea->setWidget( m_descriptionLabel );
  scrollArea->setWidgetResizable( true );
  scrollArea->setFrameShape( QFrame::NoFrame );
  scrollArea->setStyleSheet( "background: transparent;" );
  rootLayout->addWidget( scrollArea, 1 );

  setStyleSheet( "background-color: #1e1e1e;" );
}

void ExtensionDetailsView::setExtension( const Extension& ext )
{
  m_extension = ext;
  m_nameLabel->setText( ext.name );
  m_versionLabel->setText( "v" + ext.version );
  m_authorLabel->setText( "by " + ext.author );
  m_descriptionLabel->setText( ext.description );
  m_ratingLabel->setText( QString( "⭐ %1" ).arg( ext.rating, 0, 'f', 1 ) );
  m_installsLabel->setText( QString( "⬇ %1 installs" ).arg( ext.installs ) );
  
  if ( ext.installed ) {
    m_installButton->setText( tr( "INSTALLED" ) );
    m_installButton->setEnabled( false );
  } else {
    m_installButton->setText( tr( "INSTALL" ) );
    m_installButton->setEnabled( true );
  }

  // Load icon if URL is available
  if ( !ext.iconUrl.isEmpty() ) {
    QNetworkAccessManager* manager = new QNetworkAccessManager( this );
    connect( manager, &QNetworkAccessManager::finished, this, [this]( QNetworkReply* reply ) {
      if ( reply->error() == QNetworkReply::NoError ) {
        QPixmap pixmap;
        pixmap.loadFromData( reply->readAll() );
        m_iconLabel->setPixmap( pixmap.scaled( 64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
      } else {
        m_iconLabel->setText( m_extension.name.left( 1 ).toUpper() );
        m_iconLabel->setStyleSheet( "background-color: #007acc; color: white; border-radius: 8px; font-size: 32px; font-weight: bold;" );
      }
      reply->deleteLater();
    } );
    manager->get( QNetworkRequest( QUrl( ext.iconUrl ) ) );
  } else {
    m_iconLabel->setText( ext.name.left( 1 ).toUpper() );
    m_iconLabel->setStyleSheet( "background-color: #007acc; color: white; border-radius: 8px; font-size: 32px; font-weight: bold;" );
  }

  setWindowTitle( ext.name + " - Details" );
}

void ExtensionDetailsView::onInstallClicked()
{
  emit installRequested( m_extension );
}

void ExtensionDetailsView::contextMenuEvent( QContextMenuEvent* event )
{
  QMenu menu( this );
  QAction* closeAction = menu.addAction( tr( "Close" ) );
  connect( closeAction, &QAction::triggered, this, &ExtensionDetailsView::close );
  menu.exec( event->globalPos() );
}

void ExtensionDetailsView::closeEvent( QCloseEvent* event )
{
  SUIT_ViewWindow::closeEvent( event );
  if ( closable() )
    event->accept();
}

