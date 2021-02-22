import numpy as np
import matplotlib.pyplot as plt
from recursive_tf import RecursiveTF

# t = np.linspace(0, 1, 40)
t = np.arange(40)

# b_planta = np.array([1.])
# a_planta = np.array([1., -0.05, 0.05])
b_planta = np.array([0.00296 , 0.0101, 0.00276])
a_planta = np.array([1., -1.02798, 1.02185, -0.87036])


# vin_plant = np.ones(t.size)
vin_plant = t
vout_plant_method2 = np.zeros (t.size)    
recur_planta = RecursiveTF(b_planta, a_planta)
for i in range(t.size):
    vout_plant_method2[i] = recur_planta.newOutput(vin_plant[i])
    
print (vout_plant_method2)

fig, ax = plt.subplots()
ax.set_title('Step Planta Digital Recursiva ZOH')
ax.set_ylabel('Tension del Planta')
ax.set_xlabel('Tiempo [s]')
ax.grid()
ax.plot(t, vout_plant_method2, 'y')    
plt.tight_layout()
plt.show()
