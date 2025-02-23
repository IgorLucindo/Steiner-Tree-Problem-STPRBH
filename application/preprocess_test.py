from utils.graph_utils import *


def main():
    # graph graph instance
    G = create_graph_instance()

    # preprocess graph
    D = preproccess_graph(G, h=5)
    D = delete_removed_edges_vertices(D)

    # show graphs
    show_graphs([G, D])


# graph graph instance
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

    # return graph
    return G


if __name__ == "__main__":
    main()