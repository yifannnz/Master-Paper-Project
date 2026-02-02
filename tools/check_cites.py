from pathlib import Path

from ref_audit import extract_cite_keys, uniq


def main() -> int:
    chap2 = Path(__file__).resolve().parents[1] / "contents/chap2-文献综述.tex"
    bibitems = Path(__file__).resolve().parents[1] / "contents/bibitems_generated.tex"

    keys = extract_cite_keys(chap2.read_text(encoding="utf-8", errors="ignore"))
    mapping = {
        "Wang2024": "wang2024physics",
        "Zhang2024MultiphaseVNN": "zhang2024multiphase",
        "Shen2024BoneCement": "shen2024visual",
    }
    keys = [mapping.get(k, k) for k in keys]
    keys = uniq(keys)

    bib_text = bibitems.read_text(encoding="utf-8", errors="ignore")

    missing = [k for k in keys if f"\\bibitem{{{k}}}" not in bib_text]

    print(f"chap2 unique cite keys: {len(keys)}")
    print(f"missing bibitems: {len(missing)}")
    if missing:
        for k in missing:
            print(k)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
