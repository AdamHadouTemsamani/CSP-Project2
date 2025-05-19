pub fn parallel_merge_sort(lst: &mut [u32]) {
    let len = lst.len();
    if len <= 1 {
        return;
    }
    let mid = len / 2;
    let (left, right) = lst.split_at_mut(mid);
    rayon::join(|| parallel_merge_sort(left), || parallel_merge_sort(right));
    merge(lst, mid);
}

fn merge(lst: &mut [u32], mid: usize) {
    let left = lst[..mid].to_vec();
    let right = lst[mid..].to_vec();
    let mut i = 0;
    let mut j = 0;
    let mut k = 0;
    while i < left.len() && j < right.len() {
        if left[i] <= right[j] {
            lst[k] = left[i];
            i += 1;
            k += 1;
        } else {
            lst[k] = right[j];
            j += 1;
            k += 1;
        }
    }
    lst[k..k + (left.len() - i)].copy_from_slice(&left[i..]);
    k += left.len() - i;
    lst[k..k + (right.len() - j)].copy_from_slice(&right[j..]);
}
