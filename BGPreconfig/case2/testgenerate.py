from numpy import *
from random import *
from math import *

N=20
alpha=1
beta=0.5
PathinS=40

nodes = zeros((N, 2))
for i in range(N):
    for j in range(2):
        nodes[i,j]=uniform(0,1)

distances = zeros((N,N))
for i in range(N):
    for j in range(1, i+1):
        if i!=j:
            distances[i,j]=sqrt((nodes[i,0]-nodes[j,0])**2+(nodes[i,1]-nodes[j,1])**2)
        else:
            distances[i,j]=0
sortedlist=[]
for i in range(N):
    for j in range(N):
        sortedlist.append(distances[i,j])
sortedlist.sort()
i=0
sortedl=[]
while sortedlist[i]==0:
    i+=1

while i<len(sortedlist):
    sortedl.append(sortedlist[i])
    i+=1

L=sqrt(2)*sortedl[len(sortedl)-1]
A=zeros((N,N))
counter=0
for i in range(N):
    for j in range(1,i+1):
        if i!=j:
            p=uniform(0,1)
            if p<=alpha*exp(-distances[i,j]/(beta*L)):
                A[i,j]=1
                A[j,i]=1
                counter+=1
print distances[2]
#for i in range(N):
#    print A[i].T

            


