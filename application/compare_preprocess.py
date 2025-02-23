from utils.graph_utils import *
from utils.load_instances_utils import *
from utils.results_utils import *
import time


# set parameter flags
PLOT_GRAPH = False


def main():
    # get instances data
    instances_data = load_instances()

    # create results variable
    preprocess_results = []

    # get start time
    start_time = time.time()

    #  iterate for every instance data
    for i, (b, h, G, num_of_vertices, num_of_edges) in enumerate(instances_data):
        # get current time
        current_time = time.time()

        # run model in instance
        # preprocess graphs
        D1 = digraph_transformer(G)
        D1 = bfs_remove_vertices(D1, h)

        D2 = preproccess_graph(G, h)
        D2 = delete_removed_edges_vertices(D2)

        # get total time
        total_time = current_time - start_time

        
        # append results
        vertex_dec = (len(D1.nodes) - len(D2.nodes))/len(D1.nodes)*100
        arc_dec = (len(D1.edges) - len(D2.edges))/len(D1.edges)*100
        preprocess_results.append([
            len(D1.nodes),
            len(D1.edges),
            len(D2.nodes),
            len(D2.edges),
            vertex_dec,
            arc_dec
        ])


        # print diagnosis
        print_instance_diagnosis(i, total_time)


        # show graphs
        show_graphs([G, D2], PLOT_GRAPH)

        # break soon
        # if i == 10:
        #     break

    # save results
    save_preprocess_results(preprocess_results)

if __name__ == "__main__":
    main()