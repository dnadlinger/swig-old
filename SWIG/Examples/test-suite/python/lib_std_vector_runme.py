from lib_std_vector import *

iv = IntVector(4)
for i in range(0,4):
    iv[i] = i

x = average(iv)
y = average([1,2,3,4])

a = half([10,10.5,11,11.5])

dv = DoubleVector(10)
for i in range(0,10):
    dv[i] = i/2.0

halve_in_place(dv)

