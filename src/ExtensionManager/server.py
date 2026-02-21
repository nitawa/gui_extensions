#!/usr/bin/env python3
"""
server.py – Standalone HTTP catalogue server for the SALOME Extension Manager.

Usage:
    python3 server.py [--port 8765]

API:
    GET /extensions?q=<keyword>   → JSON array of matching extensions
    GET /install?id=<ext_id>      → 200 OK  (simulated install)

The C++ client (ExtensionFetcher) is compatible with this server out-of-the-box.
You can replace the CATALOGUE list with a real database backend if needed.
"""

import json
import argparse
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

# ── Extension catalogue ───────────────────────────────────────────────────────

CATALOGUE = [
    {
        "id": "salome.geometry",
        "name": "Geometry Module",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Advanced CAD geometry creation and edition (BREP, STEP, IGES).",
        "tags": ["geometry", "cad", "brep", "step"],
        "rating": 4.8,
        "installs": 125000,
    },
    {
        "id": "salome.mesh",
        "name": "Mesh Module",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Automatic and manual meshing with NETGEN, MG-Tetra and more.",
        "tags": ["mesh", "netgen", "fea", "hexa"],
        "rating": 4.7,
        "installs": 118000,
    },
    {
        "id": "salome.paravis",
        "name": "ParaVis — ParaView Integration",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Embeds ParaView post-processing directly in SALOME.",
        "tags": ["visualization", "post-processing", "paraview", "vtu"],
        "rating": 4.9,
        "installs": 99000,
    },
    {
        "id": "salome.smesh_algo",
        "name": "Advanced Meshing Algorithms",
        "version": "2.4.1",
        "author": "EDF R&D",
        "description": "Extra meshing algorithms: quadrangles, prisms, hexahedra.",
        "tags": ["mesh", "algorithm", "hex", "prism"],
        "rating": 4.5,
        "installs": 45000,
    },
    {
        "id": "salome.eficas",
        "name": "Eficas — Command File Editor",
        "version": "7.8.0",
        "author": "EDF R&D",
        "description": "Graphical editor for Code_Aster and Code_Saturne command files.",
        "tags": ["editor", "aster", "saturne", "fea"],
        "rating": 4.3,
        "installs": 33000,
    },
    {
        "id": "salome.jobmanager",
        "name": "Job Manager",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Submit and monitor HPC jobs directly from SALOME.",
        "tags": ["hpc", "slurm", "job", "cluster"],
        "rating": 4.1,
        "installs": 28000,
    },
    {
        "id": "salome.yacs",
        "name": "YACS — Workflow Engine",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Graphical workflow and coupling engine for multi-physics simulations.",
        "tags": ["workflow", "coupling", "python", "dataflow"],
        "rating": 4.4,
        "installs": 52000,
    },
    {
        "id": "salome.hexablock",
        "name": "HexaBlock — Structured Meshing",
        "version": "9.6.0",
        "author": "CEA/DEN",
        "description": "Block-structured hexahedral mesh generation.",
        "tags": ["mesh", "hexa", "structured", "block"],
        "rating": 4.2,
        "installs": 17000,
    },
    {
        "id": "community.gmsh_plugin",
        "name": "GMSH Plugin",
        "version": "1.3.0",
        "author": "Community",
        "description": "Integrate the GMSH mesher as an alternative mesh engine.",
        "tags": ["mesh", "gmsh", "community", "triangle"],
        "rating": 4.6,
        "installs": 41000,
    },
    {
        "id": "community.opencascade_viewer",
        "name": "OCC Enhanced Viewer",
        "version": "0.9.5",
        "author": "Community",
        "description": "Extended OpenCASCADE 3-D viewer with advanced display options.",
        "tags": ["viewer", "opencascade", "3d", "display"],
        "rating": 3.9,
        "installs": 9800,
    },
    {
        "id": "salome.shaper",
        "name": "Shaper — Parametric CAD",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Parametric, feature-based CAD modeller integrated in SALOME.",
        "tags": ["geometry", "parametric", "cad", "feature"],
        "rating": 4.7,
        "installs": 71000,
    },
    {
        "id": "salome.fields",
        "name": "Fields Module",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Management and manipulation of simulation result fields.",
        "tags": ["post-processing", "fields", "medfile", "results"],
        "rating": 4.5,
        "installs": 38000,
    },
    {
        "id": "community.code_aster_wizard",
        "name": "Code_Aster Setup Wizard",
        "version": "2.0.3",
        "author": "Hamid Bahai / Community",
        "description": "Step-by-step wizard to configure Code_Aster FEA studies in SALOME.",
        "tags": ["aster", "fea", "wizard", "setup"],
        "rating": 4.0,
        "installs": 14200,
    },
    {
        "id": "community.python_console_plus",
        "name": "Python Console+",
        "version": "1.1.0",
        "author": "Community",
        "description": "Enhanced Python console with auto-completion and history search.",
        "tags": ["python", "console", "scripting", "ide"],
        "rating": 4.4,
        "installs": 22000,
    },
    {
        "id": "salome.documentation",
        "name": "Documentation Browser",
        "version": "9.11.0",
        "author": "CEA/DEN",
        "description": "Integrated offline documentation browser for all SALOME modules.",
        "tags": ["documentation", "help", "browser"],
        "rating": 4.0,
        "installs": 55000,
    },
]


# ── Search logic ──────────────────────────────────────────────────────────────

def search_extensions(keyword: str) -> list:
    """Return extensions whose name / description / author / tags match keyword."""
    kw = keyword.strip().lower()
    if not kw:
        return CATALOGUE

    results = []
    for ext in CATALOGUE:
        if (kw in ext["name"].lower()
                or kw in ext["description"].lower()
                or kw in ext["author"].lower()
                or any(kw in tag for tag in ext["tags"])):
            results.append(ext)
    return results


# ── HTTP handler ──────────────────────────────────────────────────────────────

class CatalogueHandler(BaseHTTPRequestHandler):

    def log_message(self, fmt, *args):  # noqa: N802
        print(f"[server] {self.address_string()} – {fmt % args}")

    def do_GET(self):  # noqa: N802
        parsed = urlparse(self.path)
        params = parse_qs(parsed.query)

        if parsed.path == "/extensions":
            keyword = params.get("q", [""])[0]
            results = search_extensions(keyword)
            body = json.dumps(results).encode("utf-8")
            self._send_json(200, body)

        elif parsed.path == "/install":
            ext_id = params.get("id", ["unknown"])[0]
            print(f"[server] Installing extension: {ext_id}")
            body = json.dumps({"status": "ok", "id": ext_id}).encode("utf-8")
            self._send_json(200, body)

        else:
            self.send_error(404, "Not Found")

    def _send_json(self, code: int, body: bytes):
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Connection", "close")
        self.end_headers()
        self.wfile.write(body)


# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="SALOME Extension Manager – catalogue HTTP server"
    )
    parser.add_argument("--port", type=int, default=8765,
                        help="Port to listen on (default: 8765)")
    args = parser.parse_args()

    server = HTTPServer(("127.0.0.1", args.port), CatalogueHandler)
    print(f"[server] SALOME Extension Catalogue listening on "
          f"http://127.0.0.1:{args.port}/")
    print("[server] Press Ctrl-C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[server] Stopped.")


if __name__ == "__main__":
    main()
