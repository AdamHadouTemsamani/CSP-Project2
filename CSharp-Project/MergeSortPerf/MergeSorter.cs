using System;
using System.Threading.Tasks;
using System.Threading;

namespace ParallelMergeSort
{
    public static class MergeSorter
    {
        private static int counter = 0;
        public static Task SortAsync(int[] arr) => MergeSortAsync(arr, 0, arr.Length - 1);

        public static async Task MergeSortAsync(int[] arr, int left, int right)
        {
            if (left >= right) return;
            
            int mid = left + (right - left) / 2;
            
            int num = Interlocked.Increment(ref counter);
            Console.WriteLine("Before Yield {0} - ThreadId {1}", num, Thread.CurrentThread.ManagedThreadId);
            //Yield -- per parallel split
            await Task.Yield();
            Console.WriteLine("After Yield {0} - ThreadId {1}", num, Thread.CurrentThread.ManagedThreadId);
            
            var leftTask = MergeSortAsync(arr, left, mid);
            var rightTask = MergeSortAsync(arr, mid + 1, right);
            
            await Task.WhenAll(leftTask, rightTask);
            Merge(arr, left, mid, right);
        }

        private static void Merge(int[] arr, int left, int mid, int right)
        {   
            //Compute Length
            int leftLength = mid - left + 1;
            int rightLength = right - mid;
            
            //Allocate temp arrays and copy data
            var lTemp = new int[leftLength];
            var rTemp = new int[rightLength];
            Array.Copy(arr, left, lTemp, 0, leftLength);
            Array.Copy(arr, mid + 1, rTemp, 0, rightLength);
            
            //Merge
            int i = 0, j = 0, k = left;
            while (i < leftLength && j < rightLength)
                arr[k++] = (lTemp[i] <= rTemp[j]) ? lTemp[i++] : rTemp[j++];
            while (i < leftLength) arr[k++] = lTemp[i++];
            while (j < rightLength) arr[k++] = rTemp[j++];

        }
    }
}