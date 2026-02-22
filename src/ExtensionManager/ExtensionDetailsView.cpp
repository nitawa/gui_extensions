#include "ExtensionDetailsView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

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
  QLabel* descTitle = new QLabel( "Description" );
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
  m_nameLabel->setText( ext.name );
  m_versionLabel->setText( "v" + ext.version );
  m_authorLabel->setText( "by " + ext.author );
  m_descriptionLabel->setText( ext.description );
  m_ratingLabel->setText( QString( "⭐ %1" ).arg( ext.rating, 0, 'f', 1 ) );
  m_installsLabel->setText( QString( "⬇ %1 installs" ).arg( ext.installs ) );
  
  setWindowTitle( ext.name + " - Details" );
}
