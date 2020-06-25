#include <cstdlib>						// C standard library
#include <cstdio>						// C I/O (for sscanf)
#include <cstring>						// string manipulation
#include <fstream>						// file I/O
#include <ANN/ANN.h>
#include<bits/stdc++.h>					// ANN declarations
#include <chrono>
using namespace std;					// make std:: accessible

void getArgs(int argc, char **argv);			// get command-line arguments

int				k				= 1;			// number of nearest neighbors
int				dim				= 2;			// dimension
double			eps				= 0;			// error bound
int				maxPts			= 1000;			// maximum number of data points

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
    double                  L = 0.001;     		    
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


    //Computing P of G
    for(int i=0;i<nPts;i++){
        if(i==0){
            ptsGrph[i] = dataPts[i];
            kdTree = new ANNkd_tree(ptsGrph,i+1,dim);
            nGpts++;        
            }
        else{
            kdTree -> annkSearch(dataPts[i],1,nnIdx,dists,eps);
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
            graph[nnIdx[0]].push_back(nnIdx[0]);
            corsFacility[{nnIdx[0],nnIdx[0]}] = i;
        }
        else if(sqrt(dists[0])<=L&&sqrt(dists[1])<=L){
            graph[nnIdx[0]].push_back(nnIdx[1]);
            graph[nnIdx[1]].push_back(nnIdx[0]);
            corsFacility[{nnIdx[0],nnIdx[1]}] = i;
            corsFacility[{nnIdx[1],nnIdx[0]}] = i;
        } 
    }


    delete [] nnIdx;							// clean things up
    delete [] dists;
    delete kdTree;
	annClose();									// done with ANN

	return EXIT_SUCCESS;
}

//----------------------------------------------------------------------
//	getArgs - get command line arguments
//----------------------------------------------------------------------

void getArgs(int argc, char **argv)
{
	static ifstream dataStream;					// data file stream
	static ifstream queryStream;				// query file stream

	if (argc <= 1) {							// no arguments
		cerr << "Usage:\n\n"
		<< "  ann_sample [-d dim] [-max m] [-nn k] [-e eps] [-df data]"
		   " [-qf query]\n\n"
		<< "  where:\n"
		<< "    dim      dimension of the space (default = 2)\n"
		<< "    m        maximum number of data points (default = 1000)\n"
		<< "    k        number of nearest neighbors per query (default 1)\n"
		<< "    eps      the error bound (default = 0.0)\n"
		<< "    data     name of file containing data points\n"
		<< "    query    name of file containing query points\n\n"
		<< " Results are sent to the standard output.\n"
		<< "\n"
		<< " To run this demo use:\n"
		<< "    ann_sample -df data.pts -qf query.pts\n";
		exit(0);
	}
	int i = 1;
	while (i < argc) {							// read arguments
		if (!strcmp(argv[i], "-d")) {			// -d option
			dim = atoi(argv[++i]);				// get dimension to dump
		}
		else if (!strcmp(argv[i], "-max")) {	// -max option
			maxPts = atoi(argv[++i]);			// get max number of points
		}
		else if (!strcmp(argv[i], "-nn")) {		// -nn option
			k = atoi(argv[++i]);				// get number of near neighbors
		}
		else if (!strcmp(argv[i], "-e")) {		// -e option
			sscanf(argv[++i], "%lf", &eps);		// get error bound
		}
		else if (!strcmp(argv[i], "-df")) {		// -df option
			dataStream.open(argv[++i], ios::in);// open data file
			if (!dataStream) {
				cerr << "Cannot open data file\n";
				exit(1);
			}
			dataIn = &dataStream;				// make this the data stream
		}
		else if (!strcmp(argv[i], "-qf")) {		// -qf option
			queryStream.open(argv[++i], ios::in);// open query file
			if (!queryStream) {
				cerr << "Cannot open query file\n";
				exit(1);
			}
			fdataIn = &queryStream;			// make this query stream
		}
		else {									// illegal syntax
			cerr << "Unrecognized option.\n";
			exit(1);
		}
		i++;
	}
	if (dataIn == NULL || fdataIn == NULL) {
		cerr << "-df and -qf options must be specified\n";
		exit(1);
	}
}