"""
Fuzzy Membership Functions Module
Literature-Grounded EIS Freshness Model
Classification: [ENGINEERING-INFERENCE] / [LITERATURE]
"""

import numpy as np

class MembershipFunction:
    """Base class for fuzzy membership functions."""
    def evaluate(self, x: float) -> float:
        raise NotImplementedError

    def evaluate_array(self, x: np.ndarray) -> np.ndarray:
        return np.array([self.evaluate(float(v)) for v in x])


class TriangularMF(MembershipFunction):
    """
    Triangular Membership Function: trimf(x; a, b, c)
    Parameters:
        a: lower bound where membership is 0
        b: peak where membership is 1
        c: upper bound where membership is 0
    """
    def __init__(self, a: float, b: float, c: float, name: str = ""):
        self.a = float(a)
        self.b = float(b)
        self.c = float(c)
        self.name = name

    def evaluate(self, x: float) -> float:
        x = float(x)
        if x <= self.a or x >= self.c:
            return 0.0
        if x == self.b:
            return 1.0
        if self.a < x < self.b:
            if self.b == self.a:
                return 1.0
            return (x - self.a) / (self.b - self.a)
        if self.b < x < self.c:
            if self.c == self.b:
                return 1.0
            return (self.c - x) / (self.c - self.b)
        return 0.0


class TrapezoidalMF(MembershipFunction):
    """
    Trapezoidal Membership Function: trapmf(x; a, b, c, d)
    Parameters:
        a: lower bound
        b: lower plateau start
        c: upper plateau end
        d: upper bound
    """
    def __init__(self, a: float, b: float, c: float, d: float, name: str = ""):
        self.a = float(a)
        self.b = float(b)
        self.c = float(c)
        self.d = float(d)
        self.name = name

    def evaluate(self, x: float) -> float:
        x = float(x)
        if self.a == self.b and x <= self.b:
            return 1.0
        if self.c == self.d and x >= self.c:
            return 1.0
        if x <= self.a or x >= self.d:
            return 0.0
        if self.b <= x <= self.c:
            return 1.0
        if self.a < x < self.b:
            denom = self.b - self.a
            return (x - self.a) / denom if denom > 1e-9 else 1.0
        if self.c < x < self.d:
            denom = self.d - self.c
            return (self.d - x) / denom if denom > 1e-9 else 1.0
        return 0.0


class GaussianMF(MembershipFunction):
    """
    Gaussian Membership Function: gaussmf(x; mean, sigma)
    """
    def __init__(self, mean: float, sigma: float, name: str = ""):
        self.mean = float(mean)
        self.sigma = float(sigma) if sigma > 0 else 1e-6
        self.name = name

    def evaluate(self, x: float) -> float:
        x = float(x)
        return float(np.exp(-0.5 * ((x - self.mean) / self.sigma) ** 2))
