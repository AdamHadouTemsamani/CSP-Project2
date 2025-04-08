import random

#Configurations
num_elements = 100
min_value = 1
max_value = 1000
output_file = "random_int_list.txt"

#Generate random_int_list
random_list = [random.randint(min_value, max_value) for _ in range (num_elements)]

#Save to file (space seperated)
with open(output_file, "w") as f: 
    f.write("\n".join(map(str, random_list)))

print(f"Random list saved in {output_file}")      
