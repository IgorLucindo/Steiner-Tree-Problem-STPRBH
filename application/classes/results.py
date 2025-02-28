# class for getting and storing results
class Results:
    def __init__(self):
        self.compare_results = []
        self.simplicials_results = []


    # get preprocess results as a variable
    def get_comparison_results(self, D1, D2, flag=False):
        if not flag:
            return
        
        # get decreases between graphs
        vertex_dec = (len(D1.nodes) - len(D2.nodes))/len(D1.nodes)*100
        arc_dec = (len(D1.edges) - len(D2.edges))/len(D1.edges)*100

        # set and append results
        self.compare_results.append([
            len(D1.nodes),
            len(D1.edges),
            len(D2.nodes),
            len(D2.edges),
            vertex_dec,
            arc_dec
        ])

        # return compare results
        return self.compare_results
    

    # get simplicials results
    def get_simplicials_results(self, simplicials_dict, flag=False):
        if not flag:
            return
        
        # set results
        number_of_cliques = len(simplicials_dict)
        number_of_non_profitable_cliques = sum(1 for key, value in simplicials_dict.items() if len(key) + len(value['non_profitable_simplicials']) == value['number_of_vertices'])
        result = [number_of_cliques, number_of_non_profitable_cliques]

        # append results
        self.simplicials_results.append(result)

        # return simplicials results
        return self.simplicials_results