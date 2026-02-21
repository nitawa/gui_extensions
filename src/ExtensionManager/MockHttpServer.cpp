#include "MockHttpServer.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QUrl>
#include <QDebug>

// ── Static catalogue ─────────────────────────────────────────

static const QList<QJsonObject> s_catalogue = [] {
    auto make = [](const char *id, const char *name, const char *version,
                   const char *author, const char *desc,
                   QStringList tags, double rating, int installs) {
        QJsonObject o;
        o["id"]          = id;
        o["name"]        = name;
        o["version"]     = version;
        o["author"]      = author;
        o["description"] = desc;
        o["rating"]      = rating;
        o["installs"]    = installs;
        QJsonArray t;
        for (const auto &tag : tags) t.append(tag);
        o["tags"]        = t;
        return o;
    };

    return QList<QJsonObject>{
        make("salome.geometry", "Geometry Module",
             "9.11.0", "CEA/DEN",
             "Advanced CAD geometry creation and edition (BREP, STEP, IGES).",
             {"geometry","cad","brep","step"}, 4.8, 125000),

        make("salome.mesh", "Mesh Module",
             "9.11.0", "CEA/DEN",
             "Automatic and manual meshing with NETGEN, MG-Tetra and more.",
             {"mesh","netgen","fea","hexa"}, 4.7, 118000),

        make("salome.paravis", "ParaVis — ParaView Integration",
             "9.11.0", "CEA/DEN",
             "Embeds ParaView post-processing directly in SALOME.",
             {"visualization","post-processing","paraview","vtu"}, 4.9, 99000),

        make("salome.smesh_algo", "Advanced Meshing Algorithms",
             "2.4.1", "EDF R&D",
             "Extra meshing algorithms: quadrangles, prisms, hexahedra.",
             {"mesh","algorithm","hex","prism"}, 4.5, 45000),

        make("salome.eficas", "Eficas — Command File Editor",
             "7.8.0", "EDF R&D",
             "Graphical editor for Code_Aster and Code_Saturne command files.",
             {"editor","aster","saturne","fea"}, 4.3, 33000),

        make("salome.jobmanager", "Job Manager",
             "9.11.0", "CEA/DEN",
             "Submit and monitor HPC jobs directly from SALOME.",
             {"hpc","slurm","job","cluster"}, 4.1, 28000),

        make("salome.yacs", "YACS — Workflow Engine",
             "9.11.0", "CEA/DEN",
             "Graphical workflow and coupling engine for multi-physics simulations.",
             {"workflow","coupling","python","dataflow"}, 4.4, 52000),

        make("salome.hexablock", "HexaBlock — Structured Meshing",
             "9.6.0", "CEA/DEN",
             "Block-structured hexahedral mesh generation.",
             {"mesh","hexa","structured","block"}, 4.2, 17000),

        make("community.gmsh_plugin", "GMSH Plugin",
             "1.3.0", "Community",
             "Integrate the GMSH mesher as an alternative mesh engine.",
             {"mesh","gmsh","community","triangle"}, 4.6, 41000),

        make("community.opencascade_viewer", "OCC Enhanced Viewer",
             "0.9.5", "Community",
             "Extended OpenCASCADE 3-D viewer with advanced display options.",
             {"viewer","opencascade","3d","display"}, 3.9, 9800),

        make("salome.shaper", "Shaper — Parametric CAD",
             "9.11.0", "CEA/DEN",
             "Parametric, feature-based CAD modeller integrated in SALOME.",
             {"geometry","parametric","cad","feature"}, 4.7, 71000),

        make("salome.fields", "Fields Module",
             "9.11.0", "CEA/DEN",
             "Management and manipulation of simulation result fields.",
             {"post-processing","fields","medfile","results"}, 4.5, 38000),

        make("community.code_aster_wizard", "Code_Aster Setup Wizard",
             "2.0.3", "Hamid Bahai / Community",
             "Step-by-step wizard to configure Code_Aster FEA studies in SALOME.",
             {"aster","fea","wizard","setup"}, 4.0, 14200),

        make("community.python_console_plus", "Python Console+",
             "1.1.0", "Community",
             "Enhanced Python console with auto-completion and history search.",
             {"python","console","scripting","ide"}, 4.4, 22000),

        make("salome.documentation", "Documentation Browser",
             "9.11.0", "CEA/DEN",
             "Integrated offline documentation browser for all SALOME modules.",
             {"documentation","help","browser"}, 4.0, 55000),
    };
}();

// ── MockHttpServer ────────────────────────────────────────────

MockHttpServer::MockHttpServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this,     &MockHttpServer::onNewConnection);
}

int MockHttpServer::start(quint16 preferredPort)
{
    if (!m_server->listen(QHostAddress::LocalHost, preferredPort)) {
        // try any port
        if (!m_server->listen(QHostAddress::LocalHost, 0))
            return -1;
    }
    qDebug() << "[MockServer] Listening on port" << m_server->serverPort();
    return m_server->serverPort();
}

quint16 MockHttpServer::port() const
{
    return m_server->serverPort();
}

void MockHttpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead,
                this,   &MockHttpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected,
                socket, &QTcpSocket::deleteLater);
    }
}

void MockHttpServer::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    QByteArray request = socket->readAll();
    handleRequest(socket, request);
    socket->flush();
    socket->disconnectFromHost();
}

void MockHttpServer::handleRequest(QTcpSocket *socket, const QByteArray &request)
{
    // Parse first line: "GET /path?query HTTP/1.x"
    QByteArray firstLine = request.left(request.indexOf('\r'));
    QList<QByteArray> parts = firstLine.split(' ');
    if (parts.size() < 2) { send404(socket); return; }

    // Use QUrl to parse path + query
    QUrl url("http://localhost" + QString::fromUtf8(parts[1]));
    QString path = url.path();
    QUrlQuery query(url.query());

    if (path == "/extensions") {
        QString keyword = query.queryItemValue("q");
        QByteArray json = buildCatalogueJson(keyword);
        sendJson(socket, json);
    } else if (path == "/install") {
        // Just return 200 OK for any install request
        QByteArray body = R"({"status":"ok"})";
        sendJson(socket, body);
    } else {
        send404(socket);
    }
}

QByteArray MockHttpServer::buildCatalogueJson(const QString &keyword) const
{
    QJsonArray array;
    QString kw = keyword.toLower().trimmed();

    for (const QJsonObject &ext : s_catalogue) {
        if (kw.isEmpty()) {
            array.append(ext);
            continue;
        }
        // Match against name, description, tags
        bool match = ext["name"].toString().toLower().contains(kw)
                  || ext["description"].toString().toLower().contains(kw)
                  || ext["author"].toString().toLower().contains(kw);
        if (!match) {
            for (const QJsonValue &tag : ext["tags"].toArray()) {
                if (tag.toString().contains(kw)) { match = true; break; }
            }
        }
        if (match) array.append(ext);
    }
    return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

void MockHttpServer::sendJson(QTcpSocket *socket, const QByteArray &json)
{
    QByteArray response;
    response += "HTTP/1.0 200 OK\r\n";
    response += "Content-Type: application/json\r\n";
    response += "Content-Length: " + QByteArray::number(json.size()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += json;
    socket->write(response);
}

void MockHttpServer::send404(QTcpSocket *socket)
{
    QByteArray body = "Not Found";
    QByteArray response;
    response += "HTTP/1.0 404 Not Found\r\n";
    response += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += body;
    socket->write(response);
}
