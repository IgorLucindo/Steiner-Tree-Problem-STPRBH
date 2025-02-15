from utils.graph_utils import *
from utils.solve_utils import *
from utils.load_instances_utils import *
from utils.results_utils import *
import time


# set main parameters
PLOT_GRAPH = False
SAVE_CYCLE_RESULTS = True


def main():
    # get instances data
    instances_data = load_instances()

    # create results variable
    model_results = []
    cycle_counter_results = []

    # get start time
    start_time = time.time()

    #  iterate for every instance data
    for i, (b, h, G, num_of_vertices, num_of_edges) in enumerate(instances_data):
        # get current time
        current_time = time.time()


        # run model in instance
        # preprocess graph for model input
        D = digraph_transformer(G)
        D = bfs_remove_vertices(D, h)

        # solve problem
        model, modelname = base_model_sol(D, b)

        # create graph based on the results
        H = get_solution_graph(D, model)


        # get end time
        end_time = time.time()
        # get iteration time
        iteration_time = end_time - current_time
        # get total time
        total_time = current_time - start_time


        # append model results
        model_results.append([num_of_vertices, num_of_edges, iteration_time])

        # append number of each cycle type to results
        cycle_counter_results.append(count_cycle_type(H, SAVE_CYCLE_RESULTS))


        # print diagnosis
        print_instance_diagnosis(i, total_time)

        # show graphs
        show_graphs([G, H], PLOT_GRAPH)

        # break soon
        # if i == 10:
        #     break

    # save results
    save_model_results(model_results, modelname)
    save_cycle_results(cycle_counter_results, modelname, SAVE_CYCLE_RESULTS)


if __name__ == "__main__":
    main()