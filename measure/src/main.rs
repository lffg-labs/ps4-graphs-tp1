use std::{
    collections::HashMap,
    fmt, fs,
    path::{Path, PathBuf},
    process::{Command, Stdio},
    time::Instant,
};

use itertools::Itertools;
use rayon::prelude::*;

use clap::{Parser, ValueEnum};
use color_eyre::Result;
use serde::Serialize;

#[derive(Parser, Debug)]
struct Args {
    /// Path to the `arbok_cli` executable.
    #[arg(long)]
    program_path: PathBuf,

    /// List of algorithms to test.
    #[arg(short, long = "algorithm")]
    algorithms: Vec<Algorithm>,

    /// A list of directories with files (each of which must end in `.soap` or
    /// `.wsoap`) to be used as input source for the tests.
    #[arg(short, long = "test_input_source")]
    test_input_sources: Vec<PathBuf>,
}

fn main() -> Result<()> {
    color_eyre::install()?;

    let args = Args::parse();

    let mut all_timings: HashMap<String, HashMap<Algorithm, Vec<u128>>> = HashMap::new();
    for test_dir in args.test_input_sources {
        let test_name = test_dir.file_name().unwrap().to_str().unwrap().to_owned();
        eprintln!("running suite {test_name}...");
        let input_files = suite_input_files(&test_dir)?;
        let algos_map = run_test_suite(
            &args.program_path,
            input_files.len() * args.algorithms.len(),
            input_files.into_iter(),
            args.algorithms.iter().copied(),
        )?;
        all_timings.insert(test_name, algos_map);
    }

    let json = serde_json::to_string_pretty(&all_timings)?;
    println!("{json}");

    Ok(())
}

fn run_test_suite<IterS, IterA>(
    prog: &Path,
    total: usize,
    suite_tests_iter: IterS,
    algos_iter: IterA,
) -> Result<HashMap<Algorithm, Vec<u128>>>
where
    IterS: Iterator<Item = PathBuf> + Clone + Send,
    IterA: Iterator<Item = Algorithm> + Clone + Send,
{
    let raw_data: Vec<_> = algos_iter
        .cartesian_product(suite_tests_iter)
        .enumerate()
        .par_bridge()
        .map(|(i, (algo, test))| {
            eprintln!("{i}/{total}");

            let algo_string = algo.to_string();
            let start = Instant::now();

            // {prog} -algo {algo_string} -input {test} -giantCC 1
            let mut cmd = Command::new(prog);
            cmd.arg("-algo")
                .arg(&algo_string)
                .arg("-input")
                .arg(&test)
                .arg("-giantCC")
                .arg("1")
                .stdout(Stdio::null());

            match cmd.status() {
                Ok(status) if status.success() => (),
                Ok(_) => {
                    let test_str = test.to_string_lossy();
                    panic!("{test_str}: exited with non ok status");
                }
                Err(error) => panic!("error {error}"),
            }

            let elapsed = start.elapsed().as_micros();
            (algo, elapsed)
        })
        .collect();

    let mut map: HashMap<Algorithm, Vec<u128>> = HashMap::new();
    for (key, group) in &raw_data.into_iter().group_by(|(algo, _)| *algo) {
        let group = group.map(|(_, time)| time);
        map.entry(key).or_default().extend(group);
    }
    Ok(map)
}

fn suite_input_files(suite_dir: &Path) -> Result<Vec<PathBuf>> {
    let dir = fs::read_dir(suite_dir)?;
    dir.into_iter().map(|entry| Ok(entry?.path())).collect()
}

#[derive(ValueEnum, Copy, Clone, Debug, PartialEq, Eq, Hash, Serialize)]
enum Algorithm {
    Atofigh,
    Gabow,
    Felerius,
}

impl fmt::Display for Algorithm {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Algorithm::Atofigh => f.write_str("atofigh"),
            Algorithm::Gabow => f.write_str("gabow"),
            Algorithm::Felerius => f.write_str("felerius"),
        }
    }
}
