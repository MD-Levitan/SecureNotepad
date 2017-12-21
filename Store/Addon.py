

class Padding:

    def __init__(self):
        pass

    @staticmethod
    def do_padding(msg, type):
        if type == 1:
            return Padding.do_padding_null_with_one(msg)
        if type == 2:
            return Padding.do_padding_null_with_number(msg)
        return Padding.do_padding_null_with_one(msg)

    @staticmethod
    def undo_padding(msg, type):
        if type == 1:
            return Padding.undo_padding_null_with_one(msg)
        if type == 2:
            return Padding.undo_padding_null_with_number(msg)
        return Padding.undodo_padding_null_with_one(msg)

    @staticmethod
    def do_padding_null_with_one(array, len_block=8):
        array=list(array)
        dif_array=len_block-len(array)%len_block
        if dif_array == 8:
            return [array,False]
        array=array+[128]+[0]*(dif_array-1)
        return [array,True]

    @staticmethod
    def undo_padding_null_with_one(array,len_block=8):
        array = list(array)
        index=list(reversed(array)).index(128)
        return array[0:len(array)-index-1]

    @staticmethod
    def padding_null_with_number(array, len_block=8):
        array = list(array)
        dif_array = len(array) % len_block
        if dif_array == 8:
            return [array, False]
        array += [0] * dif_array
        array += [0]*7 + [dif_array*8]
        return [array, True]


class Translater:

    @staticmethod
    def from_string_to_intarray(line):
        return [ord(x) for x in str(line)]

    @staticmethod
    def from_intarray_to_string(intlist):
        return "".join([chr(x) for x in intlist])

    @staticmethod
    def get_last_bites(bytearray,num):
        line = (bin(bytearray))[2:]
        return int(line[len(line)-num:], 2)

    @staticmethod
    def from_bytarray_to_intarray(array):
        intarray = []
        for i in range(0,len(array)):
            intarray +=[Translater.get_byte(array[i],j) for j in range(0,4)]
        return intarray

    @staticmethod
    def from_64bit_to_intarray(array):
        intarray = [Translater.get_byte(array, j) for j in range(0, 8)]
        return intarray

    @staticmethod
    def get_byte(bytearray, num):
        line = (bin(bytearray))[2:]
        line = '0' * (32 - len(line)) + line
        return  int(line[(num % 4) * 8:(num % 4) * 8 + 8], 2)

    @staticmethod
    def rotate_shift_left(bytearray, num):
        line = (bin(bytearray))[2:]
        line = '0' * (32 - len(line)) + line
        line_end = line[0:num]
        line=line[num:]+line_end
        return int(line, 2)

    @staticmethod
    def mode_32(num):
        return num%(2**32)

    @staticmethod
    def mode_64(num):
        return num % (2 ** 64)

    # @staticmethod
    # def xor_intarray_64(x,y):
    #     if len(x) != len(y) != 8:
    #         raise Exception
    #     print x
    #     x = (x[0] << 56) + (x[1] << 48) + (x[2] << 40) + (x[3] << 32) + (x[4] << 24) + (x[5] << 16) + (x[6] << 8) + x[7]
    #     print x
    #     print y
    #     y = (y[0] << 56) + (y[1] << 48) + (y[2] << 40) + (y[3] << 32) + (y[4] << 24) + (y[5] << 16) + (y[6] << 8) + y[7]
    #     print y
    #     print x^y
    #     return Translater.from_64bit_to_intarray(x^y)

    @staticmethod
    def xor_intarray_64(x,y):
        return [x^y for x,y in zip(x,y) ]

    @staticmethod
    def sum_intarray_64(x, y):
        x = (x[0] << 56) + (x[1] << 48) + (x[2] << 40) + (x[3] << 32) + (x[4] << 24) + (x[5] << 16) + (x[6] << 8) + x[7]
        if type(y) == int:
            return Translater.from_64bit_to_intarray(Translater.mode_64(x + y))
        y = (y[0] << 56) + (y[1] << 48) + (y[2] << 40) + (y[3] << 32) + (y[4] << 24) + (y[5] << 16) + (y[6] << 8) + y[7]
        return Translater.from_64bit_to_intarray(Translater.mode_64(x + y))