# -*- coding: utf-8 -*-
"""
HRTEM simulation using abTEM + ASE
- Supports CNT (default), Au FCC slab, Au icosahedral nanoparticle
- Frozen phonons, multislice, CTF, save TIFF + NPY
"""

import os
import argparse
import numpy as np
import tifffile

import abtem
import ase
from ase.build import nanotube, bulk
from ase.cluster import Icosahedron


def build_structure(args):
    """
    Build ASE Atoms based on requested structure.
    Returns ASE Atoms.
    """
    if args.structure == "cnt":
        # Double-wall nanotube (default carbon nanotube from ASE)
        tube1 = nanotube(args.cnt_n1, args.cnt_m1, length=args.cnt_length)
        tube2 = nanotube(args.cnt_n2, args.cnt_m2, length=args.cnt_length)
        atoms = tube1 + tube2

        # Add vacuum in x,y directions
        atoms.center(vacuum=args.vacuum, axis=(0, 1))

        # Rotate so beam is perpendicular to tube axis
        if args.rotate_y_deg != 0:
            atoms.rotate("y", args.rotate_y_deg, rotate_cell=True)

        atoms = abtem.standardize_cell(atoms)

        # Optionally force-replace all elements (for quick tests only)
        if args.element is not None and args.element.strip() != "":
            atoms.set_chemical_symbols([args.element] * len(atoms))

        return atoms

    elif args.structure == "au_fcc":
        # Physically reasonable: Au FCC slab
        # Default lattice constant for Au ~ 4.078 Å (room temperature)
        atoms = bulk("Au", "fcc", a=args.au_a, cubic=True)
        atoms = atoms.repeat((args.au_rep_x, args.au_rep_y, args.au_rep_z))

        # Center with vacuum all directions (non-periodic simulation domain)
        atoms.center(vacuum=args.vacuum)
        atoms = abtem.standardize_cell(atoms)
        return atoms

    elif args.structure == "au_ico":
        # Physically reasonable: Au icosahedral nanoparticle
        atoms = Icosahedron("Au", noshells=args.au_ico_shells)
        atoms.center(vacuum=args.vacuum)
        atoms = abtem.standardize_cell(atoms)
        return atoms

    else:
        raise ValueError(f"Unknown structure: {args.structure}")


def parse_defocus(defocus_arg):
    """
    defocus_arg can be:
      - 'scherzer' (string)
      - a float in Angstrom (e.g., -200.0)
    """
    if isinstance(defocus_arg, str):
        s = defocus_arg.strip().lower()
        if s == "scherzer":
            return "scherzer"
        # try parse numeric string
        try:
            return float(defocus_arg)
        except ValueError:
            raise ValueError("defocus must be 'scherzer' or a number (Angstrom).")
    return defocus_arg


