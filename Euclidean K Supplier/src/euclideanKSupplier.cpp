#include <cstdlib>						// C standard library
#include <cstdio>						// C I/O (for sscanf)
#include <cstring>						// string manipulation
#include <fstream>						// file I/O
#include <ANN/ANN.h>
#include<bits/stdc++.h>					// ANN declarations
#include <chrono>
#include "MicaliVazirani.h"
using namespace std;					// make std:: accessible

void getArgs(int argc, char **argv);			// get command-line arguments

int				k				= 1;			// number of nearest neighbors
int				dim				= 2;			// dimension
double			eps				= 0;			// error bound
int				maxPts			= 100002;			// maximum number of data points

istream*		dataIn			= NULL;			// input for data points
istream*		fdataIn			= NULL;			// input for query points

bool readPt(istream &in, ANNpoint p)			// read point (false on EOF)
{
	for (int i = 0; i < dim; i++) {
		if(!(in >> p[i])) return false;
	}
	return true;
}

void printPt(ostream &out, ANNpoint p)			// print point
{
	out << "(" << p[0];
	for (int i = 1; i < dim; i++) {
		out << ", " << p[i];
	}
	out << ")\n";
}

int main(int argc, char **argv)
{
	int					    nPts;
    int                     nGpts;
    int                     f;
    double                  low = 0;
    double                  high = 100.0;                  
    double                  L ;
    int                     iter =0;
    int                     max =0;     		    
	ANNpointArray		    dataPts;
    ANNpointArray		    fdataPts;				
    ANNpoint                temp;				    
	ANNidxArray			    nnIdx;					
	ANNdistArray		    dists;					
	ANNkd_tree*			    kdTree;					
    ANNpointArray           ptsGrph;
    map<int,vector<int>>    graph;
    map<pair<int,int>,int>  corsFacility;
                                                				
	dataPts  = annAllocPts(maxPts, dim);
    ptsGrph  = annAllocPts(maxPts, dim);
    fdataPts = annAllocPts(maxPts, dim);			
	nnIdx = new ANNidx[2];						
	dists = new ANNdist[2];						

	nPts  = 0;
    nGpts = 0;
    f=0;									

    getArgs(argc, argv);

	while (readPt(*dataIn, dataPts[nPts])) {
		nPts++;
	}

	while (readPt(*fdataIn, fdataPts[f])) {
		f++;
	}

    auto start = chrono::high_resolution_clock::now();
    do {

        graph.clear();
        corsFacility.clear();
        L = (high + low)/2;
        nGpts = 0 ;
        //Computing P of G
    for(int i=0;i<nPts;i++){
        if(i==0){
            ptsGrph[i] = dataPts[i];
            kdTree = new ANNkd_tree(ptsGrph,2,dim);
            nGpts++;        
            }
        else{
            kdTree -> annkSearch(dataPts[i],1,nnIdx,dists,eps);
            // cout<< sqrt(dists[0]) << " ";
            if(sqrt(dists[0]) > (sqrt(3)*L)){
                ptsGrph[nGpts] = dataPts[i];
                kdTree -> ~ANNkd_tree();
                kdTree = new ANNkd_tree(ptsGrph,nGpts+1,dim);
                nGpts++;
            }
        }
    }

    //Computing E of G
    for(int i=0;i<f;i++){
        kdTree -> annkSearch(fdataPts[i],2,nnIdx,dists,eps);  
        if(sqrt(dists[0])<=L&&sqrt(dists[1])>L){
            if(graph[nnIdx[0]].empty())
                graph[nnIdx[0]].push_back(nnIdx[0]);
            else
            {
             if(find(graph[nnIdx[0]].begin(),graph[nnIdx[0]].end(),nnIdx[0]) == graph[nnIdx[0]].end()) 
                graph[nnIdx[0]].push_back(nnIdx[0]);       
            }
            corsFacility[{nnIdx[0],nnIdx[0]}] = i;
        }
        else if(sqrt(dists[0])<=L&&sqrt(dists[1])<=L){
            if(graph[nnIdx[0]].empty())
                graph[nnIdx[0]].push_back(nnIdx[1]);
            else
            {
             if(find(graph[nnIdx[0]].begin(),graph[nnIdx[0]].end(),nnIdx[1]) == graph[nnIdx[0]].end()) 
                graph[nnIdx[0]].push_back(nnIdx[1]);       
            }
            if(graph[nnIdx[1]].empty())
                graph[nnIdx[1]].push_back(nnIdx[0]);
            else
            {
             if(find(graph[nnIdx[1]].begin(),graph[nnIdx[1]].end(),nnIdx[0]) == graph[nnIdx[1]].end()) 
                graph[nnIdx[1]].push_back(nnIdx[0]);       
            }
            corsFacility[{nnIdx[0],nnIdx[1]}] = i;
            corsFacility[{nnIdx[1],nnIdx[0]}] = i;
        //    cout<< i << " ";
        } 
    }

    if(nGpts > graph.size())
        low = L-1 ;
    if(nGpts <= graph.size())
        high = L+1 ; 

    iter++;

    if(iter==1000)
        if(nGpts>k){
            low = 2*L;
            iter = 0;
        }
    } while(iter<=1000);
    
    // finding edge cover
    MicaliVazirani mv = MicaliVazirani(graph);
    map<pair<int, int>, pair<int, int>> maxmatch = mv.general_matching_algorithm();
    int visited[nGpts];
    for(auto elem: maxmatch){
        visited[elem.first.first] =1;
        visited[elem.second.first] =1;
    }
    for(int i=0;i<nGpts;i++){
        if(visited[nGpts]==0)
        maxmatch[{i,0}] = make_pair(graph[i][0],0);
    }

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << " time :" << duration.count() << endl;
    //self loops' facilities
    cout << "Facilities  : ";
    for(auto elem:corsFacility){
        if(elem.first.first == elem.first.second)
        cout << elem.second << " ";
    }
    //edges' facilities
    for(auto elem: maxmatch){
        if(corsFacility[{elem.first.first,elem.second.first}]!=-1){
            cout<< corsFacility[{elem.first.first,elem.second.first}] << " ";
            corsFacility[{elem.second.first,elem.first.first}] = -1;
        }
    }
    cout << endl ;
    cout << "L : " << L << endl;
    delete [] nnIdx;							
    delete [] dists;
    delete kdTree;
	annClose();									

	return EXIT_SUCCESS;
}


void getArgs(int argc, char **argv)
{
	static ifstream dataStream;					// data file stream
	static ifstream queryStream;				// query file stream

	if (argc <= 1) {							// no arguments
		exit(0);
	}
	int i = 0;
    k = atoi(argv[++i]);
    dataStream.open(argv[++i], ios::in);
    dataIn = &dataStream;
    queryStream.open(argv[++i], ios::in);
    fdataIn = &queryStream;
	
}