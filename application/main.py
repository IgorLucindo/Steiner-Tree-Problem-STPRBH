from utils.graph_utils import *
from utils.solve_utils import *
from utils.load_instances_utils import *
from utils.results_utils import *
import time


if __name__ == "__main__":
    # get instances data
    instances_data = load_instances()

    #  iterate for every instance data
    for i, (b, h, G, num_of_vertices, num_of_edges) in enumerate(instances_data):
        # get start time
        start_time = time.time()

        # run model in instance
        # preprocess graph for model input
        D = digraph_transformer(G)
        D = hop_limit_graph_transformer(D, h)

        # solve problem
        model = base_model_sol(D, b)

        # create graph based on the results
        O = get_solution_graph(model)

        # get end time
        end_time = time.time()
        # get iteration time
        iteration_time = end_time - start_time

        # show results
        show_instance_results(i, num_of_vertices, num_of_edges, iteration_time)

        # show graphs
        # if i == 0:
        #     show_graphs([G, D, O])
        #     show_graphs([G, O])
        #     break