import numpy as np


class RecursiveTF:
    def __init__(self, b_params, a_params):
        self.b_params = b_params
        self.a_params = a_params

        self.resetFilter()
        

    def resetFilter(self):
        self.current_index = 0
        self.last_inputs = np.zeros(self.b_params.size)
        self.last_outputs = np.zeros(self.a_params.size)


    def printVects(self):
        print('b_params:')
        print(self.b_params)
        print('a_params:')
        print(self.a_params)
        print('last_inputs:')
        print(self.last_inputs)
        print('last_outputs:')
        print(self.last_outputs)
        

    def newOutput(self, new_input):
        self.last_inputs[0] = new_input
        
        # check max size
        b_size = self.b_params.size
        a_size = self.a_params.size
        if self.current_index < b_size:
            b_size = self.current_index

        if self.current_index < a_size:
            # a_size = self.current_index + 1
            a_size = self.current_index          
            

        new_output = 0.0
        # print(f'new_output index: {self.current_index}')
        for b_index in range(b_size):
            new_output += self.b_params[b_index] * self.last_inputs[b_index]
            # print(f'b: {new_output}')

        for a_index in range(1, a_size):
            new_output -= self.a_params[a_index] * self.last_outputs[a_index]

        # ajusto vectores para prox vuelta
        if b_size < self.b_params.size:
            for b_index in range(b_size, 0, -1):
                self.last_inputs[b_index] = self.last_inputs[b_index - 1]
        else:
            for b_index in range(b_size - 1, 0, -1):
                self.last_inputs[b_index] = self.last_inputs[b_index - 1]
            

        self.last_outputs[0] = new_output
        if a_size < self.a_params.size:
            for a_index in range(a_size, 0, -1):
                self.last_outputs[a_index] = self.last_outputs[a_index - 1]
        else:
            for a_index in range(a_size - 1, 0, -1):
                self.last_outputs[a_index] = self.last_outputs[a_index - 1]

        # ajusto indice interno
        self.current_index += 1
        
        return new_output


