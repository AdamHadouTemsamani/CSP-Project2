using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Threading;

namespace MergeSortPerf
{
    public static class Program
    {
        private const string RandomIntegersFileName = "random_integers.bin";

        public static void Main(string[] args)
        {
            Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
            var skipSteady = false;

            if (args.Length < 2)
            {
                Console.Error.WriteLine("Usage: MergeSortPerf <maxThreads> <arraySize>");
                return;
            }

            if (args.Length >= 3)
            {
                if (args[2].Equals("skipSteady", StringComparison.OrdinalIgnoreCase))
                {
                    skipSteady = true;
                }
            }

            if (
                !int.TryParse(args[0], out int maxThreads)
                || !int.TryParse(args[1], out int arraySize)
                || maxThreads < 1
                || arraySize < 1
            )
            {
                Console.Error.WriteLine("Invalid arguments");
                return;
            }

            ThreadPool.SetMinThreads(maxThreads, maxThreads);
            ThreadPool.SetMaxThreads(maxThreads, maxThreads);

            // Look for the file in the working directory
            var path = RandomIntegersFileName;
            if (!File.Exists(path))
            {
                Console.Error.WriteLine($"Expected {RandomIntegersFileName}");
                return;
            }

            var raw = new byte[arraySize * 4];

            using (var fileStream = File.OpenRead(path))
            {
                fileStream.ReadExactly(raw);
            }

            if (raw.Length < arraySize * 4)
            {
                Console.Error.WriteLine("Size mismatch");
                return;
            }

            var master = new uint[arraySize];
            for (int i = 0; i < arraySize; i++)
                master[i] = BitConverter.ToUInt32(raw, i * 4);

            var arrCold = (uint[])master.Clone();
            var swCold = Stopwatch.StartNew();
            MergeSorter.SortAsync(arrCold).GetAwaiter().GetResult();
            swCold.Stop();

            var arrSteady = (uint[])master.Clone();
            var swSteady = Stopwatch.StartNew();
            if (!skipSteady)
            {
                MergeSorter.SortAsync(arrSteady).GetAwaiter().GetResult();
            }
            swSteady.Stop();

            ValidateSorted(arrSteady);

            Print("cold", maxThreads, arraySize, swCold.Elapsed.TotalSeconds);
            if (!skipSteady)
            {
                Print("steady", maxThreads, arraySize, swSteady.Elapsed.TotalSeconds);
            }
            Thread.Sleep(1000);
        }

        static void Print(string phase, int threads, int size, double secs)
        {
            double mips = size / secs / 1_000_000.0;
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
