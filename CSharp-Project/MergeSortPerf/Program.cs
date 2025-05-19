using System.Diagnostics;

namespace MergeSortPerf;

public static class Program
{
    private const string RandomIntegersFileName = "random_integers.bin";

    public static void Main(string[] args)
    {
        if (args.Length < 2)
        {
            Console.Error.WriteLine("Usage: ParallelMergeSort <maxThreads> <arraySize>");
            return;
        }

        if (
            !int.TryParse(args[0], out int maxThreads)
            || maxThreads < 1
            || !int.TryParse(args[1], out int arraySize)
            || arraySize < 1
        )
        {
            Console.Error.WriteLine("Invalid maxThreads or arraySize");
            return;
        }

        ThreadPool.SetMinThreads(maxThreads, maxThreads);
        ThreadPool.SetMaxThreads(maxThreads, maxThreads);
        var randomIntegersPath = Path.Combine("..", "..", RandomIntegersFileName);

        if (!File.Exists(randomIntegersPath))
        {
            Console.WriteLine($"Expected to find file {RandomIntegersFileName}");
            return;
        }

        var integerBytes = File.ReadAllBytes(randomIntegersPath);

        if (integerBytes.Length < arraySize * 4)
        {
            Console.WriteLine("Size mismatch");
            return;
        }

        var integers = new uint[arraySize];

        for (var i = 0; i < arraySize; i++)
        {
            integers[i] = BitConverter.ToUInt32(integerBytes, i * 4);
        }

        var sw = Stopwatch.StartNew();
        MergeSorter.SortAsync(integers).GetAwaiter().GetResult();
        sw.Stop();

        // Converting to million items sorted per second (MI/s)
        double secs = sw.Elapsed.TotalSeconds;
        double mips = arraySize / secs / 1_000_000.0;

        Console.WriteLine($"{maxThreads},{arraySize},{secs:F6},{mips:F3}");
    }
}
