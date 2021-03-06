#this program generate the traffic according to the origin weight
#trafficMat is the traffic matrix

from cvxopt import *

fileHandle = open('matlabtopology')
topology = fileHandle.readlines()
fileHandle.close()

fileHandle = open('weights.1')
weight = fileHandle.readlines()
fileHandle.close()

fileHandle = open('pathspecifier')
specifier = fileHandle.readlines()
fileHandle.close()

fileHandle = open('pathpref')
preference = fileHandle.readlines()
fileHandle.close()

for i in range(len(topology)):
    topology[i] = topology[i].split()
    for j in range(len(topology[i])):
        topology[i][j] = int(topology[i][j])

for i in range(len(weight)):
    weight[i] = weight[i].split()
    for j in range(len(weight[i])):
        weight[i][j] = int(weight[i][j])

for i in range(len(specifier)):
    specifier[i] = specifier[i].split()
    for j in range(len(specifier[i])):
        specifier[i][j] = int(specifier[i][j])

for i in range(len(preference)):
    preference[i] = preference[i].split()
    for j in range(len(preference[i])):
        preference[i][j] = int(preference[i][j])

N=len(topology)
Pathnum = len(specifier)
adjacencymatrix = matrix(topology).T
pathspec = matrix(specifier).T
squarew = matrix(weight).T
pathpref = matrix(preference).T

indexmat = matrix(-1,(N,N))
indexnum=0
weightorigin=[]
for i in range(N):
    for j in range(N):
        if adjacencymatrix[i,j]==1:
            indexmat[i,j]=indexnum
            indexnum+=1
            weightorigin.append(squarew[i,j])
weightorigin = matrix(weightorigin)
weightnum = len(weightorigin)
coeffmat = []
prefnum=0

for i in range(Pathnum):
    for j in range(Pathnum):
        if pathpref[i,j]==1:
            coefflist = [0]*weightnum
            path1 = pathspec[i,:]
            for k in range(N-2):
                if path1[0,k+1]!=-1:
                    xnum = path1[0,k]
                    ynum = path1[0,k+1]
                    coefflist[indexmat[xnum,ynum]]=1.0
                else:
                    break
            path2 = pathspec[j,:]
            for k in range(N-2):
                if path2[0,k+1]!=-1:
                    xnum = path2[0,k]
                    ynum = path2[0,k+1]
                    coefflist[indexmat[xnum,ynum]]=-1.0
                else:
                    break
            coeffmat.append(coefflist)
            prefnum +=1

coeffmat = matrix(coeffmat).T
zeromat = matrix(0,(prefnum,1))
onemat = matrix(-1.0,(prefnum,1))

A = spmatrix(1.0,range(weightnum),range(weightnum))
m,n=A.size
I1=matrix(-1,(n,1))
h=matrix([onemat,I1])
I2 = spmatrix(-1, range(n), range(n))
G=matrix([coeffmat,I2])
sol = solvers.qp(A.T*A, -A.T*weightorigin, G, h)
newweight = []
for i in range(len(sol['x'])):
    newweight.append(int(sol['x'][i]))



from numpy import *
routetable = ones((N,N))*-1
weighttable = ones((N,N))*900
weight = array(weightorigin)

count = 0
for i in range(N):
    for j in range(N):
        if adjacencymatrix[i,j]==1:
            weighttable[i,j] = weight[count]
            count+=1

for i in range(N):
    tempweight = weighttable.copy()
    dest = set([i])
    while len(dest)!=N:
        mini = 1000
        destnode = -1
        for j in dest:
            for k in range(N):
                if tempweight[j,k] < mini and k not in dest:
                    nextnode = routetable[i,j]
                    if j==i:
                        nextnode = k
                    destnode = k
                    mini = tempweight[j,k]
        routetable[i,destnode]=nextnode
        for j in range(N):
            tempweight[destnode,j]+=mini
        dest.add(destnode)

connect = zeros((N,1))
for i in range(N):
    connect[i] = sum(adjacencymatrix[i])

NLmatrix = zeros((N,N))
for i in range(N):
    for j in range(N):
        NLmatrix[i,j] = min(connect[i,0], connect[j,0])

ODindex  = ones((N*(N-1),2))
count = 0
for i in range(N):
    for j in range(N):
        if i!=j:
            ODindex[count, 0] = i
            ODindex[count, 1] = j
            count +=1

ODroute = ones((N*(N-1),N-2))*-1
for i in range(N*(N-1)):
    count = 0
    nextnode = routetable[ODindex[i,0],ODindex[i,1]]
    while nextnode!=ODindex[i,1]:
        ODroute[i,count]= nextnode
        count+=1
        nextnode = routetable[nextnode, ODindex[i,1]]

NFUR = zeros((N,1))
for i in range(N*(N-1)):
    for j in range(N-2):
        if ODroute[i,j]!=-1:
            NFUR[ODroute[i,j],0]+=1

NFURmatrix = zeros((N,N))
for i in range(N):
    for j in range(N):
        maxNFUR = max(NFUR[i,0], NFUR[j,0])
        if maxNFUR == 0:
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
sortarray = sort(strarray, order=['NL','NFUR'])
sortarray = sortarray[::-1]

ODtrafRank = ones((N,N))*-1
for i in range(N*(N-1)):
    ODtrafRank[sortarray[i][0], sortarray[i][1]]=i

mn, sigma = 16.6, 1.04
s=random.lognormal(mn, sigma, N*(N-1))
s=sort(s)
s=s[::-1]

trafficMat = zeros((N,N))
for i in range(N):
    for j in range(N):
        trafficMat[i,j]=round(s[ODtrafRank[i,j]])

#compute loads
loadmatrix = zeros((N,N))
routetable2 = []
for i in range(N):
    routetable2.append([])

for i in range(N):
    for j in range(N):
        routetable2[i].append([N])

for i in range(N):
    tempweight = weighttable.copy()
    dest = set([i])
    while len(dest)!=N:
        mini=1000
        destnode=-1
        for j in dest:
            for k in range(N):
                if tempweight[j,k]<mini and k not in dest:
                    destnode = k
                    mini = tempweight[j,k]
        for j in dest:
            for k in range(N):
                if tempweight[j,k]==mini and k == destnode:
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

def printpath(nodenum, load):
    if nodelist[nodenum].data==11:
        return
    else:
        childnum = len(nodelist[nodenum].children)
        split = load/childnum
        for tnode in nodelist[nodenum].children:
            if tnode!=N:
                loadmatrix[nodenum, tnode]+=round(split)
                printpath(tnode, split)

for j in range(N):
    for k in range(N+1):
        nodelist[k].child_rst()
    for i in range(N):
        nodelist[i].add_child(routetable2[i][j])
    for i in range(N):
        pth=[-1]*N
        A=[]
        printpath(i,trafficMat[i,j])

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
for i in range(N):
    print loadmatrix[i]
for i in range(N):
    print trafficMat[i]

        
