#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include "Extension.h"

// ─────────────────────────────────────────────────────────────
// ExtensionFetcher
//   Queries the remote catalogue server and parses the JSON
//   response into a list of Extension objects.
//
//   Expected JSON format from server:
//   [
//     {
//       "id"          : "salome.geometry",
//       "name"        : "Geometry Module",
//       "version"     : "9.11.0",
//       "author"      : "CEA/DEN",
//       "description" : "CAD geometry creation and edition",
//       "tags"        : ["geometry","cad","brep"],
//       "rating"      : 4.8,
//       "installs"    : 12500
//     },
//     ...
//   ]
// ─────────────────────────────────────────────────────────────
class ExtensionFetcher : public QObject
{
    Q_OBJECT

public:
    explicit ExtensionFetcher(QObject *parent = nullptr);

    /// Trigger an asynchronous search.  Results arrive via extensionsFetched().
    void search(const QString &keyword, const QUrl &serverUrl);

    /// Download / install an extension (placeholder – real impl would fetch a
    /// package archive and unpack it).
    void installExtension(const Extension &ext, const QUrl &serverUrl);

signals:
    void extensionsFetched(const QList<Extension> &extensions);
    void fetchError(const QString &errorMessage);
    void installProgress(const QString &extId, int percent);
    void installFinished(const QString &extId, bool success);

private slots:
    void onSearchReplyFinished();
    void onInstallReplyFinished();

private:
    QList<Extension> parseJson(const QByteArray &data);

    QNetworkAccessManager *m_nam;
    QString                m_pendingInstallId;
};
