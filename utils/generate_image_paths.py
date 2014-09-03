import os

rootdir = "/home/yuxiang/Projects/Tracking/Datasets/VIVID/"
seq_name = "egtest01"
format_string = "frame%05d.jpg"
start_num = 0
end_num = 1820

fo = open("../data/imlist.txt", "w");

for i in range(start_num, end_num+1):
  fo.write(os.path.join(rootdir, seq_name, format_string) % i)
  fo.write("\n")

fo.close()