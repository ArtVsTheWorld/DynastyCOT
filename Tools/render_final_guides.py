"""Render the editable, page-organized Markdown guides with ReportLab.

Run from any directory with Python and reportlab installed. Add --inspect to
render every PDF page with Poppler and check links and text using pypdf.
"""
import argparse
import re
import subprocess
from pathlib import Path
from xml.sax.saxutils import escape, quoteattr

from reportlab.lib import colors
from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.utils import ImageReader, simpleSplit
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen.canvas import Canvas
from reportlab.platypus import Paragraph

ROOT = Path(__file__).resolve().parents[1]
GUIDES = ROOT / "docs/player-guides"
QA = ROOT / "Build/PublicationFinal/guide-review"
MODULES = {
    "BeyondTheField": ("1.0.24", "#663343"),
    "GamedayAtmosphere": ("2.1.38", "#164D51"),
    "ExtraSettings": ("0.2.12", "#294A75"),
    "ExpandedPractice": ("0.8.24", "#5C4623"),
}
WIDTH, HEIGHT = letter
LEFT, RIGHT = 52, WIDTH - 52
TEXT = colors.HexColor("#242A2D")
MUTED = colors.HexColor("#49555C")
LINE = colors.HexColor("#CBD1CF")
PAPER = colors.HexColor("#FCFBF6")


def fonts():
    win = Path("C:/Windows/Fonts")
    if all((win / f).exists() for f in ("georgia.ttf", "calibri.ttf", "calibrib.ttf")):
        for name, file in (("GuideTitle", "georgia.ttf"), ("GuideBody", "calibri.ttf"),
                           ("GuideBold", "calibrib.ttf")):
            pdfmetrics.registerFont(TTFont(name, str(win / file)))
        return "GuideTitle", "GuideBody", "GuideBold"
    return "Times-Roman", "Helvetica", "Helvetica-Bold"


TITLE, BODY, BOLD = fonts()


def markup(value, accent):
    value = escape(value)
    value = re.sub(r"\*\*(.+?)\*\*", lambda m: '<font name="' + BOLD + '">' + m[1] + '</font>', value)
    return re.sub(r"\[([^]]+)\]\((https://[^ )]+)\)",
                  lambda m: '<link href=' + quoteattr(m[2]) + ' color="' + accent + '"><u>' + m[1] + '</u></link>', value)


def paragraph(c, value, left, top, width, font, size, leading, color, accent):
    style = ParagraphStyle("guide", fontName=font, fontSize=size, leading=leading,
                           textColor=color, splitLongWords=False)
    item = Paragraph(markup(value, accent), style)
    _, h = item.wrap(width, 2000)
    item.drawOn(c, left, top - h)
    return top - h


