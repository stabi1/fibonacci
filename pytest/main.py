import ctypes
from typing import List, Tuple

from big_int_helper import big_int_lib, get_partial_bigint, trim_string, random_hex_string, BigInt

'''hex_str1 = "1" + "A"*16*3
hex_str2 = "1" + "A"*16*5
print(hex_str1)
print(hex_str2)
res = int(hex_str1, 16) * int(hex_str2, 16)
res_str = hex(res).replace("0x", "")
print(res_str)
print(len(res_str)/16)'''
