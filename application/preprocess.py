from utils.graph_utils import *
from utils.load_instances_utils import *
from utils.results_utils import *
from classes.results import *
import time


# set parameter flags
COMPARE_RESULTS = False
SIMPLICIALS_RESULTS = True
PLOT_GRAPH = False


def main():
    # get instances data
    instances_data = load_instances()
    # instances_data = create_graph_instance()

    # create results class
    results = Results()

    # get start time
    start_time = time.time()

    #  iterate for every instance data
    for i, (b, h, G, num_of_vertices, num_of_edges) in enumerate(instances_data):
        # get current time
        current_time = time.time()

        # run model in instance
        # preprocess graphs
        if COMPARE_RESULTS:
            D1 = digraph_transformer(G)
            D1 = hop_limit_vertices(D1, h)
        else:
            D1 = None

        D2, simplicials_dict = preproccess_graph(G, h, SIMPLICIALS_RESULTS)
        D2 = delete_removed_edges_vertices(D2)

        # get total time
        total_time = current_time - start_time

        
        # append results
        compare_results = results.get_comparison_results(D1, D2, COMPARE_RESULTS)
        simplicials_results = results.get_simplicials_results(simplicials_dict, SIMPLICIALS_RESULTS)

        # print diagnosis
        print_instance_diagnosis(i, total_time)


        # show graphs
        show_graphs([G, D2], PLOT_GRAPH)


    # save results
    save_compare_preprocess_results(compare_results, COMPARE_RESULTS)
    save_simplicial_results(simplicials_results, SIMPLICIALS_RESULTS)


if __name__ == "__main__":
    main()