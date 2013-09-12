from numpy import *
fileHandle =open('matlabtopology')
topology=fileHandle.readlines()
fileHandle.close()

for i in range(len(topology)):
    topology[i]=topology[i].split()
    for j in range(len(topology[i])):
        topology[i][j]=int(topology[i][j])
N=len(topology)

weight=zeros((N,N))
for i in range(len(topology)):
    for j in range(len(topology[i])):
        if topology[i][j]==1:
            weight[i,j]=int(random.uniform(0,50))
print weight[19]
