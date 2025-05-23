import random
import struct

print("Amount:", end=" ")
count = int(input())
progress_divider = count // 20

with open("random_integers.bin", "wb") as f:
    for i in range(count):
        if i % progress_divider == 0:
            print("{:.0f}%".format(i / progress_divider * 5), end=" ", flush=True)
        number = random.randint(0, 2**32 - 1)
        f.write(struct.pack("<I", number))
