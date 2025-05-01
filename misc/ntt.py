# Python file with the fft needed for the SSA implemented in Python. Used for debugging

def reduce_mod_f(x: int, fermat_number: int, fermat_index_loc: int) -> int:
    total_bits = 1 << (fermat_index_loc + 1)  # total bits (2^(fermat_index + 1))
    k = total_bits // 2  # half of total_bits

    # high and low parts of x
    v = x >> (total_bits - k)  # high bits
    u = x & ((1 << (total_bits - k)) - 1)  # low bits

    # Result of reduction
    if v > u:  # if v > u
        tmp = u - v
        res = tmp + fermat_number
    else:
        res = u - v

    # Mask to ensure the result fits within total_bits
    res &= (1 << total_bits) - 1

    return res


def rotate_left_mod_f(x: int, k: int, total_bits: int) -> int:
    if k == 0:
        return x

    k %= total_bits  # make sure k is within total_bits

    # Split x into two parts
    high = x >> (total_bits - k)
    low_mask = (1 << (total_bits - k)) - 1
    low = x & low_mask

    # Rotate left
    res = (low << k) + high

    # Optional: if you want to ensure the result is still within total_bits
    res &= (1 << total_bits) - 1

    return res


def rotate_right_mod_f(x: int, k: int, total_bits: int) -> int:
    if k == 0:
        return x

    k %= total_bits  # ensure k is within total_bits

    # Split x into two parts
    low = x & ((1 << k) - 1)
    high = x >> k

    # Rotate right
    res = (low << (total_bits - k)) + high

    # Mask to totalBits
    res &= (1 << total_bits) - 1

    return res


def add_mod_f(x: int, y: int, fermat_index: int) -> int:
    s = x + y
    max_bitlength = 1 << (fermat_index + 1)  # same as 2^(fermat_index + 1)

    while s.bit_length() > max_bitlength:
        low_mask = (1 << max_bitlength) - 1  # mask to get the low bits
        low = s & low_mask
        high = s >> max_bitlength
        s = low + high

    return s


def sub_mod_f(x: int, y: int, fermat_index: int) -> int:
    rotate_amount = 1 << fermat_index
    total_bits = 1 << (fermat_index + 1)

    rotated = rotate_left_mod_f(y, rotate_amount, total_bits)
    res = add_mod_f(x, rotated, fermat_index)

    return res


def int_to_bin_list(n, length):
    bin_str = format(n, f'0{length}b')
    return list(bin_str)


def bin_list_to_int(bin_list):
    bin_str = ''.join(bin_list)
    return int(bin_str, 2)


def change_element_at_index(original_list, i, new_element):
    return original_list[:i] + [new_element] + original_list[i + 1:]


class ATable:
    def __init__(self, index, value, iteration):
        self.index: list[str] = index
        self.value: int = value
        self.interation: int = iteration

    def __str__(self):
        index_str = ''
        for i in self.index:
            index_str += i + ', '
        index_str = index_str[:-2]
        return f"A[{self.interation}]({index_str}) = {self.value:016b}"

    def str_no_value(self):
        index_str = ''
        for i in self.index:
            index_str += i + ', '
        index_str = index_str[:-2]
        return f"A[{self.interation}]({index_str})"


def fft(input_vector, vector_length, n, m_odd: bool):
    omega = 2 if m_odd else 4
    last_table: list[ATable] = []
    index_length = vector_length.bit_length() - 1

    for i in range(0, vector_length):
        last_table.append(ATable(int_to_bin_list(i, index_length), input_vector[i], 0))

    for i in range(1, n + 2):
        new_table: list[ATable] = []
        for j in range(0, vector_length):
            current_element_index = int_to_bin_list(j, index_length)
            old_one = last_table[bin_list_to_int(change_element_at_index(current_element_index, i - 1, '0'))]
            old_two = last_table[bin_list_to_int(change_element_at_index(current_element_index, i - 1, '1'))]

            # calc x
            tmp: int = 0
            for k in range(0, i - 1):
                tmp += int(current_element_index[k], 2) * 2 ** k
            x = (2 ** ((n + 1) - 1 - (i - 1))) * tmp

            # new_value
            if m_odd:
                old_two_value = rotate_left_mod_f(old_two.value, x, 1 << (n + 1))
            else:
                old_two_value = rotate_left_mod_f(old_two.value, x + 1, 1 << (n + 1))

            sign: str
            new_value: int
            if current_element_index[i - 1] == "0":
                new_value = add_mod_f(old_one.value, old_two_value, n)
                sign = "+"
            else:
                new_value = sub_mod_f(old_one.value, old_two_value, n)
                sign = "-"
            to_append = ATable(current_element_index, new_value, i)
            new_table.append(to_append)

            # print(f"{new_table[j].str_no_value()} = {old_one.str_no_value()} {sign} {old_two.str_no_value()} * {omega} ^ {x}")

        last_table = new_table

    res_vector: list[int] = []
    for i in range(0, vector_length):
        for j in range(0, vector_length):
            reverse_index = list(reversed(last_table[j].index))
            if bin_list_to_int(reverse_index) == i:
                res_vector.append(last_table[j].value)
                break

    return res_vector


