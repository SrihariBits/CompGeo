#include<bits/stdc++.h>

using namespace std;

int main()
{
    int f = no_of_facilities;
    ANN_point facilities[f];    // all facilities
    ANN_point clients[c];   // clients in the new graph
    map<ANN_point,int> transition1;  // used to get index of an ANN_point
    for(int i=0;i<c;i++)
        transition1[clients[i]] = i;
    
    int n = selected_vertices.size();   // no of clients in the new graph
    map<pair<int,int>,ANN_point> facilityEdge;  // tells which edge is corresponding to which facility - one to one correspondence b/w edges and facilities
    map<int,vector<int>> graph; // adjacency list representation of the graph
    for(int i=0;i<f;i++)
    {
        ANN_point facility = facilities[i];
        ANN_point u = nearest_neighbour(facility,clients);
        ANN_point v = second_nearest_neighbour(facility,clients);
        int indexU = transition[u];
        int indexV = transition[v];
        if(dist(u,facility)<=(1+epsilon)*L and dist(v,facility)>(1+epsilon)*L)
        {
            facilityEdge[{indexU,indexU}] = facility;
            graph[indexU].push_back(indexU);
        }
        else if(dist(u,facility)<=(1+epsilon)*L and dist(v,facility)<=(1+epsilon)*L)
        {
            facilityEdge[{indexU,indexV}] = facility;
            facilityEdge[{indexV,indexU}] = facility;
            graph[indexU].push_back(indexV);
            graph[indexV].push_back(indexU);
        }
    }
}