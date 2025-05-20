using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace MergeSortPerf
{
    public static class Program
    {
        private const string RandomIntegersFileName = "random_integers.bin";

        public static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                Console.Error.WriteLine("Usage: MergeSortPerf <maxThreads> <arraySize>");
                return;
            }

            if (!int.TryParse(args[0], out int maxThreads) 
             || !int.TryParse(args[1], out int arraySize)
             || maxThreads < 1 || arraySize < 1)
            {
                Console.Error.WriteLine("Invalid arguments");
                return;
            }

            ThreadPool.SetMinThreads(maxThreads, maxThreads);
            ThreadPool.SetMaxThreads(maxThreads, maxThreads);

            var path = Path.Combine("..", "..", RandomIntegersFileName);
            if (!File.Exists(path))
            {
                Console.Error.WriteLine($"Expected {RandomIntegersFileName}");
                return;
            }

            var raw = File.ReadAllBytes(path);
            if (raw.Length < arraySize * 4)
            {
                Console.Error.WriteLine("Size mismatch");
                return;
            }

            // Build input array (we'll copy it per‐run)
            var master = new uint[arraySize];
            for (int i = 0; i < arraySize; i++)
                master[i] = BitConverter.ToUInt32(raw, i * 4);

            // 1) Cold run: time the first SortAsync on a fresh copy
            var arrCold = (uint[])master.Clone();
            var swCold = Stopwatch.StartNew();
            MergeSorter.SortAsync(arrCold).GetAwaiter().GetResult();
            swCold.Stop();

            // 2) Steady run: time a second SortAsync on another fresh copy
            var arrSteady = (uint[])master.Clone();
            var swSteady = Stopwatch.StartNew();
            MergeSorter.SortAsync(arrSteady).GetAwaiter().GetResult();
            swSteady.Stop();

            // Validate using your exact method
            ValidateSorted(arrSteady);

            // Print both results
            Print("cold",   maxThreads, arraySize, swCold.Elapsed.TotalSeconds);
            Print("steady", maxThreads, arraySize, swSteady.Elapsed.TotalSeconds);
        }

        static void Print(string phase, int threads, int size, double secs)
        {
            double mips = size / secs / 1_000_000.0;
            // e.g. "cold,4,10000000,1.234567,8.100"
            Console.WriteLine($"{phase},{threads},{size},{secs:F6},{mips:F3}");
        }

        static void ValidateSorted(uint[] a)
        {
            for (int i = 1; i < a.Length; i++)
                if (a[i] < a[i - 1])
                    throw new Exception($"Sort failed at {i}");
        }
    }
}
