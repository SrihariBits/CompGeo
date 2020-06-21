#!/usr/bin/env python

import structures
"""
Maximum cardinality matching in general graphs.
This module implements the general matching algorithm given in "The General 
Maximum Matching Algorithm of Micali and Vazirani" by Paul A. Peterson and 
Michael C. Loui, Algorithmica, 1988. 
Many terms used in the code comments are explained in the paper by Peterson
and Loui. The paper could prove necessary in making sense of this code.
:filename matching.py
"""

# Authorship information
__author__ = "Alexander Soloviev"
__email__ = "solova@rpi.edu"
__date__ = "04/04/2015"
__all__ = ['max_cardinality_matching']

# Necessary imports


def max_cardinality_matching(G):
    """Compute a maximum cardinality matching in a general graph G.

    A matching is a subset of edges in which no node occurs more than once.
    The cardinality of a matching is the number of matched edges.
    The maximum matching is a matching of maximum cardinality.

    :param G - the NetworkX graph given
        Undirected graph.

    :return mate - dictionary
        The matching is returned as a dictionary such that
        mate[v] == w if node v is matched to node w. Unmatched
        nodes do not occur as a key in mate.

    :notes
    This function takes time O(sqrt(number_of_nodes) * number_of_edges).

    This method is based on the "blossom" method for finding augmenting
    paths.

    :references
    ..[1] "The General Maximum Matching Algorithm of Micali and Vazirani",
        Paul A. Peterson and Michael C. Loui, Algorithmica, 1988
    """

    # Global variables for initializing node attributes
    INFINITY = len(G) + 1  # Odd and even level attribute value

    UNERASED = False  # Erase attribute value
    ERASED = True

    UNVISITED = False  # Visit attribute value
    VISITED = True

    LEFT = -1  # Left and right attribute value
    UNMARKED = 0
    RIGHT = 1

    # Global variables for initializing edge attributes
    UNUSED = False
    USED = True

    class Bloom:
        """ Representation of a bloom (a generalization of a blossom).

        A blossom is a circuit of odd length, say 2k+1, that has k matched
        edges. This class stores only the peak vertices and the base vertex
        of the bloom.
        """

        __slots__ = ['peaks', 'base']

    class DfsInfo:
        """ The information needed by the left and right depth first searches.

        In calling leftDfs and rightDfs, the vertices could get updated or
        modified. This class stores all of the parameters that could be 
        altered.
        """

        def __init__(self, s, t, vL, vR, dcv, barrier):
            self.s = s
            self.t = t
            self.vL = vL
            self.vR = vR
            self.dcv = dcv
            self.barrier = barrier

    # Get a list of vertices
    gnodes = G.nodes()
    if not gnodes:
        return {}  # Ignore empty graphs

    # Initialize the top-level data structures for node attributes.
    # Each of these is a dictionary indexed by the node.
    nodeEvenLevel = {}
    nodeOddLevel = {}
    nodeBloom = {}
    nodePredecessors = {}
    nodeSuccessors = {}
    nodeAnomalies = {}
    nodeCount = {}
    nodeErase = {}
    nodeVisit = {}
    nodeMark = {}
    nodeParent = {}

    # Path compression:
    #nodeBaseStar = { }

    # Initialize the top-level data structure for nodes marked
    # left or right during the current call to augmentBlossom. If a
    # bloom is found, these nodes will be a part of the bloom.
    bloomNodes = []

    # Initialize the top-level data structure for candidates.
    # Candidates is constructed so that candidates[i] contains all of the
    # vertices to search at the current level i.
    candidates = {}

    # Initialize the top-level data structure for bridges.
    # Bridges is constructed so that bridges[i] contains all bridges at
    # level i. A bridge is an edge whose removal leaves a disconnected graph.
    bridges = {}

    # If v is a matched vertex, mate[v] is its partner vertex.
    # If v is a single vertex, v does not occur as a key in mate.
    # Initially all vertices are single and are updated during augmentation.
    mate = {}

    def search():
        """ The search subroutine.

        Find all augmenting paths of minimal length and increase the current
        matching along these paths. Call the augmentBlossom function with 
        each bridge found.

        :return augmented - True if matching was augmented, False otherwise
        """

        i = 0  # Counter for the current level

        # Insert each exposed vertex into candidates
        for v in G.nodes_iter():
            if v not in mate:
                nodeEvenLevel[v] = 0
                candidates[0].append(v)

        # Perform a breadth-first search through each of the vertices.
        # Continue iteration while candidates is not empty and no augmentation
        # occurred at level i-1.
        augmented = False
        while (i < len(gnodes) + 1) and not augmented:

            if i % 2 == 0:  # If level i is even
                for v in candidates[i]:

                    # For each unerased and unmatched neighbor u of node v,
                    # determine whether the edge (u, v) is a bridge.
                    for u in G.neighbors_iter(v):
                        if u == v:
                            continue  # Ignore self-loops
                        if mate.get(v) != u and nodeErase[u] == UNERASED:
                            assert mate.get(u) != v
                            if nodeEvenLevel[u] < INFINITY:
                                j = (nodeEvenLevel[u] + nodeEvenLevel[v]) / 2
                                bridges[j].add(tuple(sorted([u, v])))
                            else:
                                if nodeOddLevel[u] == INFINITY:
                                    nodeOddLevel[u] = i + 1
                                if nodeOddLevel[u] == i + 1:
                                    nodeCount[u] += 1
                                    nodePredecessors[u].append(v)
                                    nodeSuccessors[v].append(u)
                                    candidates[i + 1].append(u)
                                elif nodeOddLevel[u] < i:
                                    nodeAnomalies[u].append(v)

            else:  # If level i is odd
                for v in candidates[i]:

                    # For each node v in candidates such that v belongs to no bloom,
                    # determine whether the edge (u, v) is a bridge, where u is the
                    # mate of v.
                    if nodeBloom[v] == None:
                        u = mate[v]
                        if nodeOddLevel[u] < INFINITY:
                            j = (nodeOddLevel[u] + nodeOddLevel[v]) / 2
                            bridges[j].add(tuple(sorted([u, v])))
                        elif nodeEvenLevel[u] == INFINITY:
                            nodePredecessors[u] = [v]
                            nodeSuccessors[v] = [u]
                            nodeCount[u] = 1
                            nodeEvenLevel[u] = i + 1
                            candidates[i + 1].append(u)

            # Call augmentBlossom for each edge in bridges
            for s, t in bridges[i]:
                if nodeErase[s] == UNERASED and nodeErase[t] == UNERASED:
                    augmented = augmentBlossom(s, t, i)

            i += 1  # Increment the level counter

        return augmented

    def augmentBlossom(s, t, i):
        """ The augmentBlossom subroutine, or blossAug.

        Either define a new blossom, discover that s and t are in the same
        blossom, or find an augmenting path by using a double depth-first
        search. Use the functions leftDfs, rightDfs, and erasePath. Upon
        return, augmented is True if an augmenting path was found.

        :param s - first node of a bridge
        :param t - second node of a bridge
        :param i - the current level
        :return augmented - True if augmenting path was found, False otherwise
        """

        # Boolean flags for whether a bloom was found or augmentation occurred
        foundBloom = False
        augmented = False

        vL = baseStar(s) if nodeBloom[s] else s
        vR = baseStar(t) if nodeBloom[t] else t

        if vL == vR:
            return False  # Exit if s and t belong to same compressed bloom

        # Set the parent nodes accordingly
        if nodeBloom[s]:
            nodeParent[vL] = s
        if nodeBloom[t]:
            nodeParent[vR] = t

        # Mark vL left and vR right
        nodeMark[vL] = LEFT
        nodeMark[vR] = RIGHT
        bloomNodes.append(vL)
        bloomNodes.append(vR)

        # DfsInfo stores information about s, t, vL, vR, dcv, and barrier vertices
        dfsInfo = DfsInfo(s, t, vL, vR, None, vR)

        # While a bloom has not been found and no augmentation has occurred,
        # perform the double depth-first search.
        while not foundBloom and not augmented:

            # Get the levels of both vL and vR
            if dfsInfo.vL == None or dfsInfo.vR == None:
                return False
            level_vL = min(nodeEvenLevel[dfsInfo.vL], nodeOddLevel[dfsInfo.vL])
            level_vR = min(nodeEvenLevel[dfsInfo.vR], nodeOddLevel[dfsInfo.vR])

            # Increase the matching if vL and vR are both exposed
            if dfsInfo.vL not in mate and dfsInfo.vR not in mate:
                pathL = findPath(dfsInfo.s, dfsInfo.vL, None)
                pathR = findPath(dfsInfo.t, dfsInfo.vR, None)
                path = connectPath(pathL, pathR, dfsInfo.s, dfsInfo.t)
                augmentMatching(dfsInfo.vL, dfsInfo.vR)
                erasePath(path)
                augmented = True
                break
            elif level_vL >= level_vR:
                foundBloom = leftDfs(dfsInfo)  # Call leftDfs
            else:
                foundBloom = rightDfs(dfsInfo)  # Call rightDfs

        # Create a new bloom if a bloom is found by the depth-first search.
        if foundBloom and dfsInfo.dcv != None:

            # Vertex dcv cannot be in the bloom
            nodeMark[dfsInfo.dcv] = UNMARKED

            b = Bloom()  # Create a new bloom
            b.peaks = (dfsInfo.s, dfsInfo.t)  # Assign it the peak vertices
            b.base = dfsInfo.dcv  # Assign it a base vertex

            # Path compression
            #baseStardcv = baseStar( dfsInfo.dcv )
            #assert baseStardcv != None

            # Put each vertex marked left or right during this call in the
            # new bloom
            for v in bloomNodes:
                if nodeMark[v] == UNMARKED or nodeBloom[v] != None:
                    continue  # If no mark or bloom already defined, skip it

                # Set the bloom attribute of the vertex
                nodeBloom[v] = b

                # Path compression
                # Set the base* attribute of the vertex
                #nodeBaseStar[v] = baseStardcv

                level_v = min(nodeEvenLevel[v], nodeOddLevel[v])
                if level_v % 2 == 0:  # Check if v is outer
                    nodeOddLevel[v] = 2*i + 1 - nodeEvenLevel[v]
                else:  # Else v is inner
                    nodeEvenLevel[v] = 2*i + 1 - nodeOddLevel[v]
                    candidates[nodeEvenLevel[v]].append(v)
                    for z in nodeAnomalies[v]:
                        j = (nodeEvenLevel[v] + nodeEvenLevel[z]) / 2
                        bridges[j].add(tuple(sorted([v, z])))
                        G[v][z]['use'] = USED

        # Clear the bloomNodes list
        del bloomNodes[:]

        return augmented

    def connectPath(pathL, pathR, s, t):
        """ Connect two paths into a single path.

        :param pathL - the left path given as a list
        :param pathR - the right path given as a list
        :param s - first node of a bridge
        :param t - second node of a bridge
        :return path - the combination of both paths
        """

        reverseL = True if s == pathL[0] else False
        reverseR = True if t == pathR[-1] else False

        # Reverse the parent pointers of pathL
        if reverseL:
            nodeParent[pathL[0]] = None
            prevv = None
            currentv = pathL[-1]
            nextv = None
            while currentv != None:
                nextv = nodeParent[currentv]
                nodeParent[currentv] = prevv
                prevv = currentv
                currentv = nextv

            # Reverse the list pathL
            pathL.reverse()

        # Reverse the parent pointers of pathR
        if reverseR:
            nodeParent[pathR[0]] = None
            prevv = None
            currentv = pathR[-1]
            nextv = None
            while currentv != None:
                nextv = nodeParent[currentv]
                nodeParent[currentv] = prevv
                prevv = currentv
                currentv = nextv

            # Reverse the list pathR
            pathR.reverse()

        # Initialize the combined path
        path = []
        path.extend(pathL)
        path.extend(pathR)

        # Connect the parent pointers of the path nodes
        nodeParent[pathR[0]] = pathL[-1]

        return path

    def augmentMatching(lv, rv):
        """ Augment the matching by the path from vertex lv to vertex rv.

        :param lv - the left vertex
        :param rv - the right vertex
        """

        # Iterate through the path by following the parent pointers
        firstv = rv
        secondv = None
        while firstv != lv:

            # Get the parent node of firstv
            secondv = nodeParent[firstv]

            if mate.get(secondv) != firstv:
                assert mate.get(firstv) != secondv

                # Add the vertices to mate
                mate[firstv] = secondv
                mate[secondv] = firstv

            firstv = secondv

    def leftDfs(dfsInfo):
        """ The leftDfs subroutine.

        One step of the left depth-first search process. This step either
        advances vL to a predecessor, backtracks, or signals the discovery
        of a bloom.

        :param dfsInfo - the information stored for the depth-first searches
        :return bool - True if a bloom was found, False otherwise
        """

        # Search through all unused and unerased predecessor edges of vL
        for uL in nodePredecessors[dfsInfo.vL]:

            # Skip the edge (vL, uL) if it is used or erased
            if G[dfsInfo.vL][uL]['use'] == USED or nodeErase[uL] == ERASED:
                continue

            # Mark the edge (vL, uL) as used
            G[dfsInfo.vL][uL]['use'] = USED

            # If uL belongs to a bloom, set the bloombase of uL
            if nodeBloom[uL]:
                uL = baseStar(uL)

            # If uL is unmarked, set its mark and exit
            if nodeMark[uL] == UNMARKED:
                nodeMark[uL] = LEFT
                nodeParent[uL] = dfsInfo.vL
                dfsInfo.vL = uL
                bloomNodes.append(uL)
                return False

            # Otherwise if u is equal to vR, set the dcv equal to u
            elif uL == dfsInfo.vR:
                dfsInfo.dcv = uL

        # If u has a mark, then leftDfs is backtracking
        if dfsInfo.vL == dfsInfo.s:
            return True  # Signal discovery of a bloom
        elif nodeParent[dfsInfo.vL] != None:
            dfsInfo.vL = nodeParent[dfsInfo.vL]  # Keep backtracking

        return False

    def rightDfs(dfsInfo):
        """ The rightDfs subroutine.

        One step of the right depth-first search process. This step either
        advances vR to a predecessor or backtracks, or signals the discovery
        of a bloom.

        :param dfsInfo - the information stored for the depth-first searches
        :return bool - True if a bloom was found, False otherwise
        """

        # Search through all unused and unerased predecessor edges of vR
        for uR in nodePredecessors[dfsInfo.vR]:

            # Skip the edge (vR, uR) if it is used or erased
            if G[dfsInfo.vR][uR]['use'] == USED or nodeErase[uR] == ERASED:
                continue

            # Mark the edge (vR, uR) as used
            G[dfsInfo.vR][uR]['use'] = USED

            # If uR belongs to a bloom, set the bloombase of uR
            if nodeBloom[uR]:
                uR = baseStar(uR)

            # If u is unmarked, set its mark and exit
            if nodeMark[uR] == UNMARKED:
                nodeMark[uR] = RIGHT
                nodeParent[uR] = dfsInfo.vR
                dfsInfo.vR = uR
                bloomNodes.append(uR)
                return False

            # Otherwise if u is equal to vL, set the dcv equal to u
            elif uR == dfsInfo.vL:
                dfsInfo.dcv = uR

        # The vertex vR has no more unused predecessor edges
        if dfsInfo.vR == dfsInfo.barrier:
            dfsInfo.vR = dfsInfo.dcv
            dfsInfo.barrier = dfsInfo.dcv
            nodeMark[dfsInfo.vR] = RIGHT
            if nodeParent[dfsInfo.vL] != None:
                # Force leftDfs to backtrack from vL = dcv
                dfsInfo.vL = nodeParent[dfsInfo.vL]
        elif nodeParent[dfsInfo.vR] != None:
            dfsInfo.vR = nodeParent[dfsInfo.vR]  # Keep backtracking

        return False

    def erasePath(path):
        """ The erasePath subroutine (erase).

        Set the erase attribute for all vertices in the input path to
        erased. Once all predecessors of a vertex have been erased, the
        vertex itself is erased too.

        :param path - the list of vertices to be erased
        """

        # While there are vertices left in the path
        while path:

            # Get a vertex from the path
            y = path.pop()
            nodeErase[y] = ERASED

            # Iterate through each of its successors
            for z in nodeSuccessors[y]:
                if nodeErase[z] == UNERASED:
                    nodeCount[z] -= 1

                    # If the successor is unerased, add it to the path
                    if nodeCount[z] == 0:
                        path.append(z)

    def findPath(high, low, b):
        """ The findPath subroutine.

        Find an alternating path from vertex high to vertex low through
        the predecessor vertices. Note that the level of high is greater 
        or equal to the level of low. Call openBloom to find paths through
        blooms other than bloom b.

        :param high - the high vertex
        :param low - the low vertex
        :param b - the bloom given
        :return path - the alternating path found
        """

        # Determine the level of the vertices high and low
        level_high = min(nodeEvenLevel[high], nodeOddLevel[high])
        level_low = min(nodeEvenLevel[low], nodeOddLevel[low])
        assert level_high >= level_low

        # If the vertices are equivalent, return a single node path
        if high == low:
            return [high]

        # Initialize the alternating path
        path = []

        # Perform a depth-first search to find the vertex low from the vertex high
        v = high
        u = high
        while u != low:

            # Check whether v has unvisited predecessor edges
            hasUnvisitedPredecessor = False

            for p in nodePredecessors[v]:

                # Break if the edge (p, v) is unvisited
                if G[p][v]['visit'] == UNVISITED:
                    hasUnvisitedPredecessor = True

                    # Check whether vertex v belongs to a bloom, set u accordingly
                    if nodeBloom[v] == None or nodeBloom[v] == b:
                        G[p][v]['visit'] = VISITED
                        u = p
                    else:
                        u = nodeBloom[v].base

                    break

            # There are no unvisited predecessor edges, so backtrack
            if not hasUnvisitedPredecessor:
                assert nodeParent[v] != None
                v = nodeParent[v]
            else:
                # Get the level of node u
                level_u = min(nodeEvenLevel[u], nodeOddLevel[u])

                # Mark u visited and set the parent pointers
                if nodeErase[u] == UNERASED and level_u >= level_low \
                    and (u == low or (nodeVisit[u] == UNVISITED
                                      and (nodeMark[u] == nodeMark[high] != UNMARKED
                                           or (nodeBloom[u] != None and nodeBloom[u] != b)))):
                    nodeVisit[u] = VISITED
                    nodeParent[u] = v
                    v = u

        # Compute the path
        while u != high:
            path.append(u)
            u = nodeParent[u]
        path.append(u)
        path.reverse()

        # The path has been found, except for blooms other than bloom b
        # These blooms must be opened using openBloom
        j = 0
        while j < len(path) - 1:
            xj = path[j]

            # Replace the part of the path by the output of openBloom
            if nodeBloom[xj] != None and nodeBloom[xj] != b:
                nodeVisit[xj] = UNVISITED
                path[j: j + 2], pathLength = openBloom(xj)
                nodeParent[xj] = path[j - 1] if j > 0 else None
                j += pathLength - 1
            j += 1

        return path

    def openBloom(x):
        """ The openBloom subroutine (open).

        Return an alternating path from vertex x through the bloom of 
        x to the base of the bloom. Call findPath to get this alternating
        path.

        :param x - the vertex given
        :return path - the alternating path through the bloom
        """

        # Get the bloom that vertex x corresponds to
        bloom = nodeBloom[x]
        base = bloom.base
        level_x = min(nodeEvenLevel[x], nodeOddLevel[x])
        path = []

        if level_x % 2 == 0:  # If x is outer
            path = findPath(x, base, bloom)
        else:  # Else x is inner
            # Get the peaks of the bloom
            (leftPeak, rightPeak) = bloom.peaks
            if nodeMark[x] == LEFT:  # If x is marked left
                pathLeft = findPath(leftPeak, x, bloom)
                pathRight = findPath(rightPeak, base, bloom)
                path = connectPath(pathLeft, pathRight, leftPeak, rightPeak)
            elif nodeMark[x] == RIGHT:  # Else x is marked right
                pathLeft = findPath(rightPeak, x, bloom)
                pathRight = findPath(leftPeak, base, bloom)
                path = connectPath(pathLeft, pathRight, rightPeak, leftPeak)
        return (path, len(path))

    def baseStar(v):
        """ The base* function.

        Return the base* of the vertex v and compress the path 
        traversed. This has the effect of shrinking a bloom into its
        base*.

        :param v - the vertex given
        :return base - the base* of v
        """

        base = v
        while nodeBloom[base] != None:
            assert nodeBloom[base].base != base
            base = nodeBloom[base].base

        # Path compression:
        # while nodeBaseStar[n] != None:
        #    n = nodeBaseStar[n]
        # while v != n:
        #    vNext = nodeBaseStar[v]
        #    nodeBaseStar[v] = n
        #    v = vNext
        return base

    # Main loop: continue iteration until no further augmentation is possible.
    augmented = True
    while augmented:

        # Initialize/reset the nodes
        for v in G.nodes_iter():
            nodeEvenLevel[v] = INFINITY
            nodeOddLevel[v] = INFINITY
            nodeBloom[v] = None
            nodePredecessors[v] = []
            nodeSuccessors[v] = []
            nodeAnomalies[v] = []
            nodeCount[v] = 0
            nodeErase[v] = UNERASED
            nodeVisit[v] = UNVISITED
            nodeMark[v] = UNMARKED
            nodeParent[v] = None

            # Path compression
            #nodeBaseStar[v] = None

        # Initialize/reset the edges
        for u, v, d in G.edges_iter(data=True):
            if u == v:
                continue  # Ignore self-loops
            d['use'] = UNUSED
            d['visit'] = UNVISITED

        # Initialize/reset the candidates and bridges
        for i in range(len(gnodes) + 1):
            candidates[i] = []
            bridges[i] = structures.OrderedSet()

        # Call the search subroutine
        augmented = search()

        # Paranoia check that the matching is symmetric
        for v in mate:
            assert mate[mate[v]] == v

    # Delete edge attributes from graph G
    for u, v, d in G.edges_iter(data=True):
        if u == v:
            continue  # Ignore self-loops
        del d['use']
        del d['visit']

    return mate

# end
