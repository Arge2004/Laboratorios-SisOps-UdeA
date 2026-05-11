#!/usr/bin/env python3
import argparse
import csv
import random
import re
import statistics
import subprocess
from pathlib import Path

TIME_RE = re.compile(r"Time \(\d+ processes\):\s*([0-9.]+)")


def write_matrix(path: Path, n: int, rng: random.Random) -> None:
    with path.open("w", encoding="utf-8") as f:
        for _ in range(n):
            row = [str(rng.randint(0, 9)) for _ in range(n)]
            f.write(" ".join(row) + "\n")


def parse_time(output: str) -> float:
    match = TIME_RE.search(output)
    if not match:
        raise ValueError(f"No se pudo parsear salida:\n{output}")
    return float(match.group(1))


def run_case(binary: Path, a: Path, b: Path, out: Path, k: int) -> float:
    proc = subprocess.run(
        [str(binary), str(a), str(b), str(out), str(k)],
        capture_output=True,
        text=True,
        check=True,
    )
    return parse_time(proc.stdout)


def main() -> int:
    parser = argparse.ArgumentParser(description="Benchmark de multiplicacion de matrices")
    parser.add_argument("--sizes", nargs="+", type=int, default=[120, 180, 240])
    parser.add_argument("--k-values", nargs="+", type=int, default=[1, 2, 3, 4, 5, 6, 8])
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--seq-repeats", type=int, default=1)
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()

    root = Path(__file__).resolve().parent
    c_dir = root.parent / "c"
    go_dir = root.parent / "go"
    data_dir = root / "benchmark_data"
    data_dir.mkdir(exist_ok=True)

    languages = [
        {
            "name": "c",
            "binary": c_dir / "parallel_matrix_multiply",
            "build": [
                "gcc",
                "-o",
                "parallel_matrix_multiply",
                "parallel_matrix_multiply.c",
            ],
            "cwd": c_dir,
        },
        {
            "name": "go",
            "binary": go_dir / "parallel_matrix_multiply",
            "build": [
                "go",
                "build",
                "-o",
                "parallel_matrix_multiply",
            ],
            "cwd": go_dir,
        },
    ]

    for lang in languages:
        print(f"\n== Benchmark {lang['name'].upper()} ==")
        subprocess.run(lang["build"], check=True, cwd=lang["cwd"])

        raw_path = root / f"benchmark_raw_{lang['name']}.csv"
        summary_path = root / f"benchmark_summary_{lang['name']}.csv"

        raw_rows = []
        summary_rows = []

        for n in args.sizes:
            rng = random.Random(args.seed + n)
            a_path = data_dir / f"matrix_a_{n}.txt"
            b_path = data_dir / f"matrix_b_{n}.txt"
            out_path = data_dir / f"result_{n}.txt"

            write_matrix(a_path, n, rng)
            write_matrix(b_path, n, rng)

            valid_k = [k for k in args.k_values if k > 0 and n % k == 0]
            if 1 not in valid_k:
                valid_k.insert(0, 1)

            if not valid_k:
                print(f"N={n:4d} omitido: no hay k validos")
                continue

            seq_vals = []
            for _ in range(args.seq_repeats):
                seq_t = run_case(lang["binary"], a_path, b_path, out_path, 1)
                seq_vals.append(seq_t)

            seq_avg = statistics.mean(seq_vals)
            seq_std = statistics.stdev(seq_vals) if len(seq_vals) > 1 else 0.0

            for k in valid_k:
                par_vals = []
                if k == 1:
                    par_vals = [seq_avg for _ in range(args.repeats)]
                else:
                    for _ in range(args.repeats):
                        par_t = run_case(lang["binary"], a_path, b_path, out_path, k)
                        par_vals.append(par_t)

                for run_idx, par_t in enumerate(par_vals, start=1):
                    speedup = (seq_avg / par_t) if par_t > 0 else 0.0
                    efficiency = speedup / k

                    raw_rows.append(
                        {
                            "size": n,
                            "k": k,
                            "run": run_idx,
                            "seq_time_s": f"{seq_avg:.6f}",
                            "par_time_s": f"{par_t:.6f}",
                            "speedup": f"{speedup:.6f}",
                            "efficiency": f"{efficiency:.6f}",
                        }
                    )

                par_avg = statistics.mean(par_vals)
                par_std = statistics.stdev(par_vals) if len(par_vals) > 1 else 0.0
                speedup = (seq_avg / par_avg) if par_avg > 0 else 0.0
                efficiency = speedup / k

                summary_rows.append(
                    {
                        "size": n,
                        "k": k,
                        "seq_avg_s": f"{seq_avg:.6f}",
                        "seq_std_s": f"{seq_std:.6f}",
                        "par_avg_s": f"{par_avg:.6f}",
                        "par_std_s": f"{par_std:.6f}",
                        "speedup": f"{speedup:.6f}",
                        "efficiency": f"{efficiency:.6f}",
                    }
                )

                print(
                    f"N={n:4d}, k={k:2d} -> seq={seq_avg:.4f}s, par={par_avg:.4f}s, speedup={speedup:.2f}x"
                )

        with raw_path.open("w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(
                f,
                fieldnames=["size", "k", "run", "seq_time_s", "par_time_s", "speedup", "efficiency"],
            )
            writer.writeheader()
            writer.writerows(raw_rows)

        with summary_path.open("w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(
                f,
                fieldnames=["size", "k", "seq_avg_s", "seq_std_s", "par_avg_s", "par_std_s", "speedup", "efficiency"],
            )
            writer.writeheader()
            writer.writerows(summary_rows)

        print(f"CSV generado: {summary_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