def main():
    parser = argparse.ArgumentParser(description="abTEM HRTEM simulation script")

    # Output
    parser.add_argument("--out_dir", type=str, default="./sim_output", help="Output directory")
    parser.add_argument("--prefix", type=str, default="hrtem", help="Output filename prefix")

    # Global physics/simulation parameters
    parser.add_argument("--energy_keV", type=float, default=100.0, help="Beam energy in keV")
    parser.add_argument("--sampling", type=float, default=0.05, help="Potential sampling (Angstrom/pixel)")
    parser.add_argument("--slice_thickness", type=float, default=1.0, help="Slice thickness (Angstrom)")
    parser.add_argument("--projection", type=str, default="infinite", help="Potential projection mode")

    # Frozen phonons
    parser.add_argument("--num_configs", type=int, default=16, help="Number of frozen phonon configurations")
    parser.add_argument("--sigmas", type=float, default=0.1, help="Frozen phonon sigma (Angstrom)")

    # CTF parameters
    parser.add_argument("--Cs_um", type=float, default=-8.0, help="Spherical aberration Cs in micrometers (um)")
    parser.add_argument("--Cc_mm", type=float, default=1.0, help="Chromatic aberration Cc in millimeters (mm)")
    parser.add_argument("--energy_spread_eV", type=float, default=0.35, help="Energy spread (eV)")
    parser.add_argument("--defocus", type=str, default="scherzer",
                        help="Defocus: 'scherzer' or numeric value in Angstrom (e.g. -200)")
    parser.add_argument("--semiangle_cutoff", type=float, default=45.0,
                        help="Semiangle cutoff (often mrad; check your abTEM version docs)")

    # Device config
    parser.add_argument("--device", type=str, default="cpu", choices=["cpu", "cuda"], help="Compute device")
    parser.add_argument("--fft", type=str, default="fftw", help="FFT backend name (e.g., fftw)")

    # Structure selection
    parser.add_argument("--structure", type=str, default="cnt",
                        choices=["cnt", "au_fcc", "au_ico"],
                        help="Structure type: cnt / au_fcc / au_ico")

    # CNT parameters
    parser.add_argument("--cnt_n1", type=int, default=10)
    parser.add_argument("--cnt_m1", type=int, default=0)
    parser.add_argument("--cnt_n2", type=int, default=16)
    parser.add_argument("--cnt_m2", type=int, default=0)
    parser.add_argument("--cnt_length", type=int, default=5, help="ASE nanotube length parameter")
    parser.add_argument("--rotate_y_deg", type=float, default=90.0, help="Rotate around y-axis (deg)")
    parser.add_argument("--element", type=str, default="", help="Force replace all atoms with this element (test only)")

    # Au FCC parameters
    parser.add_argument("--au_a", type=float, default=4.078, help="Au FCC lattice constant (Angstrom)")
    parser.add_argument("--au_rep_x", type=int, default=6)
    parser.add_argument("--au_rep_y", type=int, default=6)
    parser.add_argument("--au_rep_z", type=int, default=3)

    # Au icosahedron parameters
    parser.add_argument("--au_ico_shells", type=int, default=5, help="Icosahedron shells")

    # Vacuum
    parser.add_argument("--vacuum", type=float, default=4.0, help="Vacuum thickness (Angstrom)")

    args = parser.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)

    # abTEM config
    abtem.config.set({
        "device": args.device,
        "fft": args.fft,
    })

    # Build structure
    atoms = build_structure(args)

    # Frozen phonons
    frozen_phonons = abtem.FrozenPhonons(
        atoms,
        num_configs=args.num_configs,
        sigmas=args.sigmas
    )

    # Potential
    potential = abtem.Potential(
        frozen_phonons,
        sampling=args.sampling,
        slice_thickness=args.slice_thickness,
        projection=args.projection,
    )

    # Incident wave
    wave = abtem.PlaneWave(energy=args.energy_keV * 1e3)  # keV -> eV

    # Multislice propagation
    exit_wave = wave.multislice(potential)
    exit_wave.compute()

    # CTF
    Cs_A = args.Cs_um * 1e-6 * 1e10   # um -> m -> Angstrom
    Cc_A = args.Cc_mm * 1e-3 * 1e10   # mm -> m -> Angstrom
    defocus_val = parse_defocus(args.defocus)

    ctf = abtem.CTF(
        Cs=Cs_A,
        energy=wave.energy,
        defocus=defocus_val,
        semiangle_cutoff=args.semiangle_cutoff
    )

    # Partial coherence (chromatic focal spread approximation)
    ctf.focal_spread = Cc_A * args.energy_spread_eV / wave.energy

    # HRTEM image (no noise)
    ensemble = exit_wave.apply_ctf(ctf).intensity()

    # Average over frozen phonon configs
    image = ensemble.mean(axis=0)

    # Save
    out_tif = os.path.join(args.out_dir, f"{args.prefix}_image_clean.tif")
    out_npy = os.path.join(args.out_dir, f"{args.prefix}_image_clean.npy")

    tifffile.imwrite(out_tif, image.array.astype(np.float32))
    np.save(out_npy, image.array.astype(np.float32))

    print("Clean HRTEM simulation finished successfully.")
    print("Saved files:")
    print("  -", out_tif)
    print("  -", out_npy)


if __name__ == "__main__":
    main()
