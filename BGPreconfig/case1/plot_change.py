from matplotlib.pyplot import *

fileHandle = open('change')
congestion = fileHandle.readlines()
fileHandle.close()

for i in range(len(congestion)):
    congestion[i] = congestion[i].split()
    for j in range(len(congestion[i])):
        congestion[i][j] = float(congestion[i][j])

y=[]
x=[]
for i in range(len(congestion)):
    y.append(congestion[i][0])
    x.append(100*(i+1))
plot(x,y)
show()

