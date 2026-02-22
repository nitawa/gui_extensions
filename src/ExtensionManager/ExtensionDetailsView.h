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
  const Extension& extension() const { return m_extension; }

signals:
  void installRequested( const Extension& ext );
  void uninstallRequested( const Extension& ext );

protected:
  virtual void contextMenuEvent( QContextMenuEvent* event ) override;
  virtual void closeEvent( QCloseEvent* event ) override;

private slots:
  void onInstallClicked();
  void onUninstallClicked();

private:
  void buildUi();
  void updateButtons();

  Extension m_extension;

  QLabel* m_iconLabel;
  QLabel* m_nameLabel;
  QLabel* m_versionLabel;
  QLabel* m_authorLabel;
  QLabel* m_descriptionLabel;
  QLabel* m_ratingLabel;
  QLabel* m_installsLabel;
  QPushButton* m_installButton;
  QPushButton* m_uninstallButton;
};

#endif // EXTENSIONDETAILSVIEW_H
