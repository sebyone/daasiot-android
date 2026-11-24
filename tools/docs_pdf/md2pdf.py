#!/usr/bin/env python3
# daas © 2019–2026 Sebyone Srl
#
# File: md2pdf.py — daasiot-android
#
# Lays out the Markdown documents of the project as A4 PDFs (docs/shared -> docs/shared/latest).
# Adapted from tools/docs_pdf/md2pdf.py of daas-api.
#
# Contributors:
#   plogiacco@sebyone.it (daas-api)
#   Paolo Lo Giacco <plogiacco@smartlab.it>
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
# Copyright holder: Sebyone Srl (sebyone.it). Third-party licenses: THIRD-PARTY-NOTICES.md.
#
# Changes:
#   2026-11-02  Paolo Lo Giacco  adapted from daas-api for esp_daas
#   2026-11-14  Lorenzo Grillo   daasiot-macos: header and footer
#   2026-11-19  l.grillo@sebyone.it  docs/shared: code _<MMMmmmpp> dropped from the PDF name, kept for _refs_
#   2026-11-24  Lorenzo Grillo   daasiot-android: header and footer
"""md2pdf - lays out a daasiot-android Markdown document as an A4 PDF.

    python tools/docs_pdf/md2pdf.py docs/shared/*.md

Adapted from tools/docs_pdf/md2pdf.py of daas-api.

Markdown -> HTML with python-markdown (tables, fenced code, table of contents), then HTML -> PDF with
a headless Chromium browser (Chrome or Edge; override with the CHROME environment variable). Page
size, margins, the running header (daas-iot · daasiot-android, official site daasiot.com) and the footer with the document
name and page number come from the print stylesheet below. The tool NEVER deletes anything.

Where the PDF goes:
- a document of docs/shared/ (the documents for those who use the distributed library): ONLY in
  docs/shared/latest/, with any version code dropped from the name. docs/shared/ keeps only the .md;
  latest/ holds the PDFs of the current release, overwritten when they are regenerated, and every
  package built by platforms/scripts/build_dist.py ships them in its docs/ folder;
- any other document: beside the .md. A name that already ends with a version code (`_<MMmmpp>`) gets
  <name>.pdf, any other <name>_<MMmmpp>.pdf, with the version read from the VERSION file at the
  root of the repository (the daasiot-android version), so the PDFs of earlier versions are not overwritten.

Requires: Python 3, the `markdown` package (pip install markdown), Chrome or Edge.
"""
import html
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import markdown

CSS = r"""
@page {
  size: A4;
  margin: 18mm 16mm 18mm 16mm;
  @top-left  { content: "daas — Device as a Service  ·  daas-iot  ·  daasiot-android"; font: 7.5pt Helvetica, Arial, sans-serif; color: #c10e1a; }
  @top-right { content: "daasiot.com  ·  official site of the daas project"; font: 7.5pt Helvetica, Arial, sans-serif; color: #52514e; }
  @bottom-left  { content: "__FOOTER__"; font: 7.5pt Helvetica, Arial, sans-serif; color: #52514e; }
  @bottom-right { content: "pag. " counter(page) " / " counter(pages); font: 7.5pt Helvetica, Arial, sans-serif; color: #52514e; }
}
html { -webkit-print-color-adjust: exact; print-color-adjust: exact; }
body { font: 9.4pt/1.45 Helvetica, Arial, sans-serif; color: #0b0b0b; background: #fff; margin: 0; }
h1 { font-size: 19pt; line-height: 1.2; margin: 6pt 0 6pt; }
h2 { font-size: 13.5pt; margin: 16pt 0 5pt; padding-bottom: 2pt; border-bottom: 0.6pt solid #c9c8c2; break-after: avoid; }
h3 { font-size: 11pt; margin: 12pt 0 4pt; break-after: avoid; }
h4 { font-size: 9.8pt; margin: 10pt 0 3pt; break-after: avoid; }
p { margin: 4pt 0 6pt; orphans: 3; widows: 3; }
ul, ol { margin: 3pt 0 6pt; padding-left: 16pt; }
li { margin: 1.5pt 0; }
a { color: #1d5fae; text-decoration: none; }
code { font: 8.2pt Consolas, "Courier New", monospace; background: #f2f1ee; padding: 0 2pt; border-radius: 2pt; }
pre { font: 7.9pt/1.35 Consolas, "Courier New", monospace; background: #f6f5f2; border: 0.5pt solid #e7e6e2;
      padding: 6pt 8pt; border-radius: 3pt; white-space: pre-wrap; word-break: break-word; break-inside: avoid; }
pre code { background: none; padding: 0; font-size: inherit; }
table { border-collapse: collapse; width: 100%; margin: 5pt 0 9pt; font-size: 8.2pt; break-inside: auto; }
thead { display: table-header-group; }
tr { break-inside: avoid; }
th { background: #f2f1ee; text-align: left; font-weight: bold; border-bottom: 0.8pt solid #c9c8c2; }
th, td { padding: 2.5pt 4pt; vertical-align: top; border-bottom: 0.4pt solid #e7e6e2; }
td code, th code { font-size: 7.6pt; word-break: break-word; }
blockquote { margin: 6pt 0; padding: 4pt 9pt; border-left: 2.5pt solid #2a78d6; background: #f5f8fd; color: #2b2b2a; }
blockquote p { margin: 2pt 0; }
img { max-width: 100%; }
img:not([height]) { height: auto; }   /* figures fit the page; a sized image (the logo) keeps its height */
hr { border: none; border-top: 0.6pt solid #c9c8c2; margin: 10pt 0; }
.toc ul { list-style: none; padding-left: 10pt; }
"""


