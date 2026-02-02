import re
from pathlib import Path
from typing import Dict, List, Tuple

from ref_audit import extract_cite_keys, parse_bibtex_entries, read_text, uniq


ROOT = Path(__file__).resolve().parents[1]


def strip_tex_braces(s: str) -> str:
    if not s:
        return ""
    amp = "@@AMP@@"
    s = s.replace("\\&", amp)

    # Convert common TeX accent macros like \'e, \'{e}, \"o, \~n to plain letters.
    s = re.sub(r"\\[`'\"\^~=.uvHc]\s*\{?\s*([A-Za-z])\s*\}?", r"\1", s)

    # Drop remaining TeX commands (best-effort).
    s = re.sub(r"\\[a-zA-Z]+\*?(\[[^\]]*\])?(\{[^}]*\})?", "", s)

    # Drop any remaining backslashes/braces.
    s = s.replace("\\", "")
    s = s.replace("{", "").replace("}", "")
    # Remove stray apostrophes introduced by stripping \' accent macros.
    s = re.sub(r"([A-Za-z])'([A-Za-z])", r"\1\2", s)
    s = re.sub(r"\s+", " ", s).strip()
    s = s.replace(amp, "\\&")
    return s


def split_authors(author_field: str) -> List[str]:
    if not author_field:
        return []
    parts = [p.strip() for p in author_field.replace("\n", " ").split(" and ")]
    return [p for p in parts if p]


def _initials(given: str) -> str:
    tokens = [t for t in re.split(r"[\s\-]+", given) if t]
    out = "".join(t[0].upper() for t in tokens if t[0].isalpha())
    return out


def format_author(name: str) -> str:
    name = strip_tex_braces(name)
    if "," in name:
        last, given = [p.strip() for p in name.split(",", 1)]
    else:
        toks = [t for t in name.split() if t]
        if not toks:
            return ""
        last = toks[-1]
        given = " ".join(toks[:-1])
    ini = _initials(given)
    if ini:
        return f"{last} {ini}"
    return last


def format_authors(author_field: str, max_authors: int = 3) -> str:
    authors = [format_author(a) for a in split_authors(author_field)]
    authors = [a for a in authors if a]
    if not authors:
        return ""
    if len(authors) > max_authors:
        return ", ".join(authors[:max_authors]) + ", et al."
    return ", ".join(authors)


def ref_tag(entry_type: str) -> str:
    t = entry_type.lower()
    if t in {"article"}:
        return "[J]"
    if t in {"inproceedings", "conference", "proceedings"}:
        return "[C]"
    if t in {"techreport"}:
        return "[R]"
    if t in {"book"}:
        return "[M]"
    if t in {"phdthesis", "mastersthesis"}:
        return "[D]"
    return "[Z]"


def pick_field(f: Dict[str, str], *names: str) -> str:
    for n in names:
        v = f.get(n, "").strip()
        if v:
            return v
    return ""


def format_entry(key: str, f: Dict[str, str]) -> str:
    et = f.get("__type", "misc")

    # Heuristic override: some .bib files use nonstandard entry types.
    has_journal = bool(pick_field(f, "journal"))
    has_booktitle = bool(pick_field(f, "booktitle"))
    if has_journal:
        tag = "[J]"
    elif has_booktitle:
        tag = "[C]"
    else:
        tag = ref_tag(et)

    authors = format_authors(pick_field(f, "author")).rstrip(".")
    title = strip_tex_braces(pick_field(f, "title"))
    year = pick_field(f, "year")

    if not title:
        title = key

    if has_journal or et.lower() == "article":
        journal = strip_tex_braces(pick_field(f, "journal"))
        vol = pick_field(f, "volume")
        num = pick_field(f, "number")
        pages = pick_field(f, "pages").replace("--", "-")
        parts = []
        if journal:
            parts.append(journal)
        if year:
            parts.append(year)
        vn = ""
        if vol and num:
            vn = f"{vol}({num})"
        elif vol:
            vn = vol
        elif num:
            vn = f"({num})"
        tail = ""
        if vn and pages:
            tail = f"{vn}: {pages}"
        elif vn:
            tail = vn
        elif pages:
            tail = pages
        if tail:
            parts.append(tail)
        venue = ", ".join(parts)
        if authors:
            return f"{authors}. {title}{tag}. {venue}."
        return f"{title}{tag}. {venue}."

    if has_booktitle or et.lower() in {"inproceedings", "conference", "proceedings"}:
        booktitle = strip_tex_braces(pick_field(f, "booktitle"))
        pages = pick_field(f, "pages").replace("--", "-")
        venue_parts = []
        if booktitle:
            venue_parts.append(booktitle)
        if year:
            venue_parts.append(year)
        if pages:
            venue_parts.append(pages)
        venue = ": ".join(venue_parts) if venue_parts else year
        if authors:
            return f"{authors}. {title}{tag}//{venue}."
        return f"{title}{tag}//{venue}."

    if et.lower() == "techreport":
        inst = strip_tex_braces(pick_field(f, "institution"))
        num = pick_field(f, "number")
        venue = ", ".join([p for p in [inst, year, num] if p])
        if authors:
            return f"{authors}. {title}{tag}. {venue}."
        return f"{title}{tag}. {venue}."

    # fallback
    how = strip_tex_braces(pick_field(f, "howpublished", "publisher", "note"))
    venue = ", ".join([p for p in [how, year] if p])
    if authors:
        return f"{authors}. {title}{tag}. {venue}." if venue else f"{authors}. {title}{tag}."
    return f"{title}{tag}. {venue}." if venue else f"{title}{tag}."


