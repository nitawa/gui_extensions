#ifndef EXTENSIONDETAILSVIEW_H
#define EXTENSIONDETAILSVIEW_H

#include <SUIT_ViewWindow.h>
#include "Extension.h"

class QLabel;
class QPushButton;

class ExtensionDetailsView : public SUIT_ViewWindow
{
  Q_OBJECT

public:
  ExtensionDetailsView( SUIT_Desktop* desktop );
  virtual ~ExtensionDetailsView();

  void setExtension( const Extension& ext );

private:
  void buildUi();

  QLabel* m_iconLabel;
  QLabel* m_nameLabel;
  QLabel* m_versionLabel;
  QLabel* m_authorLabel;
  QLabel* m_descriptionLabel;
  QLabel* m_ratingLabel;
  QLabel* m_installsLabel;
};

#endif // EXTENSIONDETAILSVIEW_H
