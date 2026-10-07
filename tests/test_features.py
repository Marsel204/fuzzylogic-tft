import pytest
import numpy as np
from feature_processing import (
    calculate_impedance_ratio,
    calculate_py_parameter,
    temperature_compensate_impedance,
    extract_ad5933_impedance,
    normalize_impedance_to_initial
)

def test_calculate_impedance_ratio():
    assert calculate_impedance_ratio(2000.0, 1000.0) == pytest.approx(2.0)
    assert calculate_impedance_ratio(1000.0, 1000.0) == pytest.approx(1.0)
    with pytest.raises(ValueError):
        calculate_impedance_ratio(1000.0, 0.0)

def test_calculate_py_parameter():
    # Py = (|Z_lf| - |Z_hf|) / |Z_hf|
    assert calculate_py_parameter(2000.0, 1000.0) == pytest.approx(1.0)
    assert calculate_py_parameter(1000.0, 1000.0) == pytest.approx(0.0)
    assert calculate_py_parameter(3000.0, 1000.0) == pytest.approx(2.0)

def test_temperature_compensate_impedance():
    # Tissue temp coeff ~ -1.56%/°C (impedance drops with rising temp)
    # Z_ref = Z_meas * (1 + alpha * (T_meas - T_ref)) where alpha = 0.0156
    z_meas = 1000.0
    t_meas = 14.0
    t_ref = 4.0
    # At higher temp, measured impedance is lower than at 4°C, so compensated should be higher
    z_comp = temperature_compensate_impedance(z_meas, temp_c=t_meas, temp_ref=t_ref, alpha=0.0156)
    assert z_comp == pytest.approx(1000.0 * (1.0 + 0.0156 * 10.0))
    assert z_comp > z_meas

def test_extract_ad5933_impedance():
    # Margo et al. (2013) Eq 4: Zx(f) = (Xv(f) / Xi(f)) * K(f)
    xv = 100.0 + 50.0j
    xi = 20.0 + 10.0j
    k_cal = 500.0 + 0.0j
    zx = extract_ad5933_impedance(xv, xi, k_cal)
    assert np.abs(zx) == pytest.approx(2500.0)
    assert np.angle(zx, deg=True) == pytest.approx(0.0)

def test_normalize_impedance_to_initial():
    z_series = np.array([1200.0, 1000.0, 800.0, 600.0])
    z_norm = normalize_impedance_to_initial(z_series, z_initial=1200.0)
    np.testing.assert_allclose(z_norm, [1.0, 1000.0/1200.0, 800.0/1200.0, 0.5])
