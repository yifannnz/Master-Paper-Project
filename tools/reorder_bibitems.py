import argparse
import re
from pathlib import Path


def strip_tex_comments(tex: str) -> str:
    out_lines: list[str] = []
    for line in tex.splitlines():
        buf: list[str] = []
        i = 0
        while i < len(line):
            ch = line[i]
            # A % starts a comment unless escaped as \%
            if ch == "%" and (i == 0 or line[i - 1] != "\\"):
                break
            buf.append(ch)
            i += 1
        out_lines.append("".join(buf))
    return "\n".join(out_lines)


CITE_PAT = re.compile(r"\\cite\w*\s*(?:\[[^\]]*\]\s*)?\{([^}]*)\}")


def extract_cite_keys_in_order(tex: str) -> list[str]:
    tex = strip_tex_comments(tex)
    keys: list[str] = []
    for m in CITE_PAT.finditer(tex):
        inner = m.group(1)
        for k in inner.split(","):
            k = k.strip()
            if k:
                keys.append(k)

    seen: set[str] = set()
    uniq: list[str] = []
    for k in keys:
        if k not in seen:
            seen.add(k)
            uniq.append(k)
    return uniq


BIBITEM_START = re.compile(r"^\\bibitem\{([^}]+)\}", re.M)


def parse_bibitems_blocks(bib_text: str) -> tuple[list[str], dict[str, str]]:
    starts = [(m.start(), m.group(1)) for m in BIBITEM_START.finditer(bib_text)]
    keys_in_file: list[str] = []
    blocks: dict[str, str] = {}

    for idx, (pos, key) in enumerate(starts):
        end = starts[idx + 1][0] if idx + 1 < len(starts) else len(bib_text)
        block = bib_text[pos:end].rstrip() + "\n\n"
        keys_in_file.append(key)
        blocks[key] = block

    return keys_in_file, blocks


def main() -> int:
    parser = argparse.ArgumentParser(description="Reorder contents/bibitems_generated.tex by citation order.")
    parser.add_argument(
        "--root",
        default=str(Path(__file__).resolve().parents[1]),
        help="Project root directory (default: repo root)",
    )
    parser.add_argument(
        "--chapters",
        nargs="*",
        default=[
            "contents/chap1-引言.tex",
            "contents/chap2-文献综述.tex",
        ],
        help="Chapter tex files (relative to root) to scan in order",
    )
    parser.add_argument(
        "--bibitems",
        default="contents/bibitems_generated.tex",
        help="Bibitems file (relative to root)",
    )
    parser.add_argument(
        "--out",
        default="out-build/bibitems_generated.reordered.tex",
        help="Output path (relative to root)",
    )

    args = parser.parse_args()

    root = Path(args.root)

    cite_order: list[str] = []
    seen: set[str] = set()
    for rel in args.chapters:
        p = (root / rel)
        if not p.exists():
            continue
        keys = extract_cite_keys_in_order(p.read_text(encoding="utf-8", errors="ignore"))
        for k in keys:
            if k not in seen:
                seen.add(k)
                cite_order.append(k)

    bib_path = root / args.bibitems
    bib_text = bib_path.read_text(encoding="utf-8", errors="ignore")
    keys_in_file, blocks = parse_bibitems_blocks(bib_text)

    missing = [k for k in cite_order if k not in blocks]
    if missing:
        print("ERROR: missing \\bibitem for cited keys:")
        for k in missing:
            print("-", k)
        return 2

    new_keys = cite_order + [k for k in keys_in_file if k not in seen]
    new_text = "".join(blocks[k] for k in new_keys).rstrip() + "\n"

    out_path = root / args.out
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(new_text, encoding="utf-8")

    print(f"Cited unique keys (in order): {len(cite_order)}")
    print(f"Bibitems blocks in file: {len(keys_in_file)}")
    print(f"Wrote reordered bibitems to: {out_path}")
    print("First 20 keys:", ", ".join(new_keys[:20]))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
