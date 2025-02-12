from utils.graph_utils import *
from utils.solve_utils import *


# set inputs
# budget
b = 60
# hop limit
h = 1
# directed graph
G = graph_generator(6, 9)
D = digraph_transformer(G)
H = hop_limit_graph_transformer(D, h)

# solve problem
model = base_model_sol(H, b)

# create graph based on the results
J = get_solution_graph(model)

# show graphs
show_graphs([G, D, H, J])