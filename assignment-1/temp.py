
# imports
import sys
import subprocess
import time
import re
import matplotlib.pyplot as plt
# import commands
max_ttl=30
done=False
# function definitions


def get_ip(output):
    print(output)
    result=re.search("Time to live exceeded",output)
    if(result is None):
        global done
        pattern ="[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*: i"
        result = re.search(pattern,output)
        if result is not None:
            done=True
            return result.group()[0:-3]
        pattern ="\([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\): i"
        result = re.search(pattern,output)
        if result is not None:
            done=True
            return result.group()[1:-4]
        return "*"
        
    pattern ="\([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\) i"
    result = re.search(pattern,output)
    if result is not None:
        return result.group()[1:-3]
    pattern ="[0-9]*\.[0-9]*\.[0-9]*\.[0-9]* i"
    result = re.search(pattern,output)
    if result is not None:
        return result.group()[0:-2]
    

def callping(hostname,ttl):
    # get the ip address at current TTL
    # print("ping -c 1 -W 1 "+hostname+" -t "+str(ttl))
    # print(1)
    proc = subprocess.Popen("ping -c 1 -W 1 "+hostname+" -t "+str(ttl), shell=True,stdout=subprocess.PIPE)
    (out, err) = proc.communicate()
    # print(out)
    # print(2)
    ip=get_ip(str(out))
    # print(3)
    if(ip=="*"):
        return ["*",0]
    
    # get time required to reach the destination
    proc = subprocess.Popen("ping -c 1 -W 1 "+ip+" -t "+str(max_ttl), shell=True,stdout=subprocess.PIPE)
    (out, err) = proc.communicate()
    result = re.search("/[0-9]*\.[0-9]*/",str(out))
    if not result:
        return [ip,0]
    # print(4)
    return [ip,result.group()[1:-1]]


# main
domain=sys.argv[1]
max_hop=30
total_hops=0
finalarr=[]
for i in range(1,max_hop):
    if done:
        break
    total_hops+=1
    ans=[]
    # send 3 packets
    for j in range(3):
        ans+=[callping(domain,i)]
    finalarr+=[ans]
    # format the output
   