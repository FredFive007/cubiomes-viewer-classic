"""Package the already deployed Windows port and its corresponding source."""
from pathlib import Path
import hashlib
import os
import zipfile

root = Path(os.path.abspath(__file__)).parent
dist = root / "dist"
package = dist / "Cubiomes-Viewer-26.3-Classic-4.0.1"
source_zip = package / "source-4.0.1-mc26.3-classic1.zip"
excluded = {"build", "build-qt", "build-qt5", "build-release", "dist", ".git", "__pycache__"}
with zipfile.ZipFile(source_zip, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for path in sorted(root.rglob("*")):
        relative = path.relative_to(root)
        if path.is_file() and not any(p in excluded for p in relative.parts):
            archive.write(path, Path("cubiomes-viewer-26.3-classic-source") / relative)
with zipfile.ZipFile(source_zip) as archive:
    assert archive.testzip() is None

manifest = package / "SHA256SUMS.txt"
with manifest.open("w", encoding="utf-8") as stream:
    for path in sorted(package.rglob("*")):
        if path.is_file() and path != manifest:
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            stream.write(f"{digest}  {path.relative_to(package).as_posix()}\n")
output = dist / "Cubiomes-Viewer-26.3-Classic-4.0.1-Windows-x64.zip"
with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for path in sorted(package.rglob("*")):
        if path.is_file():
            archive.write(path, Path(package.name) / path.relative_to(package))
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None
print(f"Package: {output}")
print(f"Size: {output.stat().st_size:,} bytes")
print(f"SHA256: {hashlib.sha256(output.read_bytes()).hexdigest()}")

