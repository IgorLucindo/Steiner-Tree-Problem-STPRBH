from utils.graph_utils import *
from utils.load_instances_utils import *
from utils.results_utils import *
import time


# set main parameters
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
        # preprocess graph for model input
        D = digraph_transformer(G)
        D1 = bfs_remove_vertices(D, h)
        D2 = bfs_remove_arcs(D, h)

        # get total time
        total_time = current_time - start_time

        
        # append results
        D2_arcs = [a for a in D2.edges if not D2.edges[a]['removed']]
        D2_vertices = {v for a in D2_arcs for v in a}
        vertex_dec = (len(D1.nodes) - len(D2_vertices))/len(D1.nodes)*100
        arc_dec = (len(D1.edges) - len(D2_arcs))/len(D1.edges)*100
        preprocess_results.append([
            len(D1.nodes),
            len(D1.edges),
            len(D2_vertices),
            len(D2_arcs),
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