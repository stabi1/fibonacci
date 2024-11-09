import ctypes

bigIntLib = ctypes.CDLL('./bigInt.so')

class BigInt(ctypes.Structure):
    _fields_ = [
        ("start", ctypes.c_size_t),            # size_t start
        ("end", ctypes.c_size_t),              # size_t end
        ("arrayOwner", ctypes.c_bool),         # bool arrayOwner
        ("bigIntArray", ctypes.POINTER(ctypes.c_uint64)),  # uint64_t* bigIntArray
        ("negative", ctypes.c_bool)            # bool negative
    ]

bigIntLib.newBigInt.argtypes = [ctypes.c_size_t]
bigIntLib.newBigInt.restype = ctypes.POINTER(BigInt)

bigIntLib.bigIntToHexString.argtypes = [ctypes.POINTER(BigInt)]  # Takes a pointer to BigInt
bigIntLib.bigIntToHexString.restype = ctypes.c_char_p

big_int_instance = bigIntLib.newBigInt(5)
hex_str = bigIntLib.bigIntToHexString(big_int_instance)

print(f"Hex String: {hex_str.decode('utf-8')}")