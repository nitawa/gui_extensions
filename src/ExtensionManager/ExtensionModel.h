#pragma once
#include <QAbstractListModel>
#include "Extension.h"

// ─────────────────────────────────────────────────────────────
// ExtensionModel – QAbstractListModel holding a list of
// Extension objects fetched from the remote server.
// ─────────────────────────────────────────────────────────────
class ExtensionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // Custom roles
    enum ExtensionRole {
        IdRole          = Qt::UserRole + 1,
        VersionRole,
        AuthorRole,
        DescriptionRole,
        RatingRole,
        InstallsRole,
        InstalledRole,
        TagsRole,
    };

    explicit ExtensionModel(QObject *parent = nullptr);

    // ── QAbstractListModel overrides ──────────────────────────
    int      rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ── Mutators ──────────────────────────────────────────────
    void setExtensions(const QList<Extension> &extensions);
    void markInstalled(const QString &extId);
    void markUninstalled(const QString &extId);

    const Extension &extensionAt(int row) const;

    bool isInstalled(const QString &extId) const;

private:
    QList<Extension> m_extensions;
};
