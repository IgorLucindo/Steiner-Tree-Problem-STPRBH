import linecache
import networkx as nx
from pathlib import Path


# load instance
def load_instance(file_path):
    # get budget
    line = linecache.getline(file_path, 16).strip()
    b = int(line[7:])

    # get hop limit
    line = linecache.getline(file_path, 17).strip()
    h = int(line[9:])

    # number of edges
    line = linecache.getline(file_path, 14).strip()
    num_of_edges = int(line[6:])

    # get edges
    edges = []
    for i in range(num_of_edges):
        line = linecache.getline(file_path, 18 + i).strip()
        u, v, c = list(map(int, line.split()[1:]))
        edges.append((u, v, {'cost': c}))

    # create graph
    G = nx.Graph()
    G.add_edges_from(edges)
    G = nx.relabel_nodes(G, {1: 'r'})

    # number of profitable vertices
    line = linecache.getline(file_path, 22 + i).strip()
    num_of_profit_vertices = int(line[19:])
    
    # get revenue
    revenues = {v: 0 for v in G.nodes}
    for j in range(num_of_profit_vertices):
        line = linecache.getline(file_path, 23 + i + j).strip()
        v, p = list(map(int, line.split()[1:]))
        revenues[v] = p

    # add revenue to graph
    nx.set_node_attributes(G, revenues, 'revenue')

    # return
    return b, h, G


# load all instances in instances folder
def load_instances():
    # get stp files (instances)
    folder_path = Path("instances")
    file_paths = list(folder_path.glob("*.stp"))

    # get data from each instance
    instances_data = [(load_instance(file_path.as_posix())) for file_path in file_paths]

    # return
    return instances_data