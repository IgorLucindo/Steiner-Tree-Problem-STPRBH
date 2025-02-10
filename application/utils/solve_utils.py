import networkx as nx
import gurobipy as gp
from gurobipy import GRB


# base model formulation
def base_model_sol(G, b):
    # get variables
    V = list(G.nodes)
    A = list(G.edges)
    c = nx.get_edge_attributes(G, 'cost')
    p = nx.get_node_attributes(G, 'revenue')

    # create model
    model = gp.Model(name="base model")

    # add variables
    x = model.addVars(A, vtype=GRB.BINARY, name='x')
    y = model.addVars(V, vtype=GRB.BINARY, name='y')

    # add objective function
    obj_fn = gp.quicksum(p[v] * y[v] for v in V)
    model.setObjective(obj_fn, GRB.MAXIMIZE)

    # add constraints
    model.addConstr(y['r'] == 1, name='c1')
    model.addConstrs((gp.quicksum(x[a] for a in list(G.in_edges(v))) == y[v] for v in V if v != 'r'), name='c2')
    model.addConstr((gp.quicksum(c[a] * x[a] for a in A) <= b), name='c3')

    # solve
    model.optimize()

    return model