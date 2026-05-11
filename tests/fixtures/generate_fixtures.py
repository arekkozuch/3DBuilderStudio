#!/usr/bin/env python3
"""
Generate test fixtures for MeshForge unit and fuzz tests.
Run from repo root: python3 tests/fixtures/generate_fixtures.py
Requires: numpy (pip install numpy)
"""

import struct
import os
import zipfile
import json

OUT = os.path.dirname(os.path.abspath(__file__))


def write_binary_stl(path: str, triangles: list[tuple]) -> None:
    """triangles: list of ((v0x,v0y,v0z), (v1x,v1y,v1z), (v2x,v2y,v2z))"""
    with open(path, "wb") as f:
        f.write(b"\x00" * 80)  # header
        f.write(struct.pack("<I", len(triangles)))
        for t in triangles:
            # normal (0,0,0 — admesh recalculates)
            f.write(struct.pack("<fff", 0.0, 0.0, 0.0))
            for v in t:
                f.write(struct.pack("<fff", *v))
            f.write(struct.pack("<H", 0))  # attribute byte count


def cube_triangles(size: float = 10.0) -> list:
    s = size
    # 6 faces × 2 triangles = 12
    return [
        # -Z
        ((0,0,0),(s,0,0),(s,s,0)),
        ((0,0,0),(s,s,0),(0,s,0)),
        # +Z
        ((0,0,s),(s,s,s),(s,0,s)),
        ((0,0,s),(0,s,s),(s,s,s)),
        # -Y
        ((0,0,0),(s,0,s),(s,0,0)),
        ((0,0,0),(0,0,s),(s,0,s)),
        # +Y
        ((0,s,0),(s,s,0),(s,s,s)),
        ((0,s,0),(s,s,s),(0,s,s)),
        # -X
        ((0,0,0),(0,s,0),(0,s,s)),
        ((0,0,0),(0,s,s),(0,0,s)),
        # +X
        ((s,0,0),(s,s,s),(s,s,0)),
        ((s,0,0),(s,0,s),(s,s,s)),
    ]


def write_ascii_stl(path: str, triangles: list) -> None:
    with open(path, "w") as f:
        f.write("solid cube_10mm\n")
        for t in triangles:
            f.write("  facet normal 0 0 0\n    outer loop\n")
            for v in t:
                f.write(f"      vertex {v[0]} {v[1]} {v[2]}\n")
            f.write("    endloop\n  endfacet\n")
        f.write("endsolid cube_10mm\n")


def write_obj(path: str, triangles: list) -> None:
    vertices = []
    faces = []
    v_map = {}
    for t in triangles:
        face = []
        for v in t:
            key = tuple(v)
            if key not in v_map:
                v_map[key] = len(vertices) + 1
                vertices.append(v)
            face.append(v_map[key])
        faces.append(face)
    with open(path, "w") as f:
        f.write("# cube_10mm.obj\n")
        for v in vertices:
            f.write(f"v {v[0]} {v[1]} {v[2]}\n")
        for face in faces:
            f.write(f"f {face[0]} {face[1]} {face[2]}\n")


def write_3mf(path: str, triangles: list, extra_namespace_xml: str = "") -> None:
    """Write a minimal valid 3MF. extra_namespace_xml injected into <model> if provided."""
    vertices = []
    v_map = {}
    for t in triangles:
        for v in t:
            key = tuple(v)
            if key not in v_map:
                v_map[key] = len(vertices)
                vertices.append(v)

    faces = []
    for t in triangles:
        face = tuple(v_map[tuple(v)] for v in t)
        faces.append(face)

    v_xml = "\n".join(
        f'          <vertex x="{v[0]}" y="{v[1]}" z="{v[2]}"/>' for v in vertices
    )
    t_xml = "\n".join(
        f'          <triangle v1="{f[0]}" v2="{f[1]}" v3="{f[2]}"/>' for f in faces
    )

    model_xml = f"""<?xml version="1.0" encoding="UTF-8"?>
<model unit="millimeter"
  xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02"
  {extra_namespace_xml}>
  <resources>
    <object id="1" type="model">
      <mesh>
        <vertices>
{v_xml}
        </vertices>
        <triangles>
{t_xml}
        </triangles>
      </mesh>
    </object>
  </resources>
  <build>
    <item objectid="1"/>
  </build>
</model>
"""

    rels_xml = """<?xml version="1.0" encoding="UTF-8"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Target="/3D/3dmodel.model" Id="rel0"
    Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/>
</Relationships>
"""

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml",
            '<?xml version="1.0"?>'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/>'
            '</Types>')
        z.writestr("_rels/.rels", rels_xml)
        z.writestr("3D/3dmodel.model", model_xml)


def main():
    tris = cube_triangles(10.0)

    # Valid binary STL
    write_binary_stl(os.path.join(OUT, "cube_10mm.stl"), tris)
    print("cube_10mm.stl")

    # Valid ASCII STL
    write_ascii_stl(os.path.join(OUT, "cube_10mm_ascii.stl"), tris)
    print("cube_10mm_ascii.stl")

    # Valid OBJ
    write_obj(os.path.join(OUT, "cube_10mm.obj"), tris)
    print("cube_10mm.obj")

    # Valid standard 3MF
    write_3mf(os.path.join(OUT, "cube_10mm.3mf"), tris)
    print("cube_10mm.3mf")

    # 3MF with Bambu extension namespace (geometry unchanged)
    write_3mf(
        os.path.join(OUT, "cube_bambu_extensions.3mf"),
        tris,
        extra_namespace_xml='xmlns:bambu="http://schemas.bambulab.com/package/2021"'
    )
    print("cube_bambu_extensions.3mf")

    # Empty files
    for name in ("empty.stl", "empty.obj", "empty.3mf"):
        open(os.path.join(OUT, name), "wb").close()
        print(name)

    # Truncated STL (valid 80-byte header + 4-byte count claiming 100 triangles, then nothing)
    with open(os.path.join(OUT, "truncated.stl"), "wb") as f:
        f.write(b"\x00" * 80)
        f.write(struct.pack("<I", 100))  # claims 100 triangles, writes 0
    print("truncated.stl")

    # STL with bad triangle count (count >> file size)
    with open(os.path.join(OUT, "bad_count.stl"), "wb") as f:
        f.write(b"\x00" * 80)
        f.write(struct.pack("<I", 0xFFFFFFFF))  # absurd count
        f.write(b"\x00" * 50)  # one "triangle" worth of data
    print("bad_count.stl")

    # Truncated 3MF (not a valid ZIP)
    with open(os.path.join(OUT, "truncated.3mf"), "wb") as f:
        f.write(b"PK\x03\x04" + b"\x00" * 20)  # ZIP local file header, truncated
    print("truncated.3mf")

    # OBJ with missing material reference
    with open(os.path.join(OUT, "no_material.obj"), "w") as f:
        f.write("mtllib does_not_exist.mtl\n")
        for tri in tris:
            for v in tri:
                f.write(f"v {v[0]} {v[1]} {v[2]}\n")
        idx = 1
        for _ in tris:
            f.write(f"f {idx} {idx+1} {idx+2}\n")
            idx += 3
    print("no_material.obj")

    print("\nAll fixtures generated.")


if __name__ == "__main__":
    main()