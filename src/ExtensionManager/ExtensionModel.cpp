#include "ExtensionModel.h"

ExtensionModel::ExtensionModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int ExtensionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_extensions.size();
}

QVariant ExtensionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_extensions.size())
        return {};

    const Extension &ext = m_extensions.at(index.row());

    switch (role) {
    case Qt::DisplayRole:       return ext.name;
    case IdRole:                return ext.id;
    case VersionRole:           return ext.version;
    case AuthorRole:            return ext.author;
    case DescriptionRole:       return ext.description;
    case RatingRole:            return ext.rating;
    case InstallsRole:          return ext.installs;
    case InstalledRole:         return ext.installed;
    case TagsRole:              return ext.tags;
    default:                    return {};
    }
}

QHash<int, QByteArray> ExtensionModel::roleNames() const
{
    return {
        { Qt::DisplayRole, "name"        },
        { IdRole,          "id"          },
        { VersionRole,     "version"     },
        { AuthorRole,      "author"      },
        { DescriptionRole, "description" },
        { RatingRole,      "rating"      },
        { InstallsRole,    "installs"    },
        { InstalledRole,   "installed"   },
        { TagsRole,        "tags"        },
    };
}

void ExtensionModel::setExtensions(const QList<Extension> &extensions)
{
    beginResetModel();
    m_extensions = extensions;
    endResetModel();
}

void ExtensionModel::markInstalled(const QString &extId)
{
    for (int i = 0; i < m_extensions.size(); ++i) {
        if (m_extensions[i].id == extId) {
            m_extensions[i].installed = true;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {InstalledRole});
            break;
        }
    }
}

const Extension &ExtensionModel::extensionAt(int row) const
{
    return m_extensions.at(row);
}
