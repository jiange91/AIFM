import os
import re

local_ = []
res_ = []

for i in range(100, 1001, 50): 
    os.system("cat log.{} | grep -a add: > tmp.txt".format(i))
    with open('tmp.txt', 'r') as f:
        tmp = f.readline()
        t = float(re.findall(r"\d+\.?\d*",tmp)[0])
    res_.append(t/1000/1000)     
    local_.append(i)

os.system('rm tmp.txt')
    
with open('array-add-aifm-summary.csv', 'w') as f:
    f.writelines('local_memory,exe\n')
    for i in range(len(local_)):
        f.writelines('{},{:.3f}\n'.format(local_[i], res_[i]))
        
    