def ifft(input_vector, vector_length, n, m_odd: bool):
    omega = 2 if m_odd else 4
    last_table: list[ATable] = []
    index_length = vector_length.bit_length() - 1

    for i in range(0, vector_length):  # also reverse input vector
        index = int_to_bin_list(i, index_length)
        target_index = bin_list_to_int(list(reversed(index)))
        last_table.append(ATable(index, input_vector[target_index], n + 1))

    for i in range(n, 0, -1):  # -1
        new_table: list[ATable] = []
        for j in range(0, vector_length):
            current_element_index = int_to_bin_list(j, index_length)
            old_one = last_table[bin_list_to_int(change_element_at_index(current_element_index, i, '0'))]
            old_two = last_table[bin_list_to_int(change_element_at_index(current_element_index, i, '1'))]

            # calc x
            tmp: int = 0
            for k in range(0, i):
                tmp += int(current_element_index[k], 2) * 2 ** k
            x = (2 ** ((n + 1) - 1 - i)) * tmp

            sign: str
            tmp_value: int
            if current_element_index[i] == "0":
                tmp_value = add_mod_f(old_one.value, old_two.value, n)
                sign = "+"
            else:
                tmp_value = sub_mod_f(old_one.value, old_two.value, n)
                sign = "-"

            tmp_value = rotate_right_mod_f(tmp_value, 1, 1 << (n + 1))

            new_value: int
            to_omega = ""
            if current_element_index[i] == "1":
                to_omega = f"{omega}^{x}"
                if m_odd:
                    new_value = rotate_right_mod_f(tmp_value, x, 1 << (n + 1))
                else:
                    new_value = rotate_right_mod_f(tmp_value, x + 1, 1 << (n + 1))
            else:
                new_value = tmp_value

            to_append = ATable(current_element_index, new_value, i)
            new_table.append(to_append)

            # print(f"{new_table[j].str_no_value()} = 2^(-1){to_omega}({old_one.str_no_value()} {sign} {old_two.str_no_value()})")

        last_table = new_table

    res_vector: list[int] = []
    for i in range(0, vector_length):
        res_vector.append(last_table[i].value)
    return res_vector


'''a = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 14, 3, 6]
b = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 15, 11]
fermat_n = 3
fermat_num = 2 ** (2 ** fermat_n) + 1
size = 16'''

a = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3689, 12794, 51674, 45747,
     27684, 35719, 54958, 13305, 42541, 29059, 42453, 30878, 19245, 27460, 7716, 4572]
b = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3337, 57617, 51169, 59308,
     46840, 51904, 47919, 50376, 48170, 58298, 43705, 5724, 50264, 57753, 52105, 62747]
fermat_n = 5
fermat_num = 2 ** (2 ** fermat_n) + 1
size = 64

a_vec = list(reversed(a))
b_vec = list(reversed(b))
resA = fft(a_vec, size, fermat_n, True)
resB = fft(b_vec, size, fermat_n, True)
print("-:-:-:-")

for it in range(0, size):
    print(f"â{it} = {resA[it]}")
for it in range(0, size):
    print(f"{resA[it]}, ", end="")
print()
print("-:-:-:-")
for it in range(0, size):
    print(f"b^{it} = {resB[it]:016b}")
for it in range(0, size):
    print(f"{resB[it]}, ", end="")
print()
print("-:-:-:-")

# mul
a_reduced = []
for it in range(0, size):
    a_reduced.append(reduce_mod_f(resA[it], fermat_num, fermat_n))
b_reduced = []
for it in range(0, size):
    b_reduced.append(reduce_mod_f(resB[it], fermat_num, fermat_n))

resC = []
for it in range(0, size):
    resC.append(a_reduced[it] * b_reduced[it])

for it in range(0, size):
    print(f"ĉ{it} = {resC[it]:016b}")
print("-:-:-:-")

c = ifft(resC, size, fermat_n, True)

for it in range(0, size):
    print(f"c{it} = {c[it]}")
print("-:-:-:-")

c_reduced = []
for it in range(0, size):
    c_reduced.append(reduce_mod_f(c[it], fermat_num, fermat_n))

for it in range(0, size):
    print(f"c_reduced{it} = {c_reduced[it]}")

for it in range(0, size):
    print(f"{c_reduced[it]}, ", end="")
print()
###

'''iresA = fft(resA, 16, 3, True)
for it in range(0, 16):
    print(f"a{it} = {resA[it]:016b}")

a_reduced = []
for it in range(0, size):
    a_reduced.append(reduce_mod_f(iresA[it], fermat_num, fermat_index))

for it in range(0, size):
    print(f"c_reduced{it} = {a_reduced[it]:08b}")
'''
