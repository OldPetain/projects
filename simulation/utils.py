def nm_to_m(nm):
    return nm * 1e-9

def um_to_m(um):
    return um * 1e-6

def validate_positive(value, name="参数"):
    val = float(value)
    if val <= 0:
        raise ValueError(f"{name} 必须为正数")
    return val
