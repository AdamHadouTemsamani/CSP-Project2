use std::fs::File;
use std::io::BufReader;
use std::process;
use std::time::Instant;
use std::{env, path::PathBuf};

use byteorder::{LittleEndian, ReadBytesExt};
use rayon::ThreadPoolBuilder;

mod merge_sort;
use merge_sort::parallel_merge_sort;

fn main() {
    let (max_threads, array_size) = parse_input();
    let mut numbers = read_integers();
    let pool = ThreadPoolBuilder::new()
        .num_threads(max_threads)
        .build()
        .unwrap();
    let start = Instant::now();
    pool.install(|| {
        parallel_merge_sort(&mut numbers);
    });
    let duration = start.elapsed();
    let secs = duration.as_secs_f64();
    let mips = (array_size as f64) / secs / 1_000_000.0;
    println!("{},{},{:.6},{:.3}", max_threads, array_size, secs, mips);
}

fn read_integers() -> Vec<u32> {
    let path = PathBuf::from("..").join("..").join("random_integers.bin");
    let file =
        File::open(&path).unwrap_or_else(|_| panic!("Expected to find file {}", path.display()));
    let mut reader = BufReader::new(file);
    let mut numbers = Vec::new();
    while let Ok(num) = reader.read_u32::<LittleEndian>() {
        numbers.push(num);
    }
    numbers
}

fn parse_input() -> (usize, usize) {
    let args: Vec<String> = env::args().collect();
    if args.len() < 3 {
        eprintln!("Error: expected at least 2 arguments.");
        process::exit(1);
    }
    let max_threads: usize = match args[1].parse() {
        Ok(n) => n,
        Err(_) => {
            eprintln!("Error: '{}' is not a valid integer.", args[1]);
            process::exit(1);
        }
    };
    let array_size: usize = match args[2].parse() {
        Ok(n) => n,
        Err(_) => {
            eprintln!("Error: '{}' is not a valid integer.", args[2]);
            process::exit(1);
        }
    };
    if max_threads <= 0 {
        eprintln!("Error: expected a positive integer for max_threads.");
        process::exit(1);
    }
    if array_size <= 0 {
        eprintln!("Error: expected a positive integer for array_size.");
        process::exit(1);
    }
    (max_threads, array_size)
}
