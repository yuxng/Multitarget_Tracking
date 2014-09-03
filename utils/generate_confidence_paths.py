import os

rootdir = "/home/yuxiang/Projects/Tracking/Datasets/VIVID/"
seq_name = "egtest01_results"
conf_dir = "confidence_motion"
format_string = "Frame_%06d.conf"
start_num = 0
end_num = 1820

fo = open("../data/conflist.txt", "w");

for i in range(start_num, end_num+1):
  fo.write(os.path.join(rootdir, seq_name, conf_dir, format_string) % i)
  fo.write("\n")

fo.close()