def load_merged_bib() -> Dict[str, Dict[str, str]]:
    bib_files = [
        ROOT / "论文/SCA2024_camera_ready_version/main.bib",
        ROOT / "论文/TVCG Unified_Viscoelastic_Solver_for_Multiphase_Fluid_Simulation_Based_on_a_Mixture_Model/main.bib",
        ROOT / "论文/BIBM2024_Visual_simulation_of_bone_cement_blending_and_dynamic_flow/main.bib",
        ROOT / "论文/CMPB Computer_Methods_and_Programs_in_Biomedicine/cas-refs.bib",
    ]

    merged: Dict[str, Dict[str, str]] = {}
    for p in bib_files:
        if not p.exists():
            continue
        entries = parse_bibtex_entries(read_text(p))
        for k, v in entries.items():
            merged.setdefault(k, v)
    return merged


def apply_canonical(keys: List[str]) -> List[str]:
    # Keep this small and explicit; expand if more duplicates are found.
    mapping = {
        "Wang2024": "wang2024physics",
        "Zhang2024MultiphaseVNN": "zhang2024multiphase",
        "Shen2024BoneCement": "shen2024visual",
    }
    out = []
    for k in keys:
        out.append(mapping.get(k, k))
    return out


def main() -> int:
    chap2 = ROOT / "contents/chap2-文献综述.tex"
    keys_in_order = extract_cite_keys(read_text(chap2))
    keys_in_order = apply_canonical(keys_in_order)
    keys_unique = uniq(keys_in_order)

    merged = load_merged_bib()

    manual: Dict[str, str] = {
        "ShenUnifiedMCT": "Shen L, Zhang Y, Frey S, et al. A unified viscoelastic solver for multiphase fluid simulation based on a mixture model[Z]. Unpublished manuscript.",
        # Keep a couple of classics in case they are absent from the bib databases.
        "cross65": "Cross M M. Rheology of non-Newtonian fluids: a new flow equation for pseudoplastic systems[J]. Journal of Colloid Science, 1965.",
        "carreau1972rheological": "Carreau P J. Rheological equations from molecular network theories[J]. Transactions of the Society of Rheology, 1972, 16(1): 99-127.",
        "gingold1977smoothed": "Gingold R A, Monaghan J J. Smoothed particle hydrodynamics: theory and application to non-spherical stars[J]. Monthly Notices of the Royal Astronomical Society, 1977, 181(3): 375-389.",
        "monaghan2005smoothed": "Monaghan J J. Smoothed particle hydrodynamics[J]. Reports on Progress in Physics, 2005, 68(8): 1703.",
        "Mikko96": "Manninen M, Taivassalo V, Kallio S. On the mixture model for multiphase flow[R]. VTT Publications, 1996(288): 3-67.",
        "wang2024physics": "Wang X, Xu Y, Liu S, et al. Physics-based fluid simulation in computer graphics: Survey, research trends, and challenges[J]. Computational Visual Media, 2024: 1-56.",
    }

    bibitems: List[str] = []
    missing: List[str] = []
    for k in keys_unique:
        if k in manual:
            bibitems.append(f"\n\t\\bibitem{{{k}}}\n\t{manual[k]}\n")
            continue
        f = merged.get(k)
        if not f:
            missing.append(k)
            continue
        bibitems.append(f"\n\t\\bibitem{{{k}}}\n\t{format_entry(k, f)}\n")

    out = "".join(bibitems).rstrip() + "\n"

    print(out)

    if missing:
        print("\n% MISSING KEYS (need manual bibitems or key mapping):")
        for k in missing:
            print(f"% - {k}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
