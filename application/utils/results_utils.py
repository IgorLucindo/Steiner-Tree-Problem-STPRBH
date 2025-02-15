import pandas as pd
import sys


# print instance results
def print_instance_diagnosis(i, total_time):
    if i % 10 == 0:
        sys.stdout.write(f"\rIteration: {i} - total time: {total_time:.3f} s")
        sys.stdout.flush()


# save model results
def save_model_results(results, modelname):
    # create dataframe for exporting to xlsx file
    df = pd.DataFrame(results, columns=["#Vertices", "#Edges", "Running Time"])
    df.index = df.index + 1
    df.to_excel(f"results/model/{modelname}.xlsx", index=True, index_label="Instance")


# save cycle results
def save_cycle_results(results, modelname, save_flag):
    if not save_flag:
        return
    
    # create dataframe for exporting to xlsx file
    df = pd.DataFrame(results)
    df = df.fillna(0)
    df = df.sort_index(axis=1)
    df.index = df.index + 1
    df.to_excel(f"results/cycle_counter/{modelname}.xlsx", index=True, index_label="Instance")