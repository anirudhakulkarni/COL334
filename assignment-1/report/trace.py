# imports
import sys
import subprocess
import time
import re
import matplotlib.pyplot as plt
# global constants
max_ttl=56
done=False
# function definitions


def get_ip(output):
    # print(output)
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
    proc = subprocess.Popen("ping -c 1 -W 1 "+hostname+" -t "+str(ttl), shell=True,stdout=subprocess.PIPE)
    (out, err) = proc.communicate()
    ip=get_ip(str(out))
    if(ip=="*"):
        return ["*",0]
    # get time required to reach the destination
    proc = subprocess.Popen("ping -c 2 -W 1 "+ip+" -t "+str(max_ttl), shell=True,stdout=subprocess.PIPE)
    (out, err) = proc.communicate()
    result = re.search("/[0-9]*\.[0-9]*/",str(out))
    if not result:
        return [ip,0]
    return [ip,float(result.group()[1:-1])]

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
    print(str(i),end="  ")
    if(ans[0][0]==ans[1][0] and ans[0][0]==ans[2][0]):
        if(ans[0][0]=="*"):
            print("* * *  Request timed out")
        else:
            print(ans[0][0],"",ans[0][1]," ms ",ans[1][1]," ms ",ans[2][1]," ms")

        
    else:
        for i in range(3):
            if(ans[i][0]=="*"):
                print("*",end=" ")
            else:
                print(ans[i][0],"",ans[i][1],end=" ms ")
        print()
# Plot the graph
garr=[]
for i in range(total_hops):
    for j in range(3):
        if finalarr[i][j][1]==0:
            finalarr[i][j][1]=9999
for i in range(total_hops):
    if finalarr[i][0][1]==finalarr[i][1][1]==finalarr[i][2][1]==9999:
        garr+=[0]
    else:
        garr+=[min(finalarr[i][0][1],min(finalarr[i][1][1],finalarr[i][2][1]))]
plt.plot([i for i in range(total_hops)],garr,marker='o' )
plt.xlabel('Hops')
plt.ylabel('Average RTT (ms)')
plt.savefig("trace-"+domain+".png")
plt.show()
print("Saved RTT vs hops graph at: trace-"+domain+".png")