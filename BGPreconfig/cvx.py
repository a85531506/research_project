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

N = len(topology)
Pathnum=len(specifier)
adjacencymatrix = matrix(topology).T
pathspec = matrix(specifier).T
squarew = matrix(weight).T
pathpref = matrix(preference).T

indexmat=matrix(-1,(N,N))
indexmat=matrix(indexmat)
indexnum=0
weightorigin=[]
for i in range(N):
    for j in range(N):
        if adjacencymatrix[i,j]==1:
            indexmat[i,j]=indexnum
            indexnum=indexnum+1
            weightorigin.append(squarew[i,j])
weightorigin = matrix(weightorigin)
weightnum=len(weightorigin)
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
            prefnum=prefnum+1
            
coeffmat=matrix(coeffmat).T
zeromat = matrix(0,(prefnum,1))
onemat = matrix(-1.0,(prefnum,1))        



A=spmatrix(1.0,range(weightnum),range(weightnum))
m,n=A.size
I1=matrix(-1,(n,1))
h=matrix([onemat,I1])

I2=spmatrix(-1,range(n),range(n))
G=matrix([coeffmat,I2])
sol=solvers.qp(A.T*A,-A.T*weightorigin,G,h)
newweight=[]
for i in range(len(sol['x'])):
    newweight.append(int(sol['x'][i]))
print newweight
    






