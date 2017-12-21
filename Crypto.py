from ctypes import cdll
import ctypes
lib = cdll.LoadLibrary('./lib.so')
c_ubyte_p = ctypes.POINTER(ctypes.c_ubyte)


def hash(data):
    """
    result value must be with len 32
    """
    hash_f = lib.wrap_hash
    hash_f.argtypes = [c_ubyte_p, ctypes.c_uint32, c_ubyte_p, ctypes.c_uint32]
    hash_f.restype = ctypes.c_int16
    data_len = len(data)
    data = str(data).encode("utf-8")
    result_hash = str("0"*32).encode("utf-8")
    hash_f(ctypes.cast(data, c_ubyte_p), data_len, ctypes.cast(result_hash, c_ubyte_p), 32)
    return result_hash


def encrypt(data, iv, key):
    """
    args = i, ilen, iv, ivlen, key, keylen, o, olen
    ilen == olen
    ivlen == 16
    keylen = 32
    """


    encrypt_f = lib.wrap_serpent_encrypt
    encrypt_f.argtypes = [c_ubyte_p, ctypes.c_uint32, c_ubyte_p, ctypes.c_uint32,
                    c_ubyte_p, ctypes.c_uint32, c_ubyte_p, ctypes.c_uint32]
    encrypt_f.restype = ctypes.c_int16
    if len(iv) < 16:
        iv = iv + "0"*(16 - len(iv))

    if len(key) < 32:
        key = key + "0"*(32 - len(key))

    data_len = len(data)
    if type(data) is str:
        data = str(data).encode("utf-8")
    result = str("0" * data_len).encode("utf-8")
    if type(iv) is str:
        iv = str(iv[:16]).encode("utf-8")
    if type(key) is str:
        key = str(key[:32]).encode("utf-8")

    encrypt_f(ctypes.cast(data, c_ubyte_p), data_len, ctypes.cast(iv, c_ubyte_p), 16,
              ctypes.cast(key, c_ubyte_p), 32, ctypes.cast(result, c_ubyte_p), data_len)
    return result


def decrypt(data, iv, key):
    """
    args = i, ilen, iv, ivlen, key, keylen, o, olen
    ilen == olen
    ivlen == 16
    keylen = 32

    """
    decrypt_f = lib.wrap_serpent_decrypt
    decrypt_f.argtypes = [c_ubyte_p, ctypes.c_uint32, c_ubyte_p, ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32, c_ubyte_p, ctypes.c_uint32]
    decrypt_f.restype = ctypes.c_int16

    if len(iv) < 16:
        iv = iv + "0" * (16 - len(iv))

    if len(key) < 32:
        key = key + "0" * (32 - len(key))

    data_len = len(data)
    if type(data) is str:
        data = str(data).encode("utf-8")
    result = str("0" * data_len).encode("utf-8")
    if type(iv) is str:
        iv = str(iv[:16]).encode("utf-8")
    if type(key) is str:
        key = str(key[:32]).encode("utf-8")



    decrypt_f(ctypes.cast(data, c_ubyte_p), data_len, ctypes.cast(iv, c_ubyte_p), 16,
              ctypes.cast(key, c_ubyte_p), 32, ctypes.cast(result, c_ubyte_p), data_len)
    return result


def generate_session_key():
    """
    """
    gen_f = lib.wrap_genKey
    gen_f.argtypes = [c_ubyte_p, ctypes.c_uint32]
    gen_f.restype = ctypes.c_int16

    result = str("0" * 32).encode("utf-8")
    gen_f(ctypes.cast(result, c_ubyte_p), 32)

    return result


def generate_keys(level):
    gen_f = lib.wrap_generate_keys
    gen_f.argtypes = [ctypes.c_uint32, c_ubyte_p, ctypes.c_uint32, c_ubyte_p, ctypes.c_uint32]
    gen_f.restype = ctypes.c_int16

    private = str("0" * (level // 4)).encode("utf-8")
    public = str("0" * (level // 2)).encode("utf-8")

    gen_f(level, ctypes.cast(private, c_ubyte_p), level // 4, ctypes.cast(public, c_ubyte_p), level // 2)
    return private, public


def encrypt_asm(level, public_key, session_key, header):
    encrypt_f = lib.wrap_asym_encrypt
    encrypt_f.argtypes = [ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32]
    encrypt_f.restype = ctypes.c_int16

    if len(header) < 16:
        header = header + "0" * (16 - len(header))

    if type(public_key) is str:
        public_key = str(public_key).encode("utf-8")
    result_len = level // 4 + len(session_key) + 16
    result = str("0" * result_len).encode("utf-8")
    if type(session_key) is str:
        session_key = str(session_key).encode("utf-8")
    if type(header) is str:
        header = str(header[:16]).encode("utf-8")

    encrypt_f(level, ctypes.cast(public_key, c_ubyte_p), level // 2,
              ctypes.cast(session_key, c_ubyte_p), len(session_key),
              ctypes.cast(header, c_ubyte_p), 16,
              ctypes.cast(result, c_ubyte_p), result_len)
    return result


def decrypt_asm(level, private_key, session_key, header):
    decrypt_f = lib.wrap_asym_decrypt
    decrypt_f.argtypes = [ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32,
                          c_ubyte_p, ctypes.c_uint32]
    decrypt_f.restype = ctypes.c_int16

    if len(header) < 16:
        header = header + "0" * (16 - len(header))

    if type(private_key) is str:
        private_key = str(private_key).encode("utf-8")
    result_len = len(session_key) - 16 - level // 4
    result = str("0" * result_len).encode("utf-8")
    if type(session_key) is str:
        session_key = str(session_key).encode("utf-8")
    if type(header) is str:
        header = str(header[:16]).encode("utf-8")

    decrypt_f(level, ctypes.cast(private_key, c_ubyte_p), level // 4,
              ctypes.cast(session_key, c_ubyte_p), len(session_key),
              ctypes.cast(header, c_ubyte_p), 16,
              ctypes.cast(result, c_ubyte_p), result_len)
    return result

print(hash("123").hex())