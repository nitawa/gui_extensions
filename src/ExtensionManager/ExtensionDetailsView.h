#ifndef EXTENSIONDETAILSVIEW_H
#define EXTENSIONDETAILSVIEW_H

#include <SUIT_ViewWindow.h>
#include "Extension.h"

class QLabel;
class QPushButton;
class QContextMenuEvent;
class QCloseEvent;

class ExtensionDetailsView : public SUIT_ViewWindow
{
  Q_OBJECT

public:
  ExtensionDetailsView( SUIT_Desktop* desktop );
  virtual ~ExtensionDetailsView();

  void setExtension( const Extension& ext );

signals:
  void installRequested( const Extension& ext );

protected:
  virtual void contextMenuEvent( QContextMenuEvent* event ) override;
  virtual void closeEvent( QCloseEvent* event ) override;

private slots:
  void onInstallClicked();

private:
  void buildUi();

  Extension m_extension;

  QLabel* m_iconLabel;
  QLabel* m_nameLabel;
  QLabel* m_versionLabel;
  QLabel* m_authorLabel;
  QLabel* m_descriptionLabel;
  QLabel* m_ratingLabel;
  QLabel* m_installsLabel;
  QPushButton* m_installButton;
};

#endif // EXTENSIONDETAILSVIEW_H
