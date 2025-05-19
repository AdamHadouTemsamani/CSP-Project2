namespace MergeSortPerf;

public static class MergeSorter
{
    public static Task SortAsync(uint[] arr) => MergeSortAsync(arr, 0, arr.Length - 1);

    public static async Task MergeSortAsync(uint[] arr, int left, int right)
    {
        if (left >= right)
        {
            return;
        }

        var mid = left + (right - left) / 2;

        // Yield to enable concurrent computation
        await Task.Yield();

        var leftTask = MergeSortAsync(arr, left, mid);
        var rightTask = MergeSortAsync(arr, mid + 1, right);

        await Task.WhenAll(leftTask, rightTask);
        Merge(arr, left, mid, right);
    }

    private static void Merge(uint[] arr, int left, int mid, int right)
    {
        var leftLength = mid - left + 1;
        var rightLength = right - mid;

        var leftTemp = new uint[leftLength];
        var rightTemp = new uint[rightLength];
        Array.Copy(arr, left, leftTemp, 0, leftLength);
        Array.Copy(arr, mid + 1, rightTemp, 0, rightLength);

        var i = 0;
        var j = 0;
        var k = left;

        while (i < leftLength && j < rightLength)
        {
            arr[k++] = (leftTemp[i] <= rightTemp[j]) ? leftTemp[i++] : rightTemp[j++];
        }
        while (i < leftLength)
        {
            arr[k++] = leftTemp[i++];
        }
        while (j < rightLength)
        {
            arr[k++] = rightTemp[j++];
        }
    }
}
