from utils.graph_utils import *
from utils.solve_utils import *
from utils.load_instances_utils import *


if __name__ == "__main__":
    # get instances data
    instances_data = load_instances()

    # get inputs
    b, h, G = instances_data[0]

    # preprocess graph for model input
    D = digraph_transformer(G)
    H = hop_limit_graph_transformer(D, h)

    # solve problem
    model = base_model_sol(H, b)

    # create graph based on the results
    J = get_solution_graph(model)

    # show graphs
    show_graphs([G, D, H, J])