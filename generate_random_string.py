import random
import string

#Configuration
num_elements = 100
min_length = 5
max_length = 10
output_file = "random_string_list.txt"

#Characters to use
characters = string.ascii_lowercase

#Generate random_string_list
random_string_list = [''.join(random.choices(characters, k=random.randint(min_length, max_length))) for _ in range(num_elements)]

#Save to output_file seperated by space
with open(output_file, "w") as f:
    f.write("\n".join(random_string_list))
print(f"Random string list saved to {output_file}")
