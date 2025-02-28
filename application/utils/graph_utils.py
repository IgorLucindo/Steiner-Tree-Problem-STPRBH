import networkx as nx
import matplotlib.pyplot as plt
from collections import Counter


# create a simple instance of a graph
def create_graph_instance():
    # create graph
    G = nx.Graph()
    graph_edges = [('r', 2, {'cost': 1}), (2, 3, {'cost': 1}), (2, 4, {'cost': 1}), (3, 4, {'cost': 1}), (2, 5, {'cost': 1}), (3, 5, {'cost': 1}), (4, 5, {'cost': 1}), 
                   ('r', 11, {'cost': 1}), (11, 12, {'cost': 1}), (12, 13, {'cost': 1})]
    G.add_edges_from(graph_edges)
    
    # get revenue
    revenues = {v: 1 for v in G.nodes}
    revenues['r'] = 0
    revenues[13] = 0
    revenues[5] = 0
    revenues[4] = 0
    nx.set_node_attributes(G, revenues, 'revenue')

    # set removed vertices
    nx.set_node_attributes(G, {v: False for v in G.nodes}, 'removed')

    # set removed edges
    nx.set_edge_attributes(G, {e: False for e in G.edges}, 'removed')


    # create instance
    b = 100
    h = 5
    num_of_vertices = len(G.nodes)
    num_of_edges = len(G.edges)
    instance = [b, h, G, num_of_vertices, num_of_edges]

    # return instance
    return [instance]


# transform graph in digraph
def digraph_transformer(G):
    D = nx.DiGraph(G)

    # Remove edges that are directed towards the root vertex r
    edges_to_remove = [a for a in D.in_edges('r')]
    D.remove_edges_from(edges_to_remove)

    # return directed graph
    return D


# hop-limit last vertices
def hop_limit_vertices(D, h):
    # compute shortest path lengths from node r
    shortest_paths = nx.single_source_shortest_path_length(D, 'r')

    # return hop limit graph
    return D.subgraph([v for v, dist in shortest_paths.items() if dist <= h])


# hop-limit last vertices and arcs, remove non profitable stop vertices
def hop_limit_arcs(D, h):
    # Hamid's idea
    # compute shortest path lengths from node r
    shortest_paths = nx.single_source_shortest_path_length(D, 'r')

    # remianed_vertices in graph
    remained_vertices = []

    for v in D.nodes:
        # hop limit vertices
        if shortest_paths[v] < h:
            remained_vertices.append(v)
        # hop limit profitable stop vertices
        elif shortest_paths[v] == h and D.nodes[v]['revenue'] > 0.5:
            remained_vertices.append(v)

            # remove outgoing arcs from 
            for a in D.out_edges(v):
                D.edges[a]['removed'] = True

    # return prunded graph
    return D.subgraph(remained_vertices)


# get simplicials dict
# keys are the stems of a clique, value are non-porfitable simplicials
def get_simplicials(D):
    # create simplicial dict
    simplicials_dict = {}

    for v in D.nodes:
        # skip if v is profitable or is the root
        if D.nodes[v]['revenue'] != 0 or v == 'r':
            continue

        # check if v is simplicial and get clique G (simple graph)
        neighbors = list(set(D.successors(v)) | set(D.predecessors(v)))

        # simple subgraph of neighbors
        G = nx.Graph(D.subgraph(neighbors))

        # if is clique
        if is_clique(G):
            # get stems from clique
            stems = get_stems_from_clique(D, neighbors)
            # append to simplicials dict
            simplicials_dict.setdefault(tuple(stems), {
                'non_profitable_simplicials': [],
                'number_of_vertices': len(neighbors) + 1
            })['non_profitable_simplicials'].append(v)
             
    # return prunded graph
    return simplicials_dict


# remove appropriated simplicials
def prune_simplicials(D):
    # cases that non-profitable simplicial v cannot be removed:
    # case 1: path from stem u_i to stem u_j passes through v
    # case 2: path from stem u_i to profitable simplicial v_j passes through v

    # cases that non-profitable simplicials can be removed
    # clique has 1 stem and only non profitable simplicials

    for v in D.nodes:
        # skip if v is profitable or is the root
        if D.nodes[v]['revenue'] != 0 or v == 'r':
            continue

        # check if v is simplicial and get clique G (simple graph)
        neighbors = list(set(D.successors(v)) | set(D.predecessors(v)))
        forms_clique, G = is_clique(D, neighbors)

        # if is clique
        if forms_clique:
            neighbor_costs = sum([G.edges[a]['cost'] for a in G.edges])
            simplicial_edge_costs = sum(D.edges[u, v]['cost'] for u in neighbors)

            # remove simplicial if it will never be accessed
            if simplicial_edge_costs >= neighbor_costs:
                D.nodes[v]['removed'] = True
             
    # return prunded graph
    return D


# preprocess graph for model input
def preproccess_graph(G, h, return_simplicials_flag=False):
    # preprocess
    D = digraph_transformer(G)
    D = hop_limit_arcs(D, h)
    simplicials_dict = get_simplicials(D)
    # D = prune_simplicials(simplicials_dict)

    # return preprocessed graph
    if return_simplicials_flag:
        return D, simplicials_dict
    return D, None


# check if G is clique
def is_clique(G):
    # number of vertices
    n = len(G.nodes)

    # return if G is clique
    return G.size() == n*(n - 1)/2


# get stems vertices from clique graph
def get_stems_from_clique(D, vertices):
    # number of vertices
    n = len(vertices)

    # return stems
    return [v for v in vertices if len(set(D.predecessors(v)) | set(D.successors(v))) != n]


# remove edges with attribute 'remove: True
def delete_removed_edges_vertices(D):
    # filter vertices where 'removed': False
    vertices = [v for v in D.nodes if D.nodes[v]['removed'] == False]

    # create subgraph
    D = D.subgraph(vertices)

    # filter edges where 'removed': False
    edges = [a for a in D.edges if D.edges[a]['removed'] == False]

    # create subgraph
    D = D.edge_subgraph(edges)

    # return new graph
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
        pos_revenue = {v: (x + .05, y) for v, (x, y) in pos.items()}

        # draw graph
        nx.draw(G, pos, with_labels=True, node_color='lightblue', edge_color='gray', node_size=200, width=1, ax=axes[i])
        axes[i].set_title(f"Graph {i + 1}")

        # draw cost and revenue
        nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels, font_color='r', ax=axes[i])
        nx.draw_networkx_labels(G, pos_revenue, labels=node_labels, font_color='g', ax=axes[i])

    # show the figure
    plt.tight_layout()
    plt.show()