import gurobipy as gp
from gurobipy import GRB


# base model formulation
def base_model_sol(D, b):
    # create model
    model = gp.Model(name="base model")

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

    # solve
    model.optimize()

    # return model
    return model