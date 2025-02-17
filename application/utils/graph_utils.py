import networkx as nx
import random
import matplotlib.pyplot as plt
from collections import Counter, deque



# graph generator
def graph_generator(n, m):
    if m < n or m > n*(n-1)/2:
        raise ValueError("wrong number of edges.")
    
    # create complete graph
    G = nx.complete_graph(n)
    
    edges = list(G.edges())
    random.shuffle(edges)
    
    # removes valid edges
    while G.number_of_edges() > m:
        u, v = edges.pop()
        G.remove_edge(u, v)

        # make sure edge is valid
        if not nx.is_connected(G) or any(d == 1 for _, d in G.degree()):
            G.add_edge(u, v)

    # rename r vertex
    G = nx.relabel_nodes(G, {0: 'r'})

    # add cost and revenue
    costs = {e: random.randint(11, 20) for e in G.edges}
    revenues = {v: random.randint(1, 5) for v in G.nodes}
    revenues['r'] = 0
    nx.set_edge_attributes(G, costs, 'cost')
    nx.set_node_attributes(G, revenues, 'revenue')

    # return the connected graph
    return G


# transform graph in digraph
def digraph_transformer(G):
    D = nx.DiGraph(G)

    # Remove edges that are directed towards the root vertex r
    edges_to_remove = [a for a in D.in_edges('r')]
    D.remove_edges_from(edges_to_remove)

    # return directed graph
    return D


# removes vertices that have distance from vertex 'r' greater than h
def bfs_remove_vertices(D, h):
    # compute shortest path lengths from node r
    shortest_paths = nx.single_source_shortest_path_length(D, 'r')

    # create a subgraph with nodes within the hop limit
    H = D.subgraph([v for v, dist in shortest_paths.items() if dist <= h]).copy()

    # return limited graph
    return H


# removes arcs that have distance from vertex 'r' greater than h
def bfs_remove_arcs(D, h):
    # bfs algorithm for removing arcs
    queue = deque(['r'])
    dist_vertex = {'r': 0}

    # remove all arcs by default
    nx.set_edge_attributes(D, {a: True for a in D.edges}, 'removed')

    while queue:
        v = queue.popleft()

        # break if exceeds h hops
        if dist_vertex[v] == h:
            break

        # iterate for each outgoing neighbor of v
        for v_n in D.successors(v):
            D.edges[(v, v_n)]['removed'] = False
            
            if v_n not in dist_vertex:
                dist_vertex[v_n] = dist_vertex[v] + 1
                queue.append(v_n)

    # return limited graph
    return D


# create graph based on the results
def get_solution_graph(D, x):
    # add edges based on the variables
    selected_arcs = [a for a in D.edges if x[a].x > 0.5]
    
    # # create subgraph of D with selected edges
    H = D.edge_subgraph(selected_arcs)

    # return graph
    return H


# count number of cycle type (2-cycle, 3-cycle, 4-cycle, ...) and returns a dict
def count_cycle_type(D, count_flag):
    if not count_flag:
        return
    
    return dict(Counter(len(cycle) for cycle in nx.simple_cycles(D)))


# show graph
def show_graph(G):
    edge_labels = nx.get_edge_attributes(G, 'cost')
    node_labels = nx.get_node_attributes(G, 'revenue')
    pos = nx.spring_layout(G)
    pos_revenue = {v: (x + .05, y) for v, (x, y) in pos.items()}

    # draw graph
    nx.draw(G, pos, with_labels=True)

    # draw cost and revenue
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_color='r')
    nx.draw_networkx_labels(G, pos_revenue, labels=node_labels, font_color='g')
    
    # show
    plt.show()


# show graphs
def show_graphs(graphs, plot_flag=True):
    if not plot_flag:
        return

    # number of graphs
    num_graphs = len(graphs)
    # create subplots
    _, axes = plt.subplots(1, num_graphs, figsize=(5 * num_graphs, 5))

    # if there's only one graph, axes will not be an array, so we wrap it in a list
    if num_graphs == 1:
        axes = [axes]

    # iterate over the graphs and draw them
    for i, G in enumerate(graphs):
        # get attributes
        edge_labels = nx.get_edge_attributes(G, 'cost')
        node_labels = nx.get_node_attributes(G, 'revenue')
        # get position for every graph
        pos = nx.spring_layout(G)
        pos_revenue = {v: (x + .1, y) for v, (x, y) in pos.items()}

        # draw graph
        nx.draw(G, pos, with_labels=True, node_color='lightblue', edge_color='gray', node_size=200, width=1, ax=axes[i])
        axes[i].set_title(f"Graph {i + 1}")

        # draw cost and revenue
        nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_color='r', ax=axes[i])
        nx.draw_networkx_labels(G, pos_revenue, labels=node_labels, font_color='g', ax=axes[i])

    # show the figure
    plt.tight_layout()
    plt.show()