import gurobipy as gp
from gurobipy import GRB


# base model formulation
def base_model_sol(G, b):
    # create model
    model = gp.Model(name="base model")

    # add variables
    x = model.addVars(G.edges, vtype=GRB.BINARY, name='x')
    y = model.addVars(G.nodes, vtype=GRB.BINARY, name='y')

    # add objective function
    obj_fn = gp.quicksum(G.nodes[v]['revenue'] * y[v] for v in G.nodes)
    model.setObjective(obj_fn, GRB.MAXIMIZE)

    # add constraints
    y['r'].lb = 1
    model.addConstrs((gp.quicksum(x[a] for a in G.in_edges(v)) == y[v] for v in G.nodes if v != 'r'), name='c1')
    model.addConstr((gp.quicksum(G.edges[a]['cost'] * x[a] for a in G.edges) <= b), name='c2')

    # solve
    model.optimize()

    return model