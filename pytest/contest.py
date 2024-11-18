import pytest
import ctypes

# @pytest.fixture(scope="session", name="big_int_lib")
# def get_big_int_lib() -> ctypes.CDLL:
# big_int_lib = ctypes.CDLL('./test/bigInt.so')
# return big_int_lib

'''
# Dereference the pointer to access the structure
big_int = big_int_instance.contents

# Access the fields of the structure
print("Start:", big_int.start)
print("End:", big_int.end)
print("Array Owner:", big_int.arrayOwner)
print("BigInt Array:", big_int.bigIntArray)
print("Negative:", big_int.negative)
'''
