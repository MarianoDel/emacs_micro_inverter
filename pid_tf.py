import numpy as np


class PID_float:
    def __init__(self, b_params, a_params):
        self.b_pid = b_params
        self.a_pid = a_params
        self.resetFilter()

        
    def changeParams(self, b_params, a_params):
        self.b_pid = b_params
        self.a_pid = a_params

        
    def resetFilter(self):
        self.error_z2 = 0.0
        self.error_z1 = 0.0
        self.new_output_z1 = 0.0


    def newOutput(self, new_input):
        error = new_input
        new_output = self.b_pid[0] * error \
                     + self.b_pid[1] * self.error_z1 \
                     + self.b_pid[2] * self.error_z2 \
                     - self.a_pid[1] * self.new_output_z1

        self.new_output_z1 = new_output
        self.error_z2 = self.error_z1
        self.error_z1 = error

        return new_output


class PID_int:
    def __init__(self, b_params, a_params, divider):
        self.b_pid = np.array(b_params, dtype=np.int16)
        self.a_pid = np.array(a_params, dtype=np.int16)
        self.div = np.int16(divider)
        print(f'b_pid: {self.b_pid} dtype: {self.b_pid.dtype}')
        print(f'a_pid: {self.a_pid} dtype: {self.a_pid.dtype}')
        print(f'divider: {self.div} dtype: {self.div.dtype}')        
        self.resetFilter()

        
    def changeParams(self, b_params, a_params):
        self.b_pid = np.array(b_params, dtype=np.int16)
        self.a_pid = np.array(a_params, dtype=np.int16)
        print(f'params to b_pid: {self.b_pid} dtype: {self.b_pid.dtype}')
        print(f'params to a_pid: {self.a_pid} dtype: {self.a_pid.dtype}')

        
    def resetFilter(self):
        self.error_z2 = np.int16(0)
        self.error_z1 = np.int16(0)
        self.new_output_z1 = np.int16(0)


    def newOutput(self, new_input):
        error = np.int16(new_input)
        new_output = np.int16(0)
        new_output = self.b_pid[0] * error / self.div \
                     + self.b_pid[1] * self.error_z1 / self.div \
                     + self.b_pid[2] * self.error_z2 / self.div \
                     - self.a_pid[1] * self.new_output_z1

        self.new_output_z1 = new_output
        self.error_z2 = self.error_z1
        self.error_z1 = error

        return new_output


