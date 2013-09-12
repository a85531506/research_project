from cvxopt import *
from random import *

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

fileHandle = open('traffic')
traffic = fileHandle.readlines()
fileHandle.close()

fileHandle = open('capacity')
capacity = fileHandle.readlines()
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
    specifier[i]=specifier[i].split()
    for j in range(len(specifier[i])):
        specifier[i][j] = int(specifier[i][j])

for i in range(len(preference)):
    preference[i] = preference[i].split()
    for j in range(len(preference[i])):
        preference[i][j] = int(preference[i][j])

for i in range(len(traffic)):
    traffic[i]=traffic[i].split()
    for j in range(len(traffic[i])):
        traffic[i][j] = round(float(traffic[i][j]))

for i in range(len(capacity)):
    capacity[i] = capacity[i].split()
    for j in range(len(capacity[i])):
        capacity[i][j] = int(capacity[i][j])

N=len(topology)
Pathnum=len(specifier)
adjacencymatrix=matrix(topology).T
pathspec = matrix(specifier).T
squarew = matrix(weight).T
pathpref = matrix(preference).T
traffic = matrix(traffic).T
capacity = matrix(capacity).T

indexmat = matrix(-1, (N,N))
indexnum = 0
weightorigin=[]
for i in range(N):
    for j in range(N):
        if adjacencymatrix[i,j]==1:
            indexmat[i,j]=indexnum
            indexnum+=1
            weightorigin.append(squarew[i,j])
weightorigin = matrix(weightorigin)
weightnum = len(weightorigin)
coeffmat=[]
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
                    coefflist[indexmat[xnum, ynum]]=1.0
                else:
                    break
            path2 = pathspec[j,:]
            for k in range(N-2):
                if path2[0,k+1]!=-1:
                    xnum = path2[0,k]
                    ynum = path2[0,k+1]
                    coefflist[indexmat[xnum, ynum]]=-1.0
                else:
                    break
            coeffmat.append(coefflist)
            prefnum+=1
coeffmat = matrix(coeffmat).T
zeromat = matrix(0, (prefnum, 1))
onemat = matrix(-1.0, (prefnum, 1))

A=spmatrix(1.0, range(weightnum), range(weightnum))
m,n=A.size
I1 = matrix(-1, (n, 1))
I2 = spmatrix(-1, range(n), range(n))
total = range(prefnum)

constnum = 64*100
congestion=[]
utilavglist=[]
utilmaxlist=[]
#capacity = 1000000000

class Node(object):
    def __init__(self, data):
        self.data=data
        self.children=[]
    def add_child(self, obj):
        self.children.extend(obj)
    def child_rst(self):
        self.children=[]

def printpath(nodenum, load):
    if nodelist[nodenum].data==11:
        return
    else:
        childnum = len(nodelist[nodenum].children)
        split = load/childnum
        for tnode in nodelist[nodenum].children:
            if tnode!=11:
                loadmatrix[nodenum, tnode]+=split
                printpath(tnode, split)

def cost(load, cap):
    phei = load/cap
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

for i in range(500):
    slic = sample(total, constnum)
    slic.sort()
    testcoeffmat = matrix(-1.0, (constnum, weightnum))
    testonemat = matrix(-1.0, (constnum, 1))
    for k in range(constnum):
        testcoeffmat[k,:]=coeffmat[slic[k],:]
        testonemat[k,:]=onemat[slic[k],:]
    h=matrix([testonemat, I1])
    G=matrix([testcoeffmat, I2])
    sol = solvers.qp(A.T*A, -A.T*weightorigin, G, h)
    newweight=[]
    for i in range(len(sol['x'])):
        newweight.append(int(sol['x'][i]))

    loadmatrix = matrix(0.0, (N, N))
    routetable=[]
    weighttable = matrix(900, (N, N))
    #weight = array(newweight)
    utilmatrix = matrix(0.0, (N, N))
    costmatrix=matrix(0, (N,N))

    count = 0
    for i2 in range(N):
        for j2 in range(N):
            if adjacencymatrix[i2, j2]==1:
                weighttable[i2, j2]=newweight[count]
                count +=1
    for i2 in range(N):
        routetable.append([])
    for i2 in range(N):
        for j2 in range(N):
            routetable[i2].append([N])

    for i2 in range(N):
        tempweight = matrix(weighttable)
        dest = set([i2])
        while len(dest)!=N:
            mini=1000
            destnode= -1
            for j2 in dest:
                for k2 in range(N):
                    if tempweight[j2, k2]<mini and k2 not in dest:
                        destnode = k2
                        mini = tempweight[j2, k2]
            for j2 in dest:
                for k2 in range(N):
                    if tempweight[j2, k2] == mini and k2 == destnode:
                        if j2==i2:
                            routetable[i2][destnode].append(k2)
                        else:
                            routetable[i2][destnode].extend(routetable[i2][j2][1:])
            for j2 in range(N):
                tempweight[destnode, j2]+=mini
            dest.add(destnode)
            
    for i2 in range(N):
        for j2 in range(N):
            if i2!=j2:
                routetable[i2][j2] = list(set(routetable[i2][j2][1:]))

    nodelist = [Node]*(N+1)
    for i2 in range(N+1):
        nodelist[i2]=Node(i2)

    for j2 in range(N):
        for k2 in range(N+1):
            nodelist[k2].child_rst()
        for i2 in range(N):
            nodelist[i2].add_child(routetable[i2][j2])
        for i2 in range(N):
            printpath(i2, traffic[i2,j2])

    for i2 in range(N):
        for j2 in range(N):
            if capacity[i2, j2]==0:
                continue
                capacity[i2, j2]=1
            utilmatrix[i2, j2] = loadmatrix[i2, j2]/(capacity[i2, j2]*3.0)
            costmatrix[i2, j2] = cost(loadmatrix[i2,j2], capacity[i2,j2]*3)
    utilmax=max(utilmatrix)
    utilavg = sum(utilmatrix)/(weightnum)
    costsum=sum(costmatrix)
    congestion.append(costsum)
    utilavglist.append(utilavg)
    utilmaxlist.append(utilmax)

avgcong = sum(congestion)/len(congestion)
avgutil=sum(utilavglist)/len(utilavglist)
avgmax=sum(utilmaxlist)/len(utilmaxlist)
print avgcong
print avgutil
print avgmax
