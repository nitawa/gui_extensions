#pragma once
#include <QString>
#include <QStringList>

// ─────────────────────────────────────────────────────────────
// Extension  –  plain data structure
// ─────────────────────────────────────────────────────────────
struct Extension
{
    QString     id;           // unique identifier  e.g. "salome.geometry"
    QString     name;         // display name
    QString     version;      // semver string
    QString     author;       // author / organisation
    QString     description;  // short description
    QStringList tags;         // search tags
    QString     iconUrl;      // optional icon URL
    double      rating  = 0.0;
    int         installs = 0;
    bool        installed = false;
};
