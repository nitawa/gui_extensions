#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

// ─────────────────────────────────────────────────────────────
// MockHttpServer
//   Minimal HTTP/1.0 server that serves a hard-coded catalogue
//   of SALOME-like extensions so the demo works without an
//   external network.
//
//   Endpoints:
//     GET /extensions?q=<keyword>  → JSON array (filtered)
//     GET /install?id=<extId>      → 200 OK (simulated install)
// ─────────────────────────────────────────────────────────────
class MockHttpServer : public QObject
{
    Q_OBJECT

public:
    explicit MockHttpServer(QObject *parent = nullptr);

    /// Start listening.  Returns the chosen port, or -1 on failure.
    int start(quint16 preferredPort = 8765);

    quint16 port() const;

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    void handleRequest(QTcpSocket *socket, const QByteArray &request);
    void sendJson(QTcpSocket *socket, const QByteArray &json);
    void send404(QTcpSocket *socket);

    QByteArray buildCatalogueJson(const QString &keyword) const;

    QTcpServer *m_server;
};
