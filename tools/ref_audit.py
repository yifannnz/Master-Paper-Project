import re
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Set, Tuple


ROOT = Path(__file__).resolve().parents[1]


CITE_PAT = re.compile(r"\\cite\w*\s*\{([^}]*)\}")


def extract_cite_keys(text: str) -> List[str]:
    keys: List[str] = []
    for m in CITE_PAT.finditer(text):
        inner = m.group(1)
        for k in inner.split(","):
            k = k.strip()
            if k:
                keys.append(k)
    return keys


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def uniq(seq: Iterable[str]) -> List[str]:
    seen: Set[str] = set()
    out: List[str] = []
    for x in seq:
        if x not in seen:
            seen.add(x)
            out.append(x)
    return out


def _strip_outer_braces(s: str) -> str:
    s = s.strip()
    if len(s) >= 2 and s[0] == "{" and s[-1] == "}":
        return s[1:-1].strip()
    return s


def _normalize_title(title: str) -> str:
    # crude normalization: drop braces and TeX commands, keep alnum
    t = title
    t = t.replace("{", " ").replace("}", " ")
    t = re.sub(r"\\\\[a-zA-Z]+\s*", " ", t)
    t = re.sub(r"[^0-9a-zA-Z]+", " ", t)
    t = re.sub(r"\s+", " ", t).strip().lower()
    return t


def parse_bibtex_entries(bib_text: str) -> Dict[str, Dict[str, str]]:
    """Parse BibTeX into a dict: key -> fields (including '__type').

    Minimal parser that handles nested braces by tracking brace depth.
    """
    entries: Dict[str, Dict[str, str]] = {}
    i = 0
    n = len(bib_text)
    while i < n:
        at = bib_text.find("@", i)
        if at == -1:
            break
        # read entry type
        j = at + 1
        while j < n and bib_text[j].isspace():
            j += 1
        k = j
        while k < n and (bib_text[k].isalpha() or bib_text[k] in "-"):
            k += 1
        entry_type = bib_text[j:k].strip().lower()
        # find opening brace
        while k < n and bib_text[k] not in "{(":
            k += 1
        if k >= n:
            break
        open_ch = bib_text[k]
        close_ch = "}" if open_ch == "{" else ")"
        k += 1
        # read key
        while k < n and bib_text[k].isspace():
            k += 1
        key_start = k
        while k < n and bib_text[k] not in ",\n\r":
            k += 1
        entry_key = bib_text[key_start:k].strip()
        # advance to first comma after key
        comma = bib_text.find(",", k)
        if comma == -1:
            i = k
            continue
        pos = comma + 1

        fields: Dict[str, str] = {"__type": entry_type}
        name = None
        value = []
        state = "name"  # name | before_value | value
        brace_depth = 0
        quote = False

        def flush_field():
            nonlocal name, value
            if name is None:
                return
            v = "".join(value).strip()
            v = _strip_outer_braces(v)
            if v.startswith('"') and v.endswith('"') and len(v) >= 2:
                v = v[1:-1]
            fields[name.lower()] = v.strip()
            name = None
            value = []

        while pos < n:
            ch = bib_text[pos]
            if state == "name":
                # skip whitespace/commas
                if ch in " \t\r\n,":
                    pos += 1
                    continue
                if ch == close_ch and brace_depth == 0 and not quote:
                    flush_field()
                    break
                # read field name
                start = pos
                while pos < n and (bib_text[pos].isalnum() or bib_text[pos] in "-_"):
                    pos += 1
                name = bib_text[start:pos].strip()
                state = "before_value"
                continue
            if state == "before_value":
                if ch.isspace() or ch == "=":
                    pos += 1
                    continue
                # start value
                state = "value"
                continue
            # state == value
            if ch == '"' and brace_depth == 0:
                quote = not quote
                value.append(ch)
                pos += 1
                continue
            if not quote:
                if ch == "{":
                    brace_depth += 1
                elif ch == "}":
                    prev_depth = brace_depth
                    brace_depth = max(0, brace_depth - 1)
                    # If we're not inside any braced value (prev_depth==0), this '}'
                    # is the entry-closing brace.
                    if ch == close_ch and prev_depth == 0:
                        flush_field()
                        break
                elif ch == "," and brace_depth == 0:
                    flush_field()
                    state = "name"
                    pos += 1
                    continue
                elif ch == close_ch and brace_depth == 0:
                    # entry-closing delimiter for ')' style entries
                    flush_field()
                    break
            value.append(ch)
            pos += 1

        if entry_key:
            entries[entry_key] = fields
        i = pos + 1

    return entries


