from utils.graph_utils import *


def main():
    # graph graph instance
    G = create_graph_instance()

    h = 5
    # preprocess graph
    D = preproccess_graph(G, h)

    # show graphs
    show_graphs([G, D])


# graph graph instance
def create_graph_instance():
    G = nx.Graph()
    graph_edges = [('r', 2), (2, 3), (2, 4), (3, 4), (2, 5), (3, 5), (4, 5), ('r', 11), (11, 12), (12, 13)]
    G.add_edges_from(graph_edges)

    # return graph
    return G


if __name__ == "__main__":
    main()