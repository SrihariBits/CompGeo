#include <bits/stdc++.h>
using namespace std;

#define UNERASED false
#define ERASED true
#define UNVISITED false
#define VISITED true
#define LEFT -1
#define UNMARKED 0
#define RIGHT 1
#define UNUSED false
#define USED true
int INFNITY = INT_MAX;
pair<int, int> None = {INT_MIN, INT_MIN};
class Bloom
{

public:
    bool defined = false;
    pair<pair<int, int>, pair<int, int>> peaks = {None, None};
    pair<int, int> base = None;
    bool isDefined()
    {
        return defined;
    }
    bool setDefined()
    {
        defined = true;
    }
};

Bloom none;

class DfsInfo
{
public: //try to move this down
    pair<int, int> s;
    pair<int, int> t;
    pair<int, int> vL;
    pair<int, int> vR;
    pair<int, int> dcv;
    pair<int, int> barrier;

    DfsInfo(pair<int, int> ss, pair<int, int> tt, pair<int, int> vLL, pair<int, int> vRR, pair<int, int> dcvv, pair<int, int> barrierr)
    {
        s = ss;
        t = tt;
        vL = vLL;
        vR = vRR;
        dcv = dcvv;
        barrier = barrierr;
    }
};

class MicaliVazirani
{
public:
    vector<pair<int, int>> nodes;
    map<pair<int, int>, vector<pair<int, int>>> neighbors; //all neightbors to a node
    map<pair<int, int>, map<pair<int, int>, map<string, bool>>> edges;
    MicaliVazirani(vector<pair<int, int>> nodess, map<pair<int, int>, vector<pair<int, int>>> neighborss, map<pair<int, int>, map<pair<int, int>, map<string, bool>>> edgess)
    {
        nodes = nodess;
        neighbors = neighborss;
        edges = edgess;
    }
    map<pair<int, int>, int> nodeEvenLevel;
    map<pair<int, int>, int> nodeOddLevel;
    map<pair<int, int>, Bloom> nodeBloom;
    map<pair<int, int>, vector<pair<int, int>>> nodePredecessors;
    map<pair<int, int>, vector<pair<int, int>>> nodeSuccessors;
    map<pair<int, int>, vector<pair<int, int>>> nodeAnomalies;
    map<pair<int, int>, int> nodeCount;
    map<pair<int, int>, bool> nodeErase;
    map<pair<int, int>, bool> nodeVisit;
    map<pair<int, int>, int> nodeMark;
    map<pair<int, int>, pair<int, int>> nodeParent;

    vector<pair<int, int>> bloomNodes;
    map<int, vector<pair<int, int>>> candidates;
    map<int, set<pair<pair<int, int>, pair<int, int>>, less<pair<pair<int, int>, pair<int, int>>>>> bridges;

    map<pair<int, int>, pair<int, int>> mate; //note all instances of != (none cases in python)

    map<pair<int, int>, pair<int, int>> max_cardinality_matching()
    {
        if (nodes.size() == 0)
            return {};

        bool augmented = true;
        while (augmented)
        {
            for (auto v : nodes)
            {
                nodeEvenLevel[v] = INFNITY;
                nodeOddLevel[v] = INFNITY;
                nodeBloom[v] = none;
                nodePredecessors[v].clear();
                nodeSuccessors[v].clear();
                nodeAnomalies[v].clear();
                nodeCount[v] = 0;
                nodeErase[v] = UNERASED;
                nodeVisit[v] = UNVISITED;
                nodeMark[v] = UNMARKED;
                nodeParent[v] = None;
            }

            for (auto e : edges)
            {
                for (auto v : e.second)
                {
                    if (e.first == v.first)
                        continue;
                    edges[e.first][v.first]["use"] = UNUSED;
                    edges[e.first][v.first]["visit"] = UNVISITED;
                }
            }

            for (int i = 0; i < nodes.size() + 1; ++i) //+1?
            {
                candidates[i] = {};
                bridges[i] = {};
            }
            augmented = search();
        }

        return mate;
    }

