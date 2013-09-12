from matplotlib.pyplot import *

fileHandle = open('time')
time = fileHandle.readlines()
fileHandle.close()

for i in range(len(time)):
    time[i] = time[i].split()
    for j in range(len(time[i])):
        time[i][j] = float(time[i][j])
y=[]
x=[]
for i in range(len(time)):
    for j in range(len(time[0])):
        y.append(time[i][j])
        x.append(100*(i+1))
plot(x,y,'.')
show()
        


