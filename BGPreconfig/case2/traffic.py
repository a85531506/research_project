from numpy import *


fileHandle = open('matlabtopology')
topology = fileHandle.readlines()
fileHandle.close()

for i in range(len(topology)):
    topology[i] = topology[i].split()
    for j in range(len(topology[i])):
        topology[i][j] = int(topology[i][j])

adjacencymatrix = matrix(topology)
N=len(topology)
routetable = ones((N,N))*-1
weighttable = ones((N,N))*900
weight = array([58,1,2,1,7,13,8,1,11,14,31,1,1,1,1,3,5,1,1,2,1,18,1,1,1,40,12,5,1,11,1,3,1,8,7,1])
count = 0
for i in range(N):
    for j in range(N):
        if adjacencymatrix[i,j]==1:
            weighttable[i,j]=weight[count]
            count +=1
#print adjacencymatrix
            
for i in range(N):
    tempweight = weighttable.copy()
    dest = set([i])
    while len(dest)!=N:
        mini=1000
        destnode = -1
        for j in dest:
            for k in range(N):
                if tempweight[j,k] < mini and k not in dest:
                    nextnode = routetable[i, j]
                    if j==i:
                        nextnode = k
                    destnode = k
                    mini = tempweight[j,k]
        routetable[i,destnode] = nextnode
        for j in range(N):
            tempweight[destnode,j]+=mini
        dest.add(destnode)
#print routetable

connect = zeros((N,1))
for i in range(N):
    connect[i] = sum(adjacencymatrix[i])

NLmatrix = zeros((N,N))
for i in range(N):
    for j in range(N):
        NLmatrix[i,j] = min(connect[i,0],connect[j,0])
        
ODindex = ones((N*(N-1),2))
count=0
for i in range(N):
    for j in range(N):
        if i!=j:
            ODindex[count,0]=i
            ODindex[count,1]=j
            count+=1

ODroute = ones((N*(N-1),N-2))*-1

for i in range(N*(N-1)):
    count=0
    nextnode=routetable[ODindex[i,0],ODindex[i,1]]
    while nextnode!=ODindex[i,1]:
        ODroute[i,count]=nextnode
        count+=1
        nextnode=routetable[nextnode, ODindex[i,1]]
        
NFUR = zeros((N,1))
for i in range(N*(N-1)):
    for j in range(N-2):
        if ODroute[i,j]!=-1:
            NFUR[ODroute[i,j],0]+=1
            
NFURmatrix = zeros((N,N))
for i in range(N):
    for j in range(N):
        maxNFUR = max(NFUR[i,0],NFUR[j,0])
        if maxNFUR==0:
            NFURmatrix[i,j]=1
        else:
            NFURmatrix[i,j]=1/maxNFUR
dtype=[('sour',int),('dest',int),('NL',int),('NFUR',float)]
values=[]
for i in range(N):
    for j in range(N):
        if i!=j:
            values.append((i,j,NLmatrix[i,j],NFURmatrix[i,j]))
strarray = array(values, dtype=dtype)
sortarray=sort(strarray,order=['NL','NFUR'])
sortarray=sortarray[::-1]

ODtrafRank=ones((N,N))*-1
for i in range(N*(N-1)):
    ODtrafRank[sortarray[i][0],sortarray[i][1]]=i

mn, sigma = 16.6, 1.04
s = random.lognormal(mn, sigma, N*(N-1))
s=sort(s)
s=s[::-1]

trafficMat=zeros((N,N))
for i in range(N):
    for j in range(N):
        trafficMat[i,j]=s[ODtrafRank[i,j]]

#compute loads
loadmatrix=zeros((N,N))
routetable2=[]
for i in range(N):
    routetable2.append([])
for i in range(N):
    for j in range(N):
        routetable2[i].append([N])

for i in range(N):
    tempweight = weighttable.copy()
    dest=set([i])
    while len(dest)!=N:
        mini=1000
        destnode=-1
        for j in dest:
            for k in range(N):
                if tempweight[j,k] < mini and k not in dest:
                    destnode = k
                    mini = tempweight[j,k]
        for j in dest:
            for k in range(N):
                if tempweight[j,k]==mini and k==destnode:
                    if j==i:
                        routetable2[i][destnode].append(k)
                    else:
                        routetable2[i][destnode].extend(routetable2[i][j][1:])
        for j in range(N):
            tempweight[destnode,j]+=mini
        dest.add(destnode)

for i in range(N):
    for j in range(N):
        if i!=j:
            routetable2[i][j]=list(set(routetable2[i][j][1:]))
    #print routetable2[i]

#compute all path
class Node(object):
    def __init__(self, data):
        self.data = data
        self.children=[]
    def add_child(self, obj):
        self.children.extend(obj)
    def child_rst(self):
        self.children=[]

nodelist = [Node]*(N+1)        
for i in range(N+1):
    nodelist[i] = Node(i)

A=[]
def printpath(nodenum, path, pathlen):
    if nodelist[nodenum].data==11:
        return
    path[pathlen]=nodelist[nodenum].data
    pathlen+=1
    if nodelist[nodenum].children[0]==11:
        #print path
        pthcp=path[:]
        A.append(pthcp)
    else:
        for tnode in nodelist[nodenum].children:
            printpath(tnode, path, pathlen)

#linkcount=zeros((N,N))   
for j in range(N): #dest node
    for k in range(N+1):
        nodelist[k].child_rst()
    for i in range(N): #parent node
        nodelist[i].add_child(routetable2[i][j])
    for i in range(N):#source node
        pth=[-1]*N
        A=[]
        printpath(i,pth,0)
        '''if i==10 and j==7:
            print A
        for i2 in range(len(A)):
            for j2 in range(N):
                if A[i2][j2+1]==-1 or A[i2][j2]==j:
                    break
                else:
                    linkcount[A[i2][j2],A[i2][j2+1]]+=1'''
        avgflow = trafficMat[i,j]/len(A)
        for i2 in range(len(A)):
            for j2 in range(N):
                if A[i2][j2]==j or A[i2][j2+1]==-1:
                    break
                else:
                    loadmatrix[A[i2][j2],A[i2][j2+1]]+=avgflow

capacity = 500000000
costmatrix=zeros((N,N))

def cost(load):
    phei = load/capacity
    if phei<0.33333:
        return 1
    elif phei<0.66666:
        return 3
    elif phei<0.9:
        return 10
    elif phei<1:
        return 70
    elif phei<1.1:
        return 500
    else:
        return 5000

for i in range(N):
    for j in range(N):
        costmatrix[i,j]=cost(loadmatrix[i,j])
print costmatrix

                


        
    
