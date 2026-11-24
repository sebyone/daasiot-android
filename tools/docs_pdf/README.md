<!-- daas © 2019–2026 Sebyone Srl -->
<!-- Contributors: plogiacco@smartlab.it -->

# docs_pdf

`md2pdf.py` lays out a Markdown document as an A4 PDF. It is adapted from the tool of the same name
in daas-api, with the same cover, stylesheet and rules.

```sh
python tools/docs_pdf/md2pdf.py docs/shared/*.md
```

- The documents of `docs/shared/` go **only** to `docs/shared/latest/<name>.pdf` (the code `_<MMMmmmpp>` dropped,
  except for the API reference `*_refs_<code>`), and the links
  between documents point to the matching PDFs.
- Any other document goes next to its `.md`, with the `_<MMmmpp>` version suffix read from `VERSION`.
- The tool never deletes anything.

Requirements: Python 3, the `markdown` package (`pip install markdown`), and Chrome or Edge (or the
`CHROME` variable set to the browser path).

Regenerate the PDFs of `docs/shared/latest/` before every release: `platforms/scripts/build_dist.py`
copies them into the `docs/` folder of the packages and refuses a PDF older than its `.md`.
