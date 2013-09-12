clear all
clc
N=11; %%this is the number of nodes in the graph
PathinS=544;
Mysize=PathinS;
fid=fopen('/home/behnaz/ospf_wt_optimizer/sample_topos/matlabtopology','r');
fid2=fopen('/home/behnaz/ospf_wt_optimizer/weights.1','r');
fid3=fopen('/home/behnaz/ospf_wt_optimizer/pathspecifier','r');
fid4=fopen('/home/behnaz/ospf_wt_optimizer/pathpref','r');
adjacencymatrix=fscanf(fid,'%d',[N,N]);
squarew=fscanf(fid2,'%d',[N,N]);
pathspec1=fscanf(fid3,'%d',[N-1,PathinS]);
pathpref1=fscanf(fid4,'%d',[PathinS,PathinS]);
temp=-1*ones(1,PathinS);
pathspec1=[pathspec1;temp];
pathspec1=pathspec1';
PathinS=Mysize;
pathspec=pathspec1(1:Mysize,:);
pathpref=pathpref1(1:Mysize,1:Mysize);
size(pathspec)
%%%%%%%%%%%%%%%%%%%%%%%%%%%delete this
%%%%%%%%%%%%%%%%%%%%%%%%%%%later%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
counter=0;
originalpathins=PathinS;
for i=1:PathinS
   for j=2:N 
    while pathspec(i,j)==0 && pathspec(i,j-1)==4
        
      
        pathspec(i,:)=[];
        pathpref(i,:)=[];
        pathpref(:,i)=[];
        counter=counter+1;
    end
    while pathspec(i,j)==4 &&pathspec(i,j-1)==0
           
            pathspec(i,:)=[];
        pathpref(i,:)=[];
        pathpref(:,i)=[];
        counter=counter+1;
        end
  
  
    while pathspec(i,j)==0 && pathspec(i,j-1)==5
        
        pathspec(i,:)=[];
        pathpref(i,:)=[];
        pathpref(:,i)=[];
        counter=counter+1;
    end
    while pathspec(i,j)==5 &&pathspec(i,j-1)==0
            i=i-1;
            pathspec(i,:)=[];
        pathpref(i,:)=[];
        pathpref(:,i)=[];
        counter=counter+1;
        end
   
   end
   [n,m]=size(pathspec);
   if i>=(n)
       break;
   end
end
PathinS=PathinS-counter;
pathspec(2,:)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fid3=fopen('/home/behnaz/ospf_wt_optimizer/pathspecifier','w');
fid4=fopen('/home/behnaz/ospf_wt_optimizer/pathpref','w');
for i=1:PathinS
    for j=1:PathinS
        fprintf(fid4,'%d\t',pathpref(i,j));
    end
    fprintf(fid4,'\n');
end
for i=1:PathinS
    for j=1:N
        fprintf(fid3,'%d\t',pathspec(i,j));
    end
    fprintf(fid3,'\n');
end
fclose(fid3);
fclose(fid4);
size(pathspec)
size(pathpref)