def render(module, version, accent):
    src = GUIDES / (module + "-Player-Guide.md")
    source = src.read_text(encoding="utf-8").strip()
    header, *pages = re.split(r"(?m)^## ", source)
    name = header.strip().removeprefix("# ")
    destination = src.with_suffix(".pdf")
    c = Canvas(str(destination), pagesize=letter, pageCompression=1, invariant=1)
    c.setTitle(name + " | Player guide")
    c.setAuthor("Ace")
    c.setSubject("College Football 27 player guide, version " + version)
    for n, section in enumerate(pages, 1):
        title, body = section.strip().split("\n", 1)
        c.bookmarkPage("p" + str(n))
        c.addOutlineEntry(title, "p" + str(n), level=0, closed=False)
        c.setFillColor(PAPER)
        c.rect(0, 0, WIDTH, HEIGHT, fill=1, stroke=0)
        c.setFillColor(colors.HexColor(accent))
        c.rect(0, HEIGHT - 9, WIDTH, 9, fill=1, stroke=0)
        c.setFont(BOLD, 11)
        c.drawString(LEFT, HEIGHT - 43, name.upper())
        c.setFont(BODY, 11)
        c.drawRightString(RIGHT, HEIGHT - 43, "PLAYER GUIDE    /    " + version)
        c.setStrokeColor(LINE)
        c.line(LEFT, HEIGHT - 53, RIGHT, HEIGHT - 53)
        y = HEIGHT - 78
        for line in simpleSplit(title, TITLE, 30, RIGHT - LEFT):
            c.setFont(TITLE, 30)
            c.setFillColor(TEXT)
            c.drawString(LEFT, y - 22, line)
            y -= 36
        y -= 6
        blocks = re.split(r"\n\s*\n", body.strip())
        for block in blocks:
            block = block.strip()
            if block.startswith("!["):
                match = re.fullmatch(r"!\[([^]]+)\]\((assets/[A-Za-z]+\.png)\)", block)
                if not match:
                    raise ValueError(f"Invalid guide illustration in {module}: {block}")
                caption, relative = match.groups()
                picture = ImageReader(str(GUIDES / relative))
                pixels_w, pixels_h = picture.getSize()
                w = RIGHT - LEFT
                h = w * pixels_h / pixels_w
                c.drawImage(picture, LEFT, y - h, width=w, height=h, mask="auto")
                y = paragraph(c, caption, LEFT, y - h - 7, w, BODY, 11, 14, MUTED, accent) - 13
            elif block.startswith("### "):
                y -= 7
                y = paragraph(c, block[4:], LEFT, y, RIGHT - LEFT, BOLD, 16, 19, colors.HexColor(accent), accent) - 5
            elif block.startswith("> "):
                value = block[2:]
                preview = Paragraph(markup(value, accent), ParagraphStyle("inset", fontName=BODY, fontSize=12, leading=16))
                _, h = preview.wrap(RIGHT - LEFT - 30, 2000)
                c.setFillColor(colors.HexColor("#EEF1EB"))
                c.rect(LEFT, y - h - 17, RIGHT - LEFT, h + 17, stroke=0, fill=1)
                c.setFillColor(colors.HexColor(accent))
                c.rect(LEFT, y - h - 17, 3, h + 17, stroke=0, fill=1)
                paragraph(c, value, LEFT + 15, y - 7, RIGHT - LEFT - 30, BODY, 12, 16, TEXT, accent)
                y -= h + 25
            elif block.startswith("```seats"):
                # Editable seat key in Markdown. Spaces represent real aisles.
                # A schematic illustrates the concept without implying capture.
                lines = block.splitlines()[1:-1]
                c.setFont(BOLD, 11)
                c.setFillColor(TEXT)
                c.drawString(LEFT, y - 9, "Reading a section across an aisle")
                x0, y0 = LEFT + 6, y - 25
                for row, line in enumerate(lines):
                    for col, seat in enumerate(line):
                        if seat != " ":
                            c.setFillColor(colors.HexColor(accent if seat == "V" else "#FFFFFF"))
                            c.setStrokeColor(colors.HexColor("#546668"))
                            c.circle(x0 + col * 10, y0 - row * 10, 3.2, fill=1, stroke=1)
                legend = "Filled circles are visitors. Open circles are home fans. The gap between the blocks is an aisle. This seating example is illustrative."
                paragraph(c, legend, LEFT + 257, y - 20, RIGHT - LEFT - 257, BODY, 11.5, 15, TEXT, accent)
                y = y0 - len(lines) * 10 - 12
            elif block.startswith("|"):
                rows = [[v.strip() for v in line.strip("|").split("|")] for line in block.splitlines() if not re.fullmatch(r"[\s:|-]+", line)]
                for i, (label, value) in enumerate(rows):
                    if i == 0:
                        c.setFillColor(colors.HexColor(accent))
                        c.rect(LEFT, y - 22, RIGHT - LEFT, 22, fill=1, stroke=0)
                    color = colors.white if i == 0 else TEXT
                    a = paragraph(c, label, LEFT + 8, y - 4, 160, BOLD, 11, 14, color, accent)
                    b = paragraph(c, value, LEFT + 181, y - 4, RIGHT - LEFT - 189, BOLD if i == 0 else BODY, 11.5, 14, color, accent)
                    y = min(a, b) - 5
                    c.setStrokeColor(LINE)
                    c.line(LEFT, y, RIGHT, y)
                y -= 9
            else:
                y = paragraph(c, block, LEFT, y, RIGHT - LEFT, BODY, 12.3, 16.5, TEXT, accent) - 7
        if y < 49:
            raise ValueError(f"{module} p{n} overlaps footer at {y:.1f} pt: {title}")
        c.setStrokeColor(LINE)
        c.line(LEFT, 43, RIGHT, 43)
        c.setFont(BODY, 10)
        c.setFillColor(MUTED)
        c.drawString(LEFT, 27, "Ace   /   College Football 27")
        c.setFont(BOLD, 10)
        c.drawRightString(RIGHT, 27, f"{n}  /  {len(pages)}")
        c.showPage()
    c.save()
    return destination, pages


def inspect(module, pdf, pages):
    from pypdf import PdfReader
    reader = PdfReader(pdf)
    assert len(reader.pages) == len(pages)
    urls = []
    for n, (page, source) in enumerate(zip(reader.pages, pages), 1):
        text = page.extract_text()
        title = source.splitlines()[0]
        assert " ".join(title.split()) in " ".join(text.split()), (module, n, title)
        for ann in page.get("/Annots", []):
            action = ann.get_object().get("/A", {})
            if action.get("/URI"):
                urls.append(action["/URI"])
    expected = re.findall(r"\]\((https://[^ )]+)\)", "\n".join(pages))
    assert set(urls) == set(expected), (module, urls, expected)
    QA.mkdir(parents=True, exist_ok=True)
    subprocess.run(["pdftoppm", "-r", "105", "-png", str(pdf), str(QA / module)], check=True)
    print(f"{pdf.name}: {len(pages)} pages, {len(set(urls))} distinct links, all pages rendered")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--inspect", action="store_true")
    args = parser.parse_args()
    for key, (release, color) in MODULES.items():
        pdf, pages = render(key, release, color)
        if args.inspect:
            inspect(key, pdf, pages)
