from ctypes import cdll
lib = cdll.LoadLibrary('./lib.so')

hash = lib.wrap_hash
hash.argtypes = [ctypes.c_char_p]
hash.restype = ctypes.c_uint16