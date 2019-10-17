# -*- coding: utf-8 -*-
#usar python3 o python?
import numpy as np
from scipy import signal
from sympy import *
from scipy.signal import lti, step, bode, zpk2tf



#### FUNCIONES TC-2017 ESTIGARRIBIA y SALARIATO ####
#Multiplico funciones
def multiplico_sistemas(sys1, sys2):

    if not isinstance(sys1, signal.lti):
        sys1 = signal.lti(*sys1)
    if not isinstance(sys2, signal.lti):
        sys2 = signal.lti(*sys2)
    num = np.polymul(sys1.num, sys2.num)
    den = np.polymul(sys1.den, sys2.den)
    sys = signal.lti(num, den)
    return sys

#Sumo funciones
def sumo_sistemas(sys3, sys4):

    if not isinstance(sys3, signal.lti):
        sys3 = signal.lti(*sys3)
    if not isinstance(sys4, signal.lti):
        sys4 = signal.lti(*sys4)
    num = np.polyadd(sys3.num, sys4.num)
    den = np.polyadd(sys3.den, sys4.den)
    sys = signal.lti(num, den)
    return sys

#Realimento con Multiple Feedback, hay que tener en cuenta cual es la
#realimentacion, tipicamente es 1
#si alpha es el lazo forward y beta es el lazo backwards... N -> numerador; D -> denominador
#alpha = Nalpha / Dalpha
#beta = Nbeta / Dbeta
#syst realimentado = (Nalpha * Dbeta) / (Dalpha * Dbeta + Nalpha * Nbeta)
def realimento (alpha, beta):

    if not isinstance(alpha, signal.lti):
        alpha = signal.lti(*alpha)

    if not isinstance(beta, signal.lti):
        beta = signal.lti(*beta)

    #numerador sistema
    num_sys = np.polymul(alpha.num, beta.den)

    #armo el denominador
    den_sys1 = np.polymul(alpha.den, beta.den)
    den_sys2 = np.polymul(alpha.num, beta.num)
    den_sys = np.polyadd(den_sys1, den_sys2)

    #convierto nuevamente a TF
    sys = signal.lti(num_sys, den_sys)
    return sys


### FUNCONES DE INTERNET ########
from scipy import signal
import sympy as sy

def lti_to_sympy(lsys, symplify=True):
    """ Convert Scipy's LTI instance to Sympy expression """
    s = sy.Symbol('s')
    G = sy.Poly(lsys.num, s) / sy.Poly(lsys.den, s)
    return sy.simplify(G) if symplify else G

def sympy_to_lti(xpr, s=sy.Symbol('s')):
    """ Convert Sympy transfer function polynomial to Scipy LTI """
    num, den = sy.simplify(xpr).as_numer_denom()  # expressions
    p_num_den = sy.poly(num, s), sy.poly(den, s)  # polynomials
    c_num_den = [sy.expand(p).all_coeffs() for p in p_num_den]  # coefficients
    l_num, l_den = [sy.lambdify((), c)() for c in c_num_den]  # convert to floats
    return signal.lti(l_num, l_den)

# #ejemplo de uso
# pG, pH, pGH, pIGH = sy.symbols("G, H, GH, IGH")  # only needed for displaying
#
#
# # Sample systems:
# lti_G = signal.lti([1], [1, 2])
# lti_H = signal.lti([2], [1, 0, 3])
#
# # convert to Sympy:
# Gs = lti_to_sympy(lti_G)
# print (Gs)

# realm = Gs/(1+Gs)
# print (realm)

# Gs, Hs = lti_to_sympy(lti_G), lti_to_sympy(lti_H)
#
# print("Converted LTI expressions:")
# print (sy.Eq(pG, Gs))
# print (sy.Eq(pH, Hs))
#
# print("Multiplying Systems:")
# GHs = sy.simplify(Gs*Hs).expand()  # make sure polynomials are canceled and expanded
# print (sy.Eq(pGH, GHs))
#
#
# print("Closing the loop:")
# IGHs = sy.simplify(GHs / (1+GHs)).expand()
# print (sy.Eq(pIGH, IGHs))
#
# print("Back to LTI:")
# lti_IGH = sympy_to_lti(IGHs)
# print(lti_IGH)

#### PID analogico a PID digital
# def PID_analog_digital (kp=1, ki=1, kd=1, ts=1):
#     #de analogico a digital
#     fs = 1 / ts
#     print ("kp: %d, ki: %d, kd: %d", kp,ki,kd)

#     s = sy.Symbol('s')
#     pid = kp + ki/s + kd*s
#     pid.roots
    
#     ki_dig = ki / fs
#     kp_dig = kp - ki_dig / 2
#     kd_dig = kd * fs

#     k1 = kp_dig + ki_dig + kd_dig
#     k2 = -kp_dig - 2*kd_dig
#     k3 = kd_dig

#     print ("k1: %d, k2: %d, k3: %d",k1,k2,k3)
#     return k1, k2, k3








#### COMO CLASE DERIVADA de lti tf ####
from scipy.signal.ltisys import TransferFunction as TransFun
from numpy import polymul,polyadd

class ltimul(TransFun):
    def __neg__(self):
        return ltimul(-self.num,self.den)

    def __mul__(self,other):
        if type(other) in [int, float]:
            return ltimul(self.num*other,self.den)
        elif type(other) in [TransFun, ltimul]:
            numer = polymul(self.num,other.num)
            denom = polymul(self.den,other.den)
            return ltimul(numer,denom)

    def __truediv__(self,other):
        if type(other) in [int, float]:
            return ltimul(self.num,self.den*other)
        if type(other) in [TransFun, ltimul]:
            numer = polymul(self.num,other.den)
            denom = polymul(self.den,other.num)
            return ltimul(numer,denom)

    def __rtruediv__(self,other):
        if type(other) in [int, float]:
            return ltimul(other*self.den,self.num)
        if type(other) in [TransFun, ltimul]:
            numer = polymul(self.den,other.num)
            denom = polymul(self.num,other.den)
            return ltimul(numer,denom)

    def __add__(self,other):
        if type(other) in [int, float]:
            return ltimul(polyadd(self.num,self.den*other),self.den)
        if type(other) in [TransFun, type(self)]:
            numer = polyadd(polymul(self.num,other.den),polymul(other.den,self.num))
            denom = polymul(self.den,other.den)
            return ltimul(numer,denom)

    def __sub__(self,other):
        if type(other) in [int, float]:
            return ltimul(polyadd(self.num,-self.den*other),self.den)
        if type(other) in [TransFun, type(self)]:
            numer = polyadd(polymul(self.num,other.den),-polymul(other.den,self.num))
            denom = polymul(self.den,other.den)
            return ltimul(numer,denom)

    def __rsub__(self,other):
        if type(other) in [int, float]:
            return ltimul(polyadd(-self.num,self.den*other),self.den)
        if type(other) in [TransFun, type(self)]:
            numer = polyadd(polymul(other.num,self.den),-polymul(self.den,other.num))
            denom = polymul(self.den,other.den)
            return ltimul(numer,denom)

   # sheer laziness: symmetric behaviour for commutative operators
    __rmul__ = __mul__
    __radd__ = __add__
