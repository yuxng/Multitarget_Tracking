import os

rootdir = os.path.abspath("../")

seq_name = "egtest01"
end_num = 1820

format_string = "Frame_%06d.conf"
start_num = 0

fo = open(os.path.join(rootdir, "data/conflist.txt"), "w");

for i in range(start_num, end_num+1):
  fo.write(os.path.join(rootdir, "data", "Confidences", seq_name, format_string) % i)
  fo.write("\n")

fo.close()
