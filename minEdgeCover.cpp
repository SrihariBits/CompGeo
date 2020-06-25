#include<bits/stdc++.h>

int main()
{
    int visited[n];
    vector<pair<int,int>> matchedEdges;   // a vector of edges (pair of indices of nodes) which are matched
    for(int i=0;i<matchedEdges.size();i++)
    {
        visited[matchedEdges[0]]=1;
        visited[matchedEdges[1]]=1;
    }
    for(int i=0;i<n;i++)
        if(visited[i]==0)
            matchedEdges.push_back({i,graph[i][0]);
    
    for(int i=0;i<matchedEdges.size();i++)
    {
        int u=matchedEdges[i][0], v=matchedEdges[i][1];
        cout<<facilityEdge[{u,v}]<<" ";
    }
}