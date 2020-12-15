import numpy as np


class Custom_int:
    def __init__(self, b_params, a_params, divider):
        self.b = np.array(b_params, dtype=np.int32)
        self.a = np.array(a_params, dtype=np.int32)
        self.div = np.int16(divider)
        print(f'b: {self.b} dtype: {self.b.dtype}')
        print(f'a: {self.a} dtype: {self.a.dtype}')
        print(f'divider: {self.div} dtype: {self.div.dtype}')        
        self.resetFilter()

        
    def changeParams(self, b_params, a_params):
        self.b = np.array(b_params, dtype=np.int32)
        self.a = np.array(a_params, dtype=np.int32)
        print(f'params to b: {self.b} dtype: {self.b.dtype}')
        print(f'params to a: {self.a} dtype: {self.a.dtype}')

        
    def resetFilter(self):
        self.error_z1 = np.int16(0)
        self.new_output_z1 = np.int16(0)


    def newOutput(self, new_input):
        error = np.int32(new_input)
        new_output = np.int32(0)
        
        new_output = self.b[0] * error / self.div
        new_output += self.b[1] * self.error_z1 / self.div
        new_output -= self.a[1] * self.new_output_z1

        self.new_output_z1 = np.int16(new_output)
        self.error_z1 = error

        return np.int16(new_output)

    

        
