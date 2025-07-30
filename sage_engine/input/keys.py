# Key constants for Input module

KEY_SPACE = 32
KEY_LEFT = 37
KEY_UP = 38
KEY_RIGHT = 39
KEY_DOWN = 40

# Alphanumeric keys
KEY_A = 65
KEY_B = 66
KEY_C = 67
KEY_D = 68
KEY_E = 69
KEY_F = 70
KEY_G = 71
KEY_H = 72
KEY_I = 73
KEY_J = 74
KEY_K = 75
KEY_L = 76
KEY_M = 77
KEY_N = 78
KEY_O = 79
KEY_P = 80
KEY_Q = 81
KEY_R = 82
KEY_S = 83
KEY_T = 84
KEY_U = 85
KEY_V = 86
KEY_W = 87
KEY_X = 88
KEY_Y = 89
KEY_Z = 90

KEY_ESCAPE = 27

CODE_TO_NAME = {
    KEY_SPACE: "SPACE",
    KEY_LEFT: "LEFT",
    KEY_RIGHT: "RIGHT",
    KEY_UP: "UP",
    KEY_DOWN: "DOWN",
    KEY_ESCAPE: "ESCAPE",
}
for code in range(KEY_A, KEY_Z + 1):
    CODE_TO_NAME[code] = chr(code)

