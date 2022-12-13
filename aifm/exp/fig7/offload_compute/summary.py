import os
import re

local_ = []
res_ = []

for i in range(1, 32, 2):
    v = []
    
    for j in range(1, 10):
        os.system("cat log.{} | grep -a 'Step {}' > tmp.txt".format(i, j))
        with open('tmp.txt', 'r') as f:
            tmp = f.readline()
            t = float(re.findall(r"\d+\.?\d*",tmp)[1])
        v.append(t/1000/1000)

    os.system("cat log.{} | grep -a Total > tmp.txt".format(i))
    with open('tmp.txt', 'r') as f:
        tmp = f.readline()
        t = float(re.findall(r"\d+\.?\d*",tmp)[0])
    v.append(t/1000/1000) 
    
    local_.append(i)
    res_.append(v)

os.system('rm tmp.txt')
    
with open('dataframe-aifm-slow-summary.csv', 'w') as f:
    f.writelines('local_memory,step1,step2,step3,step4,step5,step6,step7,step8,step9,total\n')
    for i in range(len(local_)):
        f.writelines('{},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f}\n'.format(local_[i], res_[i][0], res_[i][1], res_[i][2], res_[i][3], res_[i][4], res_[i][5], res_[i][6], res_[i][7], res_[i][8], res_[i][9]))
        