    bool search()
    {
        int i = 0;
        for (auto v : nodes)
        {
            if (mate.find(v) == mate.end()) //exposed vertex
            {
                nodeEvenLevel[v] = 0;
                candidates[0].push_back(v);
            }
        }
        bool augmented = false;
        while (i < nodes.size() + 1 and !augmented) //check empty candidiates?
        {
            if (i % 2 == 0)
            {
                for (auto v : candidates[i])
                {
                    for (auto u : neighbors[v])
                    {
                        if (u == v)
                            continue;
                        if ((mate.find(v) == mate.end() or mate[v] != u) and nodeErase[u] == UNERASED)
                        {
                            if (nodeEvenLevel[u] < INFNITY)
                            {
                                int j = (nodeEvenLevel[u] + nodeEvenLevel[v]) / 2;
                                if (u <= v)
                                    bridges[j].insert({u, v});
                                else
                                    bridges[j].insert({v, u});
                            }
                            else
                            {
                                if (nodeOddLevel[u] == INFNITY)
                                    nodeOddLevel[u] = i + 1;
                                if (nodeOddLevel[u] == i + 1)
                                {
                                    nodeCount[u] += 1;
                                    nodePredecessors[u].push_back(v);
                                    nodeSuccessors[v].push_back(u);
                                    candidates[i + 1].push_back(u);
                                }
                                else if (nodeOddLevel[u] < i)
                                {
                                    nodeAnomalies[u].push_back(v);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (auto v : candidates[i])
                {
                    if (!nodeBloom[v].isDefined())
                    {
                        if (mate.find(v) == mate.end())
                        {
                            cout << "Improper Input"; //case should not happen!
                            return true;
                        }
                        pair<int, int> u = mate[v];
                        if (nodeOddLevel[u] < INFNITY)
                        {
                            int j = (nodeOddLevel[u] + nodeOddLevel[v]) / 2;
                            if (u <= v)
                                bridges[j].insert({u, v});
                            else
                                bridges[j].insert({v, u});
                        }
                        else if (nodeEvenLevel[u] == INFNITY)
                        {
                            nodePredecessors[u] = {v};
                            nodeSuccessors[v] = {u};
                            nodeCount[u] = 1;
                            nodeEvenLevel[u] = i + 1;
                            candidates[i + 1].push_back(u);
                        }
                    }
                }
            }

            for (auto pairs : bridges[i])
            {
                pair<int, int> s = pairs.first;
                pair<int, int> t = pairs.second;
                if (nodeErase[s] == UNERASED and nodeErase[t] == UNERASED)
                {
                    augmented = augmentBlossom(s, t, i);
                }
            }

            ++i;
        }
        return augmented;
    }

    bool augmentBlossom(pair<int, int> s, pair<int, int> t, int i)
    {
        bool foundBloom = false;
        bool augmented = false;

        pair<int, int> vL = nodeBloom[s].isDefined() ? baseStar(s) : s;
        pair<int, int> vR = nodeBloom[t].isDefined() ? baseStar(t) : t;

        if (vL == vR)
            return false;

        if (nodeBloom[s].isDefined())
            nodeParent[vL] = s;
        if (nodeBloom[t].isDefined())
            nodeParent[vR] = t;

        nodeMark[vL] = LEFT;
        nodeMark[vR] = RIGHT;
        bloomNodes.push_back(vL);
        bloomNodes.push_back(vR);

        DfsInfo dfsInfo(s, t, vL, vR, None, vR);

        while (!foundBloom and !augmented)
        {
            if (dfsInfo.vL == None or dfsInfo.vR == None)
                return false;
            int level_vL = min(nodeEvenLevel[dfsInfo.vL], nodeOddLevel[dfsInfo.vL]);
            int level_vR = min(nodeEvenLevel[dfsInfo.vR], nodeOddLevel[dfsInfo.vR]);
            if (mate.find(dfsInfo.vL) == mate.end() and mate.find(dfsInfo.vR) == mate.end()) //both exposed
            {
                vector<pair<int, int>> pathL = findPath(dfsInfo.s, dfsInfo.vL, none);
                vector<pair<int, int>> pathR = findPath(dfsInfo.t, dfsInfo.vR, none);
                vector<pair<int, int>> path = connectPath(pathL, pathR, dfsInfo.s, dfsInfo.t);
                augmentMatching(dfsInfo.vL, dfsInfo.vR);
                erasePath(path);
                augmented = true;
                break;
            }
            else if (level_vL >= level_vR)
            {
                foundBloom = leftDfs(dfsInfo);
            }
            else
            {
                foundBloom = rightDfs(dfsInfo);
            }
        }

        if (foundBloom and dfsInfo.dcv != None)
        {
            nodeMark[dfsInfo.dcv] = UNMARKED;
            Bloom b;
            b.peaks = {dfsInfo.s, dfsInfo.t};
            b.base = dfsInfo.dcv;
            b.setDefined();

            for (auto y : bloomNodes)
            {
                if (nodeMark[y] == UNMARKED or nodeBloom[y].isDefined())
                    continue;
                nodeBloom[y] = b;

                int level_y = min(nodeEvenLevel[y], nodeOddLevel[y]);
                if (level_y % 2 == 0) //outer
                    nodeOddLevel[y] = 2 * i + 1 - nodeEvenLevel[y];
                else //inner
                {
                    nodeEvenLevel[y] = 2 * i + 1 - nodeOddLevel[y];
                    candidates[nodeEvenLevel[y]].push_back(y);
                    for (auto z : nodeAnomalies[y])
                    {
                        int j = (nodeEvenLevel[y] + nodeEvenLevel[z]) / 2;
                        if (y <= z)
                            bridges[j].insert({y, z});
                        else
                            bridges[j].insert({z, y});
                        edges[y][z]["use"] = USED;
                    }
                }
            }
        }
        bloomNodes.clear();
        return augmented;
    }

    vector<pair<int, int>> connectPath(vector<pair<int, int>> &pathL, vector<pair<int, int>> &pathR, pair<int, int> s, pair<int, int> t)
    {
        bool reverseL = s == pathL[0] ? true : false;
        bool reverseR = t == pathR[pathR.size() - 1] ? true : false;

        if (reverseL)
        {
            nodeParent[pathL[0]] = None;
            pair<int, int> prevv = None;
            pair<int, int> currentv = pathL[pathL.size() - 1];
            pair<int, int> nextv = None;
            while (currentv != None)
            {
                nextv = nodeParent[currentv];
                nodeParent[currentv] = prevv;
                prevv = currentv;
                currentv = nextv;
            }
            reverse(pathL.begin(), pathL.end());
        }

        if (reverseR)
        {
            nodeParent[pathR[0]] = None;
            pair<int, int> prevv = None;
            pair<int, int> currentv = pathR[pathR.size() - 1];
            pair<int, int> nextv = None;
            while (currentv != None)
            {
                nextv = nodeParent[currentv];
                nodeParent[currentv] = prevv;
                prevv = currentv;
                currentv = nextv;
            }
            reverse(pathR.begin(), pathR.end());
        }

        vector<pair<int, int>> path;
        path.insert(path.begin(), pathL.begin(), pathL.end());
        path.insert(path.end(), pathR.begin(), pathR.end());

        nodeParent[pathR[0]] = pathL[pathL.size() - 1];
        return path;
    }

    void augmentMatching(pair<int, int> lv, pair<int, int> rv)
    {
        pair<int, int> firstv = rv;
        pair<int, int> secondv = None;
        while (firstv != lv)
        {
            secondv = nodeParent[firstv];
            if (mate.find(secondv) == mate.end() or mate[secondv] != firstv)
            {
                mate[firstv] = secondv;
                mate[secondv] = firstv;
            }
            firstv = secondv;
        }
    }

    bool leftDfs(DfsInfo &dfsInfo)
    {
        for (auto u : nodePredecessors[dfsInfo.vL])
        {
            if (edges[dfsInfo.vL][u]["use"] == USED or nodeErase[u] == ERASED)
                continue;

            edges[dfsInfo.vL][u]["use"] = USED;

            if (nodeBloom[u].isDefined())
                u = baseStar(u);

            if (nodeMark[u] == UNMARKED)
            {
                nodeMark[u] = LEFT;
                nodeParent[u] = dfsInfo.vL;
                dfsInfo.vL = u;
                bloomNodes.push_back(u);
                return false;
            }
            else if (u == dfsInfo.vR) //required?
            {
                dfsInfo.dcv = u;
            }
        }

        if (dfsInfo.vL == dfsInfo.s)
            return true;
        else if (nodeParent[dfsInfo.vL] != None)
        {
            dfsInfo.vL = nodeParent[dfsInfo.vL];
        }

        return false;
    }

    bool rightDfs(DfsInfo &dfsInfo)
    {
        for (auto u : nodePredecessors[dfsInfo.vR])
        {
            if (edges[dfsInfo.vR][u]["use"] == USED or nodeErase[u] == ERASED)
                continue;

            edges[dfsInfo.vR][u]["use"] = USED;

            if (nodeBloom[u].isDefined())
                u = baseStar(u);

            if (nodeMark[u] == UNMARKED)
            {
                nodeMark[u] = RIGHT;
                nodeParent[u] = dfsInfo.vR;
                dfsInfo.vR = u;
                bloomNodes.push_back(u);
                return false;
            }
            else if (u == dfsInfo.vL)
            {
                dfsInfo.dcv = u;
            }
        }

        if (dfsInfo.vR == dfsInfo.barrier)
        {
            dfsInfo.vR = dfsInfo.dcv;
            dfsInfo.barrier = dfsInfo.dcv;
            nodeMark[dfsInfo.vR] = RIGHT;
            if (nodeParent[dfsInfo.vL] != None)
            {
                dfsInfo.vL = nodeParent[dfsInfo.vL];
            }
        }
        else if (nodeParent[dfsInfo.vR] != None)
        {
            dfsInfo.vR = nodeParent[dfsInfo.vR];
        }

        return false;
    }

    void erasePath(vector<pair<int, int>> &path)
    {
        while (path.size() > 0)
        {
            pair<int, int> y = path[path.size() - 1];
            path.pop_back();
            nodeErase[y] = ERASED;

            for (auto z : nodeSuccessors[y])
            {
                if (nodeErase[z] == UNERASED)
                {
                    nodeCount[z] -= 1;
                    if (nodeCount[z] == 0)
                        path.push_back(z);
                }
            }
        }
    }

    vector<pair<int, int>> findPath(pair<int, int> high, pair<int, int> low, Bloom b)
    {
        int level_high = min(nodeEvenLevel[high], nodeOddLevel[high]);
        int level_low = min(nodeEvenLevel[low], nodeOddLevel[low]);
        if (high == low)
            return {high};

        vector<pair<int, int>> path;
        pair<int, int> v = high;
        pair<int, int> u = high;
        while (u != low)
        {

            bool hasUnvisitedPredecessor = false;

            for (auto p : nodePredecessors[v])
            {
                if (edges[p][v]["visit"] == UNVISITED)
                {
                    hasUnvisitedPredecessor = true;
                    if (!nodeBloom[v].isDefined() or (nodeBloom[v].peaks == b.peaks and nodeBloom[v].base == b.base))
                    {
                        edges[p][v]["visit"] = VISITED;
                        u = p;
                    }
                    else
                    {
                        u = nodeBloom[v].base;
                    }
                    break;
                }
            }

            if (!hasUnvisitedPredecessor)
            {
                v = nodeParent[v];
            }
            else
            {
                int level_u = min(nodeEvenLevel[u], nodeOddLevel[u]);
                if (nodeErase[u] == UNERASED and level_u >= level_low and (u == low or (nodeVisit[u] == UNVISITED and (nodeMark[u] == nodeMark[high] != UNMARKED or (nodeBloom[u].isDefined() and (nodeBloom[u].peaks != b.peaks and nodeBloom[u].base != b.base))))))
                {
                    nodeVisit[u] = VISITED;
                    nodeParent[u] = v;
                    v = u;
                }
            }
        }
        while (u != high)
        {
            path.push_back(u);
            u = nodeParent[u];
        }
        path.push_back(u);
        reverse(path.begin(), path.end());

        int j = 0;
        while (j < path.size() - 1)
        {
            pair<int, int> xj = path[j];

            if (nodeBloom[xj].isDefined() and (nodeBloom[xj].peaks != b.peaks and nodeBloom[xj].base != b.base))
            {
                nodeVisit[xj] = UNVISITED;
                vector<pair<int, int>> temp = openBloom(xj);
                int pathLength = temp.size();
                path.erase(path.begin() + j, path.begin() + j + 2);
                path.insert(path.begin() + j, temp.begin(), temp.end());
                nodeParent[xj] = j > 0 ? path[j - 1] : None;
                j += pathLength - 1;
            }
            ++j;
        }
        return path;
    }

    vector<pair<int, int>>
    openBloom(pair<int, int> x)
    {
        Bloom bloom = nodeBloom[x];
        pair<int, int> base = bloom.base;
        int level_x = min(nodeEvenLevel[x], nodeOddLevel[x]);
        vector<pair<int, int>> path;

        if (level_x % 2 == 0) //outer
        {
            path = findPath(x, base, bloom);
        }
        else
        {
            pair<int, int> leftPeak = bloom.peaks.first;
            pair<int, int> rightPeak = bloom.peaks.second;

            if (nodeMark[x] == LEFT)
            {
                vector<pair<int, int>> pathLeft = findPath(leftPeak, x, bloom);
                vector<pair<int, int>>
                    pathRight = findPath(rightPeak, base, bloom);
                vector<pair<int, int>> path = connectPath(pathLeft, pathRight, leftPeak, rightPeak);
            }
            else if (nodeMark[x] == RIGHT)
            {
                vector<pair<int, int>> pathLeft = findPath(rightPeak, x, bloom);
                vector<pair<int, int>> pathRight = findPath(leftPeak, base, bloom);
                vector<pair<int, int>> path = connectPath(pathLeft, pathRight, rightPeak, leftPeak);
            }
        }
        return path;
    }

    pair<int, int> baseStar(pair<int, int> v)
    {
        pair<int, int> base = v;
        while (nodeBloom[base].isDefined())
        {
            base = nodeBloom[base].base;
        }
        return base;
    }
};

int main()
{
    int e_count;
    cin >> e_count;
    vector<pair<int, int>> nodes;
    map<pair<int, int>, vector<pair<int, int>>> neighbors;
    map<pair<int, int>, map<pair<int, int>, map<string, bool>>> edges;
    for (int i = 0; i < e_count; ++i)
    {
        int a, b, c, d;
        cin >> a >> b >> c >> d;
        if (find(nodes.begin(), nodes.end(), make_pair(a, b)) == nodes.end())
            nodes.push_back({a, b});
        if (find(nodes.begin(), nodes.end(), make_pair(c, d)) == nodes.end())
            nodes.push_back({c, d});
        neighbors[{a, b}].push_back({c, d});
        neighbors[{c, d}].push_back({a, b});
        edges[{a, b}][{c, d}]["use"] = UNUSED;
        edges[{a, b}][{c, d}]["visit"] = UNVISITED;
    }
    MicaliVazirani MV(nodes, neighbors, edges);
    map<pair<int, int>, pair<int, int>> mapping = MV.max_cardinality_matching();
    for (auto maps : mapping)
    {
        cout << "\n(" << maps.first.first << "," << maps.first.second << ") = > (" << maps.second.first << "," << maps.second.second << ")";
    }
    return 0;
}