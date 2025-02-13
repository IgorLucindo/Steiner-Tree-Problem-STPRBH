# show instance results
def show_instance_results(i, num_of_vertices, num_of_edges, iteration_time):
    if i == 0:
        print(f"{'Instance':<10} {'#Vertices':<11} {'#Edges':<8} {'Running Time':<12}")
    print(f"{i+1:<10} {num_of_vertices:<11} {num_of_edges:<8} {iteration_time:<12.3f}")