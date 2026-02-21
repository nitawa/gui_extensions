#include "ExtensionFetcher.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QTimer>

ExtensionFetcher::ExtensionFetcher(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{}

// ── Public API ────────────────────────────────────────────────

void ExtensionFetcher::search(const QString &keyword, const QUrl &serverUrl)
{
    QUrl url = serverUrl;
    // Append the keyword as a query parameter: GET /extensions?q=<keyword>
    QUrlQuery query;
    query.addQueryItem("q", keyword);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished,
            this,  &ExtensionFetcher::onSearchReplyFinished);
}

void ExtensionFetcher::installExtension(const Extension &ext, const QUrl &serverUrl)
{
    // Build URL:  GET /install?id=<ext.id>
    QUrl url = serverUrl;
    url.setPath(url.path().replace("extensions", "install"));
    QUrlQuery query;
    query.addQueryItem("id", ext.id);
    url.setQuery(query);

    m_pendingInstallId = ext.id;

    QNetworkRequest request(url);
    QNetworkReply *reply = m_nam->get(request);

    // Simulate download progress via the reply's downloadProgress signal
    connect(reply, &QNetworkReply::downloadProgress,
            this,  [this, reply](qint64 received, qint64 total) {
        if (total > 0)
            emit installProgress(m_pendingInstallId,
                                 static_cast<int>(received * 100 / total));
    });

    connect(reply, &QNetworkReply::finished,
            this,  &ExtensionFetcher::onInstallReplyFinished);
}

// ── Private slots ─────────────────────────────────────────────

void ExtensionFetcher::onSearchReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit fetchError(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QList<Extension> extensions = parseJson(data);
    emit extensionsFetched(extensions);
}

void ExtensionFetcher::onInstallReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;
    reply->deleteLater();

    bool ok = (reply->error() == QNetworkReply::NoError);
    emit installFinished(m_pendingInstallId, ok);
    if (!ok)
        emit fetchError(tr("Installation failed: ") + reply->errorString());
}

// ── JSON parser ───────────────────────────────────────────────

QList<Extension> ExtensionFetcher::parseJson(const QByteArray &data)
{
    QList<Extension> result;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray())
        return result;

    for (const QJsonValue &val : doc.array()) {
        QJsonObject obj = val.toObject();
        Extension ext;
        ext.id          = obj["id"].toString();
        ext.name        = obj["name"].toString();
        ext.version     = obj["version"].toString();
        ext.author      = obj["author"].toString();
        ext.description = obj["description"].toString();
        ext.rating      = obj["rating"].toDouble();
        ext.installs    = obj["installs"].toInt();
        ext.iconUrl     = obj["iconUrl"].toString();

        for (const QJsonValue &tag : obj["tags"].toArray())
            ext.tags << tag.toString();

        result.append(ext);
    }
    return result;
}
