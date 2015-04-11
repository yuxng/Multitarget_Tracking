import os

rootdir = os.path.abspath("../")

#seq_name = "egtest01"
#end_num = 1820
#seq_name = "egtest02"
#end_num = 1300
#seq_name = "egtest03"
#end_num = 2570
seq_name = "egtest04"
end_num = 1832

format_string = "Frame_%06d.conf"
start_num = 0

fo = open(os.path.join(rootdir, "data/conflist.txt"), "w");

for i in range(start_num, end_num+1):
  fo.write(os.path.join(rootdir, "data", "Confidences", seq_name, format_string) % i)
  fo.write("\n")

fo.close()
