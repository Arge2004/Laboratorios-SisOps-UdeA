#!/usr/bin/env python3
import argparse
import csv
from collections import defaultdict
from pathlib import Path

COLORS = ["#0f4c5c", "#e36414", "#6a994e", "#bc4749", "#577590", "#8f2d56"]


def read_summary(path: Path):
    rows = []
    with path.open("r", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            rows.append(
                {
                    "size": int(row["size"]),
                    "k": int(row["k"]),
                    "seq_avg_s": float(row["seq_avg_s"]),
                    "par_avg_s": float(row["par_avg_s"]),
                    "speedup": float(row["speedup"]),
                }
            )
    return rows


def scale(val, min_v, max_v, min_px, max_px):
    if max_v == min_v:
        return (min_px + max_px) / 2.0
    t = (val - min_v) / (max_v - min_v)
    return min_px + t * (max_px - min_px)


def write_svg(path: Path, title: str, x_label: str, y_label: str, series: list, ideal_line=None):
    width = 900
    height = 520
    margin_left = 90
    margin_right = 30
    margin_top = 60
    margin_bottom = 80
    plot_w = width - margin_left - margin_right
    plot_h = height - margin_top - margin_bottom

    all_x = [x for _, pts, _ in series for x, _ in pts]
    all_y = [y for _, pts, _ in series for _, y in pts]
    if ideal_line:
        all_x.extend([ideal_line[0][0], ideal_line[1][0]])
        all_y.extend([ideal_line[0][1], ideal_line[1][1]])

    x_min, x_max = min(all_x), max(all_x)
    y_min = 0.0
    y_max = max(all_y) * 1.1 if max(all_y) > 0 else 1.0

    lines = []
    lines.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}">')
    lines.append('<rect width="100%" height="100%" fill="#f8f9fb"/>')
    lines.append(f'<text x="{width/2}" y="32" text-anchor="middle" font-family="sans-serif" font-size="22" fill="#1f2937">{title}</text>')

    x0 = margin_left
    y0 = margin_top + plot_h
    x1 = margin_left + plot_w
    y1 = margin_top

    lines.append(f'<line x1="{x0}" y1="{y0}" x2="{x1}" y2="{y0}" stroke="#111827" stroke-width="2"/>')
    lines.append(f'<line x1="{x0}" y1="{y0}" x2="{x0}" y2="{y1}" stroke="#111827" stroke-width="2"/>')

    k_ticks = sorted(set(all_x))
    for k in k_ticks:
        px = scale(k, x_min, x_max, x0, x1)
        lines.append(f'<line x1="{px}" y1="{y0}" x2="{px}" y2="{y0+6}" stroke="#111827"/>')
        lines.append(f'<text x="{px}" y="{y0+24}" text-anchor="middle" font-family="sans-serif" font-size="12" fill="#111827">{k}</text>')

    y_ticks = 6
    for i in range(y_ticks + 1):
        v = y_min + (y_max - y_min) * (i / y_ticks)
        py = scale(v, y_min, y_max, y0, y1)
        lines.append(f'<line x1="{x0-6}" y1="{py}" x2="{x0}" y2="{py}" stroke="#111827"/>')
        lines.append(f'<line x1="{x0}" y1="{py}" x2="{x1}" y2="{py}" stroke="#e5e7eb"/>')
        lines.append(f'<text x="{x0-10}" y="{py+4}" text-anchor="end" font-family="sans-serif" font-size="12" fill="#111827">{v:.2f}</text>')

    lines.append(f'<text x="{width/2}" y="{height-22}" text-anchor="middle" font-family="sans-serif" font-size="14" fill="#111827">{x_label}</text>')
    lines.append(f'<text x="22" y="{height/2}" text-anchor="middle" transform="rotate(-90 22 {height/2})" font-family="sans-serif" font-size="14" fill="#111827">{y_label}</text>')

    if ideal_line:
        (ix1, iy1), (ix2, iy2) = ideal_line
        px1 = scale(ix1, x_min, x_max, x0, x1)
        py1 = scale(iy1, y_min, y_max, y0, y1)
        px2 = scale(ix2, x_min, x_max, x0, x1)
        py2 = scale(iy2, y_min, y_max, y0, y1)
        lines.append(f'<line x1="{px1}" y1="{py1}" x2="{px2}" y2="{py2}" stroke="#4b5563" stroke-width="2" stroke-dasharray="8,6"/>')

    legend_x = x1 - 220
    legend_y = y1 + 10
    for idx, (name, points, color) in enumerate(series):
        pts = []
        for x, y in points:
            px = scale(x, x_min, x_max, x0, x1)
            py = scale(y, y_min, y_max, y0, y1)
            pts.append(f"{px},{py}")
        polyline = " ".join(pts)
        lines.append(f'<polyline fill="none" stroke="{color}" stroke-width="3" points="{polyline}"/>')
        for x, y in points:
            px = scale(x, x_min, x_max, x0, x1)
            py = scale(y, y_min, y_max, y0, y1)
            lines.append(f'<circle cx="{px}" cy="{py}" r="3.5" fill="{color}"/>')

        ly = legend_y + idx * 22
        lines.append(f'<line x1="{legend_x}" y1="{ly}" x2="{legend_x+22}" y2="{ly}" stroke="{color}" stroke-width="3"/>')
        lines.append(f'<text x="{legend_x+28}" y="{ly+4}" font-family="sans-serif" font-size="12" fill="#111827">{name}</text>')

    lines.append("</svg>")
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generar graficas SVG para el benchmark")
    parser.add_argument("--summary", type=Path, default=None)
    parser.add_argument("--out-dir", type=Path, default=None)
    parser.add_argument("--prefix", type=str, default="")
    parser.add_argument("--title-suffix", type=str, default="")
    args = parser.parse_args()

    root = Path(__file__).resolve().parent
    summary = args.summary or (root / "benchmark_summary.csv")
    plots_dir = args.out_dir or (root / "plots")
    plots_dir.mkdir(exist_ok=True)

    rows = read_summary(summary)
    by_size = defaultdict(list)
    for r in rows:
        by_size[r["size"]].append(r)

    series_time = []
    series_speedup = []

    sizes = sorted(by_size.keys())
    for i, n in enumerate(sizes):
        points_time = sorted((r["k"], r["par_avg_s"]) for r in by_size[n])
        points_speedup = sorted((r["k"], r["speedup"]) for r in by_size[n])
        color = COLORS[i % len(COLORS)]
        series_time.append((f"N={n}", points_time, color))
        series_speedup.append((f"N={n}", points_speedup, color))

    max_k = max(r["k"] for r in rows)

    title_suffix = f" {args.title_suffix}" if args.title_suffix else ""

    write_svg(
        plots_dir / f"{args.prefix}time_vs_k.svg",
        title=f"Tiempo Paralelo Promedio vs k{title_suffix}",
        x_label="Numero de procesos (k)",
        y_label="Tiempo paralelo promedio (s)",
        series=series_time,
    )

    write_svg(
        plots_dir / f"{args.prefix}speedup_vs_k.svg",
        title=f"Speedup vs k{title_suffix}",
        x_label="Numero de procesos (k)",
        y_label="Speedup",
        series=series_speedup,
        ideal_line=((1, 1), (max_k, max_k)),
    )

    print(f"Graficas generadas en: {plots_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
