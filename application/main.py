from utils.graph_utils import *
from utils.solve_utils import *
from utils.load_instances_utils import *
import pandas as pd
import time


if __name__ == "__main__":
    # create data frame for results
    df = pd.DataFrame(columns=['Instance', 'Running Time'])

    # get instances data
    instances_data = load_instances()

    #  iterate for every instance data
    for i, instance_data in enumerate(instances_data):
        # get start time
        start_time = time.time()

        # run model in instance
        # get inputs
        b, h, G = instance_data

        # preprocess graph for model input
        D = digraph_transformer(G)
        H = hop_limit_graph_transformer(D, h)

        # solve problem
        model = base_model_sol(H, b)

        # create graph based on the results
        J = get_solution_graph(model)

        # get end time
        end_time = time.time()
        # get iteration time
        iteration_time = end_time - start_time

        # print results
        new_row = {
            'Instance': i,
            'Running Time': iteration_time
        }
        df.loc[len(df)] = new_row
        print(df.iloc[-1])

        # show graphs
        # if i == 0:
        #     show_graphs([G, D, H, J])
        #     show_graphs([G, J])
        #     break