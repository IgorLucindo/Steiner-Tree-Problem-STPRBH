import gurobipy as gp
from gurobipy import GRB
from utils.graph_utils import get_solution_graph


# base model formulation
def base_model_sol(D, b):
    # create model
    model = gp.Model(name="base model")

    # Disable all output
    model.setParam('OutputFlag', 0)

    # add variables
    x = model.addVars(D.edges, vtype=GRB.BINARY, name='x')
    y = model.addVars(D.nodes, vtype=GRB.BINARY, name='y')

    # add objective function
    obj_fn = gp.quicksum(D.nodes[v]['revenue'] * y[v] for v in D.nodes)
    model.setObjective(obj_fn, GRB.MAXIMIZE)

    # add constraints
    y['r'].lb = 1
    model.addConstrs((gp.quicksum(x[a] for a in D.in_edges(v)) == y[v] for v in D.nodes if v != 'r'), name='c1')
    model.addConstr((gp.quicksum(D.edges[a]['cost'] * x[a] for a in D.edges) <= b), name='c2')
    model.addConstrs((x[u, v] <= y[u] for (u, v) in D.edges), name='c3')
    model.addConstrs((x[u, v] + x[v, u] <= 1 for (u, v) in D.edges if 'r' not in {u, v}), name='c4')
    # model.addConstrs((x[u, v] + x[v, u] <= 1 for (u, v) in D.edges if 'r' not in {u, v}), name='c5') # remove 3-cylcle

    # solve
    model.optimize()

    # Get solutions from solution variables
    H = get_solution_graph(D, x)

    # return solution graph
    modelname = "Base Model"
    return H, modelname