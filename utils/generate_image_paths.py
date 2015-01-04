import os

rootdir = os.path.abspath("../")

seq_name = "egtest01"
end_num = 1820

format_string = "frame%05d.jpg"
start_num = 0

fo = open(os.path.join(rootdir, "data/imlist.txt"), "w");

for i in range(start_num, end_num+1):
  fo.write(os.path.join(rootdir, "data", "VIVID", seq_name, format_string) % i)
  fo.write("\n")

fo.close()
