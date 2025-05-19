import random
import struct

print("Amount:", end=" ")
count = int(input())

with open("random_integers.bin", "wb") as f:
    for _ in range(count):
        number = random.randint(0, 2**32 - 1)
        f.write(struct.pack("<I", number))
