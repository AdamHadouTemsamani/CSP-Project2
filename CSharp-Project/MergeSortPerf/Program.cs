using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;

namespace ParallelMergeSort
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                Console.Error.WriteLine("Usage: ParallelMergeSort <maxThreads> <arraySize>");
                return;
            }

            if (!int.TryParse(args[0], out int maxThreads) || maxThreads < 1 ||
                !int.TryParse(args[1], out int arraySize) || arraySize < 1)
            {
                Console.Error.WriteLine("Invalid maxThreads or arraySize");
                return;
            }

            ThreadPool.SetMinThreads(maxThreads, maxThreads);
            ThreadPool.SetMaxThreads(maxThreads, maxThreads);

            var path = Path.Combine("..", "insert_filename.txt");
            if (!File.Exists(path))
            {
                Console.WriteLine("File not found");
                return;
            }

            var lines = File.ReadLines(path).Take(arraySize).ToList();
            if (lines.Count < arraySize)
            {
                Console.WriteLine("Size mismatch");
                return;
            }

            var data = lines
                .Select(line => int.Parse(line.Trim()))
                .ToArray();
            
            //Run and timing
            var sw = Stopwatch.StartNew(); //start
            MergeSorter.SortAsync(data).GetAwaiter().GetResult();
            sw.Stop(); //end
            
            //Converting to million items sorted per second (MI/s)
            double secs = sw.Elapsed.TotalSeconds;
            double mips = (data.Length / secs) / 1_000_000.0;
            
            //Print to output
            Console.WriteLine(
                $"{maxThreads},{arraySize},{secs:F6},{mips:F3}");
            
        }

    }
}
