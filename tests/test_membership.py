import pytest
import numpy as np
from membership_functions import TriangularMF, TrapezoidalMF, GaussianMF

def test_triangular_mf():
    mf = TriangularMF(a=1.0, b=2.0, c=3.0)
    assert mf.evaluate(0.5) == 0.0
    assert mf.evaluate(1.0) == 0.0
    assert mf.evaluate(1.5) == pytest.approx(0.5)
    assert mf.evaluate(2.0) == 1.0
    assert mf.evaluate(2.5) == pytest.approx(0.5)
    assert mf.evaluate(3.0) == 0.0
    assert mf.evaluate(3.5) == 0.0

def test_triangular_mf_vectorized():
    mf = TriangularMF(a=0.0, b=5.0, c=10.0)
    x = np.array([-1.0, 0.0, 2.5, 5.0, 7.5, 10.0, 12.0])
    expected = np.array([0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0])
    np.testing.assert_allclose(mf.evaluate_array(x), expected)

def test_trapezoidal_mf():
    mf = TrapezoidalMF(a=1.0, b=2.0, c=4.0, d=5.0)
    assert mf.evaluate(0.0) == 0.0
    assert mf.evaluate(1.5) == pytest.approx(0.5)
    assert mf.evaluate(2.0) == 1.0
    assert mf.evaluate(3.0) == 1.0
    assert mf.evaluate(4.0) == 1.0
    assert mf.evaluate(4.5) == pytest.approx(0.5)
    assert mf.evaluate(5.0) == 0.0
    assert mf.evaluate(6.0) == 0.0

def test_gaussian_mf():
    mf = GaussianMF(mean=5.0, sigma=1.0)
    assert mf.evaluate(5.0) == 1.0
    assert mf.evaluate(4.0) == pytest.approx(np.exp(-0.5))
    assert mf.evaluate(6.0) == pytest.approx(np.exp(-0.5))