def main() -> int:
    cite_files = {
        "chap2": ROOT / "contents/chap2-文献综述.tex",
        "SCA_teaser": ROOT / "论文/SCA2024_camera_ready_version/EGauthorGuidelines-body_with_teaser.inc",
        "SCA": ROOT / "论文/SCA2024_camera_ready_version/EGauthorGuidelines-body.inc",
        "TVCG": ROOT / "论文/TVCG Unified_Viscoelastic_Solver_for_Multiphase_Fluid_Simulation_Based_on_a_Mixture_Model/main.tex",
        "BIBM": ROOT / "论文/BIBM2024_Visual_simulation_of_bone_cement_blending_and_dynamic_flow/conference_101719.tex",
        "CMPB": ROOT / "论文/CMPB Computer_Methods_and_Programs_in_Biomedicine/cas-sc-template.tex",
    }

    bib_files = [
        ROOT / "论文/SCA2024_camera_ready_version/main.bib",
        ROOT / "论文/TVCG Unified_Viscoelastic_Solver_for_Multiphase_Fluid_Simulation_Based_on_a_Mixture_Model/main.bib",
        ROOT / "论文/BIBM2024_Visual_simulation_of_bone_cement_blending_and_dynamic_flow/main.bib",
        ROOT / "论文/CMPB Computer_Methods_and_Programs_in_Biomedicine/cas-refs.bib",
    ]

    all_keys: Dict[str, List[str]] = {}
    for name, p in cite_files.items():
        if not p.exists():
            continue
        keys = extract_cite_keys(read_text(p))
        all_keys[name] = sorted(set(keys))

    union: List[str] = sorted(set().union(*all_keys.values())) if all_keys else []

    print("# Citation key counts")
    for k in sorted(all_keys.keys()):
        print(f"{k}: {len(all_keys[k])}")
    print(f"union: {len(union)}")

    print("\n# Sample keys (first 120)")
    for k in union[:120]:
        print(k)

    # Parse bib files
    merged: Dict[str, Dict[str, str]] = {}
    per_bib_counts: Dict[str, int] = {}
    for p in bib_files:
        if not p.exists():
            continue
        try:
            entries = parse_bibtex_entries(read_text(p))
        except Exception as e:
            print(f"WARN: failed to parse {p}: {e}")
            continue
        per_bib_counts[str(p)] = len(entries)
        # later files do not override earlier ones unless missing
        for key, fields in entries.items():
            merged.setdefault(key, fields)

    missing = [k for k in union if k not in merged]
    print("\n# Bib availability")
    print("bib per-file counts:")
    for p, c in sorted(per_bib_counts.items(), key=lambda x: x[0]):
        print(f"{p}: {c}")
    print(f"bib entries loaded: {len(merged)}")
    print(f"missing keys: {len(missing)}")
    if missing:
        print("missing sample (first 80):")
        for k in missing[:80]:
            print(k)

    # Duplicate detection by DOI, else by (title, year)
    doi_groups: Dict[str, List[str]] = {}
    ty_groups: Dict[Tuple[str, str], List[str]] = {}
    for key, f in merged.items():
        doi = f.get("doi", "").strip().lower()
        title = f.get("title", "").strip()
        year = f.get("year", "").strip()
        if doi:
            doi_groups.setdefault(doi, []).append(key)
        elif title and year:
            ty_groups.setdefault((_normalize_title(title), year), []).append(key)

    dup_doi = {d: ks for d, ks in doi_groups.items() if len(ks) > 1}
    dup_ty = {t: ks for t, ks in ty_groups.items() if len(ks) > 1 and t[0]}

    print("\n# Potential duplicates")
    print(f"doi-duplicates: {len(dup_doi)}")
    for doi, ks in list(sorted(dup_doi.items(), key=lambda x: -len(x[1])))[:20]:
        print(f"DOI {doi}: {sorted(ks)}")
    print(f"title/year-duplicates: {len(dup_ty)}")
    for (nt, year), ks in list(sorted(dup_ty.items(), key=lambda x: -len(x[1])))[:20]:
        # show one original title if possible
        t0 = merged[ks[0]].get("title", "")
        print(f"{year} '{t0}': {sorted(ks)}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