def find_browser():
    env = os.environ.get("CHROME")
    if env and Path(env).exists():
        return env
    for c in (r"C:\Program Files\Google\Chrome\Application\chrome.exe",
              r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
              r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
              r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
              "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"):
        if Path(c).exists():
            return c
    for name in ("google-chrome", "chromium", "chromium-browser", "chrome", "msedge"):
        p = shutil.which(name)
        if p:
            return p
    sys.exit("md2pdf: no Chrome/Chromium/Edge found (set CHROME=<path>)")


_VERSIONED = re.compile(r"_\d{6}$")
_DOC_CODE = re.compile(r"_\d{8}$")   # docs/shared: _<MMMmmmpp> of VERSION (conventions of the daasiot-<target> repositories)


def version_code(start: Path) -> str:
    """daasiot-android version from the VERSION file of the repository, as MMmmpp (0.24.4 -> 002404)."""
    for d in [start.resolve(), *start.resolve().parents]:
        v = d / "VERSION"
        if v.is_file():
            parts = (v.read_text(encoding="utf-8").strip().split(".") + ["0", "0"])[:3]
            return "".join(f"{int(x):02d}" for x in parts)
    sys.exit("md2pdf: VERSION file not found above " + str(start))


def pdf_path(md_path: Path) -> Path:
    """docs/shared/latest/<name without _MMmmpp>.pdf for a document of docs/shared/; otherwise the PDF
    beside the source, where unversioned names get the current version code."""
    folder = md_path.resolve().parent
    if folder.name == "shared" and folder.parent.name == "docs":
        if "_refs_" in md_path.stem:   # the API reference keeps its code
            return folder / "latest" / (md_path.stem + ".pdf")
        return folder / "latest" / (_VERSIONED.sub("", _DOC_CODE.sub("", md_path.stem)) + ".pdf")
    stem = md_path.stem if _VERSIONED.search(md_path.stem) else f"{md_path.stem}_{version_code(md_path.parent)}"
    return md_path.resolve().with_name(stem + ".pdf")


def to_html(md_path: Path) -> str:
    text = md_path.read_text(encoding="utf-8")
    # Nested lists in these documents are indented by 2 spaces; python-markdown wants 4.
    text = re.sub(r"^((?:  )+)([-*] |\d+\. )", lambda m: "    " * (len(m.group(1)) // 2) + m.group(2), text, flags=re.M)
    body = markdown.markdown(text, extensions=["tables", "fenced_code", "toc", "sane_lists", "attr_list"],
                             output_format="html5")
    # Links to other Markdown documents point to their PDF when it exists, relative to where this PDF goes.
    out_dir = pdf_path(md_path).parent
    def relink(m):
        href = m.group(1)
        if href.startswith(("http:", "https:", "#", "mailto:")) or ".md" not in href:
            return m.group(0)
        target, _, frag = href.partition("#")
        pdf = pdf_path(md_path.parent / target)
        if not pdf.exists():
            return m.group(0)
        rel = Path(os.path.relpath(pdf, out_dir)).as_posix()
        return f'href="{rel}{"#" + frag if frag else ""}"'
    body = re.sub(r'href="([^"]+)"', relink, body)
    title = re.search(r"^# (.+)$", text, flags=re.M)
    title = title.group(1).strip() if title else md_path.stem
    footer = f"daasiot-android - {md_path.name} - Sebyone Srl - daasiot.com".replace('"', "'")
    css = CSS.replace("__FOOTER__", footer)
    return (f"<!doctype html><html lang='en'><head><meta charset='utf-8'><title>{html.escape(title)}</title>"
            f"<style>{css}</style></head><body>{body}</body></html>")


def convert(md_path: Path, browser: str) -> Path:
    out = pdf_path(md_path.resolve())
    out.parent.mkdir(exist_ok=True)   # docs/shared/latest/ on the first run
    with tempfile.TemporaryDirectory() as tmp:
        # temporary page beside the source, so that relative links resolve against the source folder
        page_in_place = md_path.parent / f".{md_path.stem}.md2pdf.html"
        page_in_place.write_text(to_html(md_path), encoding="utf-8")
        try:
            subprocess.run([browser, "--headless=new", "--disable-gpu", "--no-pdf-header-footer",
                            f"--user-data-dir={Path(tmp) / 'profile'}", f"--print-to-pdf={out}",
                            page_in_place.resolve().as_uri()],
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=180)
        finally:
            page_in_place.unlink(missing_ok=True)   # only the temporary page this run created
    if not out.exists():
        sys.exit(f"md2pdf: {out} was not produced")
    return out


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    browser = find_browser()
    for a in sys.argv[1:]:
        p = Path(a)
        if p.suffix.lower() != ".md" or not p.exists():
            sys.exit(f"md2pdf: not a Markdown file: {a}")
        print("written", convert(p, browser))


if __name__ == "__main__":
    main()
