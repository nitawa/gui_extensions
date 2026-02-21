# SALOME Extension Manager — Maquette Qt5

Extension manager style **VS Code** intégré dans une fenêtre principale imitation **SALOME Platform**.

---

## Architecture

```
salome_ext_manager/
├── CMakeLists.txt
├── server.py                   ← Serveur HTTP Python (alternative au mock C++)
└── src/
    ├── main.cpp
    ├── Extension.h             ← Structure de données (POD)
    ├── ExtensionFetcher.{h,cpp}← Requêtes HTTP asynchrones (QNetworkAccessManager)
    ├── ExtensionModel.{h,cpp}  ← QAbstractListModel
    ├── ExtensionDelegate.{h,cpp}← Rendu custom style VSCode (QPainter)
    ├── ExtensionManagerDock.{h,cpp} ← QDockWidget principal
    ├── MockHttpServer.{h,cpp}  ← Serveur HTTP embarqué (QTcpServer, sans dépendance externe)
    └── MainWindow.{h,cpp}      ← Fenêtre SALOME (menubar, viewport, statusbar)
```

### Flux de données

```
Utilisateur tape dans QLineEdit
        │  (debounce 350 ms via QTimer)
        ▼
ExtensionManagerDock::triggerSearch()
        │
        ▼
ExtensionFetcher::search(keyword, serverUrl)
  → GET http://127.0.0.1:8765/extensions?q=<keyword>
        │
        ▼  (QNetworkReply::finished)
ExtensionFetcher::parseJson()
  → QList<Extension>
        │
        ▼  (signal extensionsFetched)
ExtensionModel::setExtensions()
  → beginResetModel() / endResetModel()
        │
        ▼
QListView rafraîchit via ExtensionDelegate::paint()
```

### Flux d'installation

```
Clic sur le bouton "Install" dans ExtensionDelegate::editorEvent()
        │  (signal installRequested)
        ▼
ExtensionManagerDock::onInstallRequested()
        │
        ▼
ExtensionFetcher::installExtension()
  → GET http://127.0.0.1:8765/install?id=<extId>
        │  (downloadProgress → signal installProgress)
        ▼
QProgressBar mis à jour
        │  (QNetworkReply::finished → signal installFinished)
        ▼
ExtensionModel::markInstalled()  → badge "✓ Installed"
```

---

## Compilation

### Prérequis

| Outil | Version minimale |
|-------|-----------------|
| CMake | 3.10 |
| Qt    | 5.12 (Core, Widgets, Network) |
| GCC / Clang / MSVC | C++17 |

### Linux / macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./SalomeExtensionManager
```

### Windows (MSVC)

```powershell
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
.\Release\SalomeExtensionManager.exe
```

---

## Serveur HTTP

### Option A — Mock embarqué (défaut)

Le `MockHttpServer` (C++, `QTcpServer`) démarre automatiquement sur le port **8765**
à l'intérieur du même processus. Aucune dépendance externe requise.

### Option B — Serveur Python autonome

```bash
python3 server.py --port 8765
```

Pour pointer le client vers un serveur externe, modifiez dans `MainWindow.cpp` :

```cpp
m_serverUrl = QUrl("http://mon-serveur.example.com:8765/extensions");
```

### Format JSON de l'API

```json
// GET /extensions?q=mesh
[
  {
    "id":          "salome.mesh",
    "name":        "Mesh Module",
    "version":     "9.11.0",
    "author":      "CEA/DEN",
    "description": "Automatic and manual meshing with NETGEN, MG-Tetra and more.",
    "tags":        ["mesh", "netgen", "fea"],
    "rating":      4.7,
    "installs":    118000
  }
]

// GET /install?id=salome.mesh
{ "status": "ok", "id": "salome.mesh" }
```

---

## Fonctionnalités UI

| Fonctionnalité | Détail |
|----------------|--------|
| Recherche debounce | 350 ms après la dernière frappe (QTimer::singleShot) |
| Rendu custom | `ExtensionDelegate` peint icône colorée, nom, auteur, description, étoiles, compteur d'installs |
| Bouton Install | Cliquable directement dans la liste (`editorEvent`) |
| Barre de progression | `QProgressBar` alimentée par `downloadProgress` |
| État installé | Badge vert "✓ Installed" persistant dans le modèle |
| Thème sombre | Palette VS Code / SALOME appliquée via `setStyleSheet` global |
| Dock flottant | `QDockWidget` ancrable à droite, flottable, réouvrable |
| Raccourci clavier | `Ctrl+Shift+X` ouvre le gestionnaire (comme VS Code) |

---

## Extension du projet

### Ajouter un vrai backend

Remplacez `MockHttpServer` par un appel vers votre dépôt SALOME réel ou PyPI-like.
L'interface `ExtensionFetcher` ne change pas.

### Persistance des extensions installées

Ajoutez dans `ExtensionModel::markInstalled()` une écriture dans un fichier JSON
local (`~/.salome/extensions.json`) pour survivre aux redémarrages.

### Téléchargement réel d'un paquet

Dans `ExtensionFetcher::installExtension()`, remplacez le GET simple par un
téléchargement de `.tar.gz` / `.zip`, puis décompressez avec `libarchive` ou
`quazip`.

### Affichage d'un détail

Connectez `QListView::activated` à un panneau de détail (`QStackedWidget` ou
`QDialog`) affichant la description complète, le changelog et les dépendances.
