use std::fs::File;
use std::io::{BufReader, Read};
use std::process;
use std::time::Instant;
use std::{env, path::PathBuf};

use byteorder::{LittleEndian, ReadBytesExt};
use rayon::ThreadPoolBuilder;

mod merge_sort;
use merge_sort::parallel_merge_sort;

fn main() {
    let (max_threads, array_size) = parse_input();

    let mut integers = read_integers(array_size);

    let pool = ThreadPoolBuilder::new()
        .num_threads(max_threads)
        .build()
        .unwrap();

    let start = Instant::now();
    pool.install(|| {
        parallel_merge_sort(&mut integers);
    });
    let duration = start.elapsed();

    // validate
    for i in 1..integers.len() {
        if integers[i] < integers[i-1] {
            eprintln!(
                "Array is not sorted at index {}: {} < {}",
                i, integers[i], integers[i-1]
            );
            process::exit(1);
        }
    }

    let secs = duration.as_secs_f64();
    let mips = (array_size as f64) / secs / 1_000_000.0;
    println!("steady,{},{:.6},{:.3}", max_threads, array_size, secs, mips);
}

fn read_integers(n: usize) -> Vec<u32> {
    let path = PathBuf::from("random_integers.bin");
    let file = File::open(&path)
        .unwrap_or_else(|_| panic!("Expected to find file {}", path.display()));
    let mut reader = BufReader::new(file);

    let mut numbers = Vec::with_capacity(n);
    for i in 0..n {
        match reader.read_u32::<LittleEndian>() {
            Ok(num) => numbers.push(num),
            Err(e) => {
                eprintln!("Error: only read {} integers (at index {}): {}", i, i, e);
                process::exit(1);
            }
        }
    }
    numbers
}

fn parse_input() -> (usize, usize) {
    let args: Vec<String> = env::args().collect();
    if args.len() < 3 {
        eprintln!("Usage: <program> <maxThreads> <arraySize>");
        process::exit(1);
    }
    let max_threads: usize = args[1].parse().unwrap_or_else(|_| {
        eprintln!("Error: '{}' is not a valid integer.", args[1]);
        process::exit(1);
    });
    let array_size: usize = args[2].parse().unwrap_or_else(|_| {
        eprintln!("Error: '{}' is not a valid integer.", args[2]);
        process::exit(1);
    });
    if max_threads < 1 || array_size < 1 {
        eprintln!("Invalid arguments: both must be ≥ 1");
        process::exit(1);
    }
    (max_threads, array_size)
}