#!/usr/bin/env python3
"""
E2E test runner pro překladač ifj26 a interpret ic26int.

AI-GENERATED FILE: Tento celý soubor byl vygenerován pomocí nástroje Google Gemini.
Použité prompty:
    "Vytvoř e2e testy pro jazyka IFJ26 celou specifikaci a různých dalších edge cases."
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
CASES_DIR = Path(__file__).resolve().parent / "cases"
LOGS_DIR = Path(__file__).resolve().parent / "logs"
COMPILER_BIN = ROOT_DIR / "ifj26"

# (nazev_souboru, ocekavany_navratovy_kod_prekladace, ocekavany_navratovy_kod_interpretu)
CASES = [
    # Validni programy + overeni vystupu
    ("01_factorial", 0, 0),
    ("02_arithmetic", 0, 0),
    ("03_strings", 0, 0),
    ("04_functions_returns", 0, 0),
    ("05_scoping_shadowing", 0, 0),
    ("06_builtins_io", 0, 0),
    ("07_syntax_comments_indent", 0, 0),
    ("08_relational", 0, 0),
    ("09_recursion_fib", 0, 0),
    ("10_nested_returns", 0, 0),
    ("11_unit_everywhere", 0, 0),
    ("12_double_edges", 0, 0),
    ("13_string_edges", 0, 0),
    ("14_deep_shadowing", 0, 0),
    ("15_comments_whitespace", 0, 0),
    ("16_const_folding", 0, 0),
    ("17_while_nested_defvar", 0, 0),
    ("18_unary_assoc", 0, 0),
    ("19_mixed_int_double", 0, 0),
    ("60_many_params", 0, 0),
    ("61_same_names_rec", 0, 0),
    ("62_string_loop_build", 0, 0),
    ("63_deep_early_return", 0, 0),
    ("64_io_edge_inputs", 0, 0),
    ("65_reverse_string", 0, 0),
    ("66_hofstadter_rec", 0, 0),
    # Behove chyby 40
    ("20_substr_neg_start", 0, 40),
    ("21_substr_end_oob", 0, 40),
    ("22_substr_start_gt_end", 0, 40),
    ("23_substr_start_eq_len", 0, 40),
    ("24_substr_empty", 0, 40),
    # Chyby pri prekladu (1, 2, 3, 4, 5, 6, 7, 10)
    ("30_lex_tab_indent", 1, None),
    ("31_lex_unclosed_str", 1, None),
    ("32_syn_missing_import", 2, None),
    ("33_syn_bad_indent", 2, None),
    ("34_sem_missing_main", 3, None),
    ("35_sem_redef_var", 4, None),
    ("36_sem_bad_arg_count", 5, None),
    ("37_sem_type_mismatch", 6, None),
    ("38_sem_write_val", 7, None),
    ("39_sem_empty_return_int", 10, None),
    ("40_lex_invalid_escape", 1, None),
    ("41_lex_unclosed_nested_comment", 1, None),
    ("42_lex_underscore_ident", 1, None),
    ("43_lex_leading_zero_int", 1, None),
    ("44_lex_token_after_multiline_comment", 1, None),
    ("45_syn_empty_block", 2, None),
    ("46_syn_missing_else", 2, None),
    ("47_syn_chained_relop", 2, None),
    ("48_syn_funexp_in_expr", 2, None),
    ("49_sem_undef_var_in_expr", 3, None),
    ("50_sem_undef_func", 3, None),
    ("51_sem_redef_func", 4, None),
    ("52_sem_redef_param", 4, None),
    ("53_sem_bad_arg_type", 5, None),
    ("54_sem_main_with_param", 5, None),
    ("55_sem_if_not_bool", 6, None),
    ("56_sem_while_not_bool", 6, None),
    ("57_sem_return_unit_in_int", 6, None),
    ("58_sem_if_ending_int_func", 6, None),
    ("59_sem_assign_param", 7, None),
    # 20 slozitych lex testu (201-220)
    ("201_lex_trailing_underscore_int", 1, None),
    ("202_lex_trailing_underscore_double", 1, None),
    ("203_lex_trailing_underscore_exp", 1, None),
    ("204_lex_underscore_before_dot", 1, None),
    ("205_lex_underscore_after_dot", 1, None),
    ("206_lex_underscore_after_exp", 1, None),
    ("207_lex_float_suffix_f", 1, None),
    ("208_lex_int_suffix_L", 1, None),
    ("209_lex_int_suffix_d", 1, None),
    ("210_lex_dot_without_digits", 1, None),
    ("211_lex_exp_without_digits", 1, None),
    ("212_lex_exp_sign_without_digits", 1, None),
    ("213_lex_hex_literal", 1, None),
    ("214_lex_single_bang", 1, None),
    ("215_lex_bad_annotation_at", 1, None),
    ("216_lex_at_main_extra_chars", 1, None),
    ("217_lex_unclosed_triple_str", 1, None),
    ("218_lex_tab_in_paren_cont", 1, None),
    ("219_lex_unindented_paren_cont", 1, None),
    ("220_lex_unknown_char_brace", 1, None),
    # 20 slozitych syn/sem testu (221-240)
    ("221_syn_bad_dedent", 2, None),
    ("222_syn_else_misaligned", 2, None),
    ("223_syn_double_semicolon", 2, None),
    ("224_syn_line_starts_op", 2, None),
    ("225_syn_call_expr_arg", 2, None),
    ("226_syn_trailing_comma_inline", 2, None),
    ("227_syn_assign_to_expr", 2, None),
    ("228_sem_use_before_def", 3, None),
    ("229_sem_out_of_scope", 3, None),
    ("230_sem_undef_in_while", 3, None),
    ("231_sem_duplicate_main", 4, None),
    ("232_sem_duplicate_params", 4, None),
    ("233_sem_redef_builtin", 4, None),
    ("234_sem_builtin_arg_type", 5, None),
    ("235_sem_builtin_arg_count", 5, None),
    ("236_sem_double_to_int", 6, None),
    ("237_sem_string_minus", 6, None),
    ("238_sem_main_wrong_ret", 6, None),
    ("239_sem_return_inferred", 6, None),
    ("240_sem_assign_val_while", 7, None),
] + [
    (p.stem, 0, 0)
    for p in sorted(CASES_DIR.glob("1[0-9][0-9]_*.scala"))
]


def find_interpreter(explicit_path: str | None) -> tuple[str, list[str]] | None:
    """Najde interpret ic26int (binarka nebo docker/podman kontejner)."""
    candidates = []
    if explicit_path:
        candidates.append(explicit_path)
    if os.environ.get("IC26INT"):
        candidates.append(os.environ["IC26INT"])
    candidates.append(str(ROOT_DIR / "ifj26Interpret"))
    candidates.append(str(ROOT_DIR / "ic26int"))
    which_bin = shutil.which("ic26int")
    if which_bin:
        candidates.append(which_bin)
    candidates.append("/pub/courses/ifj/ic26int/linux/ic26int")

    for path in candidates:
        if path and os.path.isfile(path) and os.access(path, os.X_OK):
            return ("binary", [path])

    for runtime in ("docker", "podman"):
        rt_path = shutil.which(runtime)
        if rt_path:
            return ("container", [rt_path])

    return None


def run_interpreter(
    interp: tuple[str, list[str]], code_path: Path, stdin_data: bytes, verbose: bool = False
) -> tuple[subprocess.CompletedProcess, list[str]]:
    kind, cmd = interp
    if kind == "binary":
        full_cmd = cmd + (["--verbose"] if verbose else []) + [str(code_path)]
        res = subprocess.run(
            full_cmd,
            input=stdin_data,
            capture_output=True,
            timeout=10,
        )
        return res, full_cmd
    # Docker / Podman
    runtime = cmd[0]
    abs_code = str(code_path.resolve())
    full_cmd = [
        runtime,
        "run",
        "--rm",
        "-i",
        "-v",
        f"{abs_code}:/app/prog:ro",
        "docker.io/leondryaso/ic26int:latest",
    ] + (["--verbose"] if verbose else []) + ["prog"]
    res = subprocess.run(
        full_cmd,
        input=stdin_data,
        capture_output=True,
        timeout=30,
    )
    return res, full_cmd


def save_failure_log(
    name: str,
    full_cmd: list[str],
    code_bytes: bytes,
    run_res: subprocess.CompletedProcess,
) -> Path:
    LOGS_DIR.mkdir(parents=True, exist_ok=True)
    code_log = LOGS_DIR / f"{name}.ifjcode"
    code_log.write_bytes(code_bytes)
    detail_log = LOGS_DIR / f"{name}.log"
    detail_log.write_text(
        f"CMD: {' '.join(full_cmd)}\n"
        f"RETURNCODE: {run_res.returncode}\n"
        f"--- STDOUT ---\n{run_res.stdout.decode(errors='replace')}\n"
        f"--- STDERR ---\n{run_res.stderr.decode(errors='replace')}\n",
        encoding="utf-8",
    )
    return detail_log


def main() -> int:
    parser = argparse.ArgumentParser(description="E2E testy pro IFJ26 prekladac")
    parser.add_argument(
        "--ic26int",
        help="Cesta k binarce interpretu ic26int (nebo pouzijte env IC26INT)",
    )
    parser.add_argument(
        "--compile-only",
        action="store_true",
        help="Spusti pouze preklad (bez interpretu ic26int)",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Preda --verbose interpretu a vypise detailni logy",
    )
    args = parser.parse_args()

    if not COMPILER_BIN.exists():
        print(f"CHYBA: Prekladac {COMPILER_BIN} neexistuje. Spustte nejdrive 'make'.")
        return 1

    interp = None if args.compile_only else find_interpreter(args.ic26int)
    if interp is None:
        print(
            "POZNAMKA: Interpret ic26int (ani docker/podman) nebyl nalezen. "
            "Bezi pouze kontrola prekladu (--compile-only).\n"
            "          Pro plne spusteni zadejte --ic26int /cesta/k/ic26int "
            "nebo nastavte IC26INT.\n"
        )
    else:
        print(f"Pouzity interpret: {interp[0]} ({' '.join(interp[1])})\n")

    passed = 0
    failed = 0

    with tempfile.TemporaryDirectory() as tmpdir:
        tmp_path = Path(tmpdir)

        for name, exp_comp_rc, exp_run_rc in CASES:
            src_file = CASES_DIR / f"{name}.scala"
            in_file = CASES_DIR / f"{name}.in"
            out_file = CASES_DIR / f"{name}.out"

            src_bytes = src_file.read_bytes()
            comp = subprocess.run(
                [str(COMPILER_BIN)],
                input=src_bytes,
                capture_output=True,
                timeout=10,
            )

            if comp.returncode != exp_comp_rc:
                print(
                    f"[FAIL] {name}: ocekavan navratovy kod prekladace {exp_comp_rc}, "
                    f"vracen {comp.returncode}"
                )
                if comp.stderr:
                    print(f"       stderr: {comp.stderr.decode(errors='replace').strip()}")
                failed += 1
                continue

            if exp_comp_rc != 0:
                print(f"[PASS] {name} (preklad chyba {comp.returncode})")
                passed += 1
                continue

            if not comp.stdout.startswith(b".IFJcode26"):
                print(f"[FAIL] {name}: vystup nezacina hlavickou .IFJcode26")
                failed += 1
                continue

            if interp is None:
                print(f"[PASS] {name} (pouze preklad OK)")
                passed += 1
                continue

            code_file = tmp_path / f"{name}.ifjcode"
            code_file.write_bytes(comp.stdout)
            stdin_data = in_file.read_bytes() if in_file.exists() else b""

            run_res, full_cmd = run_interpreter(interp, code_file, stdin_data, args.verbose)
            if run_res.returncode != exp_run_rc:
                log_path = save_failure_log(name, full_cmd, comp.stdout, run_res)
                print(
                    f"[FAIL] {name}: ocekavan navratovy kod interpretu {exp_run_rc}, "
                    f"vracen {run_res.returncode}"
                )
                print(f"       cmd: {' '.join(full_cmd)}")
                if run_res.stdout:
                    print(f"       interp stdout: {run_res.stdout.decode(errors='replace').strip()}")
                if run_res.stderr:
                    print(f"       interp stderr: {run_res.stderr.decode(errors='replace').strip()}")
                print(f"       log: {log_path}")
                failed += 1
                continue

            if exp_run_rc == 0 and out_file.exists():
                expected_out = out_file.read_bytes()
                if run_res.stdout != expected_out:
                    log_path = save_failure_log(name, full_cmd, comp.stdout, run_res)
                    print(f"[FAIL] {name}: vystup interpretu neodpovida {out_file.name}")
                    print(f"       ocekavano: {expected_out!r}")
                    print(f"       ziskano:   {run_res.stdout!r}")
                    if run_res.stderr:
                        print(f"       interp stderr: {run_res.stderr.decode(errors='replace').strip()}")
                    print(f"       log: {log_path}")
                    failed += 1
                    continue

            print(f"[PASS] {name}")
            passed += 1

    print(f"\nSouhrn E2E: {passed}/{passed + failed} proslo.")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
