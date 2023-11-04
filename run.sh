#!/usr/bin/env bash

algos=("atofigh" "gabow" "felerius")
target="//apps:arbok_cli"

[ $# -eq 0 ] && echo "error: missing argument with input file" && exit 1
input="$(realpath "$1")"
echo "input is ($input)"

tmp_dir=$(realpath "${PS4_GRAPHS_OUT_DIR:-$(mktemp -d)}")
echo "tmp dir is ($tmp_dir)"

echo "building cli..."
bazel build "$target" || exit 1

program="$(bazel cquery --output=files "$target" 2>/dev/null)"

for algo in "${algos[@]}"; do
    out_file="$tmp_dir/$(basename "$input")_$algo.out"

    echo "running ($algo)..."
    set -x
    "$program" -algo "$algo" -input "$input" -outfile "$out_file" -giantCC 1
    status="$?"
    set +x

    [ $status -ne 0 ] && echo "error: while running $algo" && exit 1
    sort -o "$out_file" "$out_file"
    echo "done"
done
