from Crypto import Random
from S_blocks import S1,S2,S3,S4,S5,S6,S7,S8
from Addon import Translater, Padding

class key_generator:

    def __init__(self,len):
        if len%8:
            raise Exception()
        key = Random.get_random_bytes(16)
        key = [ord(x) for x in key]
        self.key = [(key[0+i*4] << 24) + (key[1+i*4] << 16) + (key[2+i*4]<<8) + key[3+i*4] for i in range(0,4)]
        self.subkeys=self.generate_subkeys()

    @staticmethod
    def get_byte(bytearray, part):
        line = (bin(bytearray[part // 4]))[2:]
        line = '0' * (32 - len(line)) + line
        return int(line[(part % 4) * 8:(part % 4) * 8 + 8], 2)

    def generate_subkeys(self):
        """ Function generates from key(128bit) 32 subkeys(32bit)"""
        k=[0]*32
        z=[0]*4
        x=self.key
        for i in (0,1):
            z[0] = x[0] ^ S5[self.get_byte(x, 0xD)] ^ S6[self.get_byte(x, 0xF)] ^ S7[self.get_byte(x, 0xC)] ^ S8[self.get_byte(x, 0xE)] ^ S7[self.get_byte(x, 0x8)];
            z[1] = x[2] ^ S5[self.get_byte(z, 0x0)] ^ S6[self.get_byte(z, 0x2)] ^ S7[self.get_byte(z, 0x1)] ^ S8[self.get_byte(z, 0x3)] ^ S8[self.get_byte(x, 0xA)];
            z[2] = x[3] ^ S5[self.get_byte(z, 0x7)] ^ S6[self.get_byte(z, 0x6)] ^ S7[self.get_byte(z, 0x5)] ^ S8[self.get_byte(z, 0x4)] ^ S5[self.get_byte(x, 0x9)];
            z[3] = x[1] ^ S5[self.get_byte(z, 0xA)] ^ S6[self.get_byte(z, 0x9)] ^ S7[self.get_byte(z, 0xB)] ^ S8[self.get_byte(z, 0x8)] ^ S6[self.get_byte(x, 0xB)];

            k[0 + i * 16] = S5[self.get_byte(z, 0x8)] ^ S6[self.get_byte(z, 0x9)] ^ S7[self.get_byte(z, 0x7)] ^ S8[self.get_byte(z, 0x6)] ^ S5[self.get_byte(z, 0x2)];
            k[1 + i * 16] = S5[self.get_byte(z, 0xA)] ^ S6[self.get_byte(z, 0xB)] ^ S7[self.get_byte(z, 0x5)] ^ S8[self.get_byte(z, 0x4)] ^ S6[self.get_byte(z, 0x6)];
            k[2 + i * 16] = S5[self.get_byte(z, 0xC)] ^ S6[self.get_byte(z, 0xD)] ^ S7[self.get_byte(z, 0x3)] ^ S8[self.get_byte(z, 0x2)] ^ S7[self.get_byte(z, 0x9)];
            k[3 + i * 16] = S5[self.get_byte(z, 0xE)] ^ S6[self.get_byte(z, 0xF)] ^ S7[self.get_byte(z, 0x1)] ^ S8[self.get_byte(z, 0x0)] ^ S8[self.get_byte(z, 0xC)];

            x[0] = z[2] ^ S5[self.get_byte(z, 0x5)] ^ S6[self.get_byte(z, 0x7)] ^ S7[self.get_byte(z, 0x4)] ^ S8[self.get_byte(z, 0x6)] ^ S7[self.get_byte(z, 0x0)];
            x[1] = z[0] ^ S5[self.get_byte(x, 0x0)] ^ S6[self.get_byte(x, 0x2)] ^ S7[self.get_byte(x, 0x1)] ^ S8[self.get_byte(x, 0x3)] ^ S8[self.get_byte(z, 0x2)];
            x[2] = z[1] ^ S5[self.get_byte(x, 0x7)] ^ S6[self.get_byte(x, 0x6)] ^ S7[self.get_byte(x, 0x5)] ^ S8[self.get_byte(x, 0x4)] ^ S5[self.get_byte(z, 0x1)];
            x[3] = z[3] ^ S5[self.get_byte(x, 0xA)] ^ S6[self.get_byte(x, 0x9)] ^ S7[self.get_byte(x, 0xB)] ^ S8[self.get_byte(x, 0x8)] ^ S6[self.get_byte(z, 0x3)];

            k[4 + i * 16] = S5[self.get_byte(x, 0x3)] ^ S6[self.get_byte(x, 0x2)] ^ S7[self.get_byte(x, 0xC)] ^ S8[self.get_byte(x, 0xD)] ^ S5[self.get_byte(x, 0x8)];
            k[5 + i * 16] = S5[self.get_byte(x, 0x1)] ^ S6[self.get_byte(x, 0x0)] ^ S7[self.get_byte(x, 0xE)] ^ S8[self.get_byte(x, 0xF)] ^ S6[self.get_byte(x, 0xD)];
            k[6 + i * 16] = S5[self.get_byte(x, 0x7)] ^ S6[self.get_byte(x, 0x6)] ^ S7[self.get_byte(x, 0x8)] ^ S8[self.get_byte(x, 0x9)] ^ S7[self.get_byte(x, 0x3)];
            k[7 + i * 16] = S5[self.get_byte(x, 0x5)] ^ S6[self.get_byte(x, 0x4)] ^ S7[self.get_byte(x, 0xA)] ^ S8[self.get_byte(x, 0xB)] ^ S8[self.get_byte(x, 0x7)];

            z[0] = x[0] ^ S5[self.get_byte(x, 0xD)] ^ S6[self.get_byte(x, 0xF)] ^ S7[self.get_byte(x, 0xC)] ^ S8[self.get_byte(x, 0xE)] ^ S7[self.get_byte(x, 0x8)];
            z[1] = x[2] ^ S5[self.get_byte(z, 0x0)] ^ S6[self.get_byte(z, 0x2)] ^ S7[self.get_byte(z, 0x1)] ^ S8[self.get_byte(z, 0x3)] ^ S8[self.get_byte(x, 0xA)];
            z[2] = x[3] ^ S5[self.get_byte(z, 0x7)] ^ S6[self.get_byte(z, 0x6)] ^ S7[self.get_byte(z, 0x5)] ^ S8[self.get_byte(z, 0x4)] ^ S5[self.get_byte(x, 0x9)];
            z[3] = x[1] ^ S5[self.get_byte(z, 0xA)] ^ S6[self.get_byte(z, 0x9)] ^ S7[self.get_byte(z, 0xB)] ^ S8[self.get_byte(z, 0x8)] ^ S6[self.get_byte(x, 0xB)];

            k[8 + i * 16] = S5[self.get_byte(z, 0x3)] ^ S6[self.get_byte(z, 0x2)] ^ S7[self.get_byte(z, 0xC)] ^ S8[self.get_byte(z, 0xD)] ^ S5[self.get_byte(z, 0x9)];
            k[9 + i * 16] = S5[self.get_byte(z, 0x1)] ^ S6[self.get_byte(z, 0x0)] ^ S7[self.get_byte(z, 0xE)] ^ S8[self.get_byte(z, 0xF)] ^ S6[self.get_byte(z, 0xC)];
            k[10 + i * 16] = S5[self.get_byte(z, 0x7)] ^ S6[self.get_byte(z, 0x6)] ^ S7[self.get_byte(z, 0x8)] ^ S8[self.get_byte(z, 0x9)] ^ S7[self.get_byte(z, 0x2)];
            k[11 + i * 16] = S5[self.get_byte(z, 0x5)] ^ S6[self.get_byte(z, 0x4)] ^ S7[self.get_byte(z, 0xA)] ^ S8[self.get_byte(z, 0xB)] ^ S8[self.get_byte(z, 0x6)];

            x[0] = z[2] ^ S5[self.get_byte(z, 0x5)] ^ S6[self.get_byte(z, 0x7)] ^ S7[self.get_byte(z, 0x4)] ^ S8[self.get_byte(z, 0x6)] ^ S7[self.get_byte(z, 0x0)];
            x[1] = z[0] ^ S5[self.get_byte(x, 0x0)] ^ S6[self.get_byte(x, 0x2)] ^ S7[self.get_byte(x, 0x1)] ^ S8[self.get_byte(x, 0x3)] ^ S8[self.get_byte(z, 0x2)];
            x[2] = z[1] ^ S5[self.get_byte(x, 0x7)] ^ S6[self.get_byte(x, 0x6)] ^ S7[self.get_byte(x, 0x5)] ^ S8[self.get_byte(x, 0x4)] ^ S5[self.get_byte(z, 0x1)];
            x[3] = z[3] ^ S5[self.get_byte(x, 0xA)] ^ S6[self.get_byte(x, 0x9)] ^ S7[self.get_byte(x, 0xB)] ^ S8[self.get_byte(x, 0x8)] ^ S6[self.get_byte(z, 0x3)];

            k[12 + i * 16] = S5[self.get_byte(x, 0x8)] ^ S6[self.get_byte(x, 0x9)] ^ S7[self.get_byte(x, 0x7)] ^ S8[self.get_byte(x, 0x6)] ^ S5[self.get_byte(x, 0x3)];
            k[13 + i * 16] = S5[self.get_byte(x, 0xA)] ^ S6[self.get_byte(x, 0xB)] ^ S7[self.get_byte(x, 0x5)] ^ S8[self.get_byte(x, 0x4)] ^ S6[self.get_byte(x, 0x7)];
            k[14 + i * 16] = S5[self.get_byte(x, 0xC)] ^ S6[self.get_byte(x, 0xD)] ^ S7[self.get_byte(x, 0x3)] ^ S8[self.get_byte(x, 0x2)] ^ S7[self.get_byte(x, 0x8)];
            k[15 + i * 16] = S5[self.get_byte(x, 0xE)] ^ S6[self.get_byte(x, 0xF)] ^ S7[self.get_byte(x, 0x1)] ^ S8[self.get_byte(x, 0x0)] ^ S8[self.get_byte(x, 0xD)];

        return k


class ciphertext:

    def __init__(self, line, padding, IV=None):
        self.line = line
        self.padding = padding
        self.IV = IV


class CAST_128:
    '''
        Class generates key using Crypto.Random, and then generate 32 subkeys, using S-blocks (5-8), from  key.
        Message = 64 bites(bit) = 8 bytes(by)
        L(left part of message) = 32 bit = 4 by
        R(right -||-) = 32 bit = 4 by
        Key = 128 bit = 16 by
        Key_R = 32 bit = 4 by
        Key_M = 32 bit = 4 by
        '''

    def __init__(self, padding=1, mode=1):
        self.type_padding = padding
        self.type_mode=mode
        self.key_generator = key_generator(128)
        self.key = self.key_generator.key
        self.keyM = self.key_generator.subkeys[0:16]
        self.keyR = self.key_generator.subkeys[16:]


    def generate_IV(self, nbyte=8):
        """nbyte - number of bytes, function generates IV, using Crypto."""
        kr = Random.get_random_bytes(nbyte)
        return  [ord(x) for x in kr]

    def encrypt(self, message):
        if self.type_mode == 1 :
            return self.encrypt_ECB_mode(message)
        if self.type_mode == 2 :
            return self.encrypt_CFB_mode(message)
        if self.type_mode == 3 :
            return  self.encrypt_CBC_mode(message)
        return self.ecncrypt_CTR_mode(message)

    def decrypt(self, message, padding, IV=None):
        if self.type_mode == 1 :
            return self.decrypt_ECB_mode(message, padding)
        IV=Translater.from_string_to_intarray(IV)
        if self.type_mode == 2 :
            return self.decrypt_CFB_mode(message, padding, IV)
        if self.type_mode == 3 :
            return  self.decrypt_CBC_mode(message, padding, IV)
        return self.decrypt_CTR_mode(message, padding, IV)


    def encrypt_ECB_mode(self, message):
        array=Padding.do_padding(Translater.from_string_to_intarray(message), self.type_padding)
        paddingBoolean = array[1]
        array=array[0]
        encrypt_array=[]
        print array
        for t in range(0,len(array),8):
            encrypt_array += self.run(array[0+t:8+t],False)

        return ciphertext(Translater.from_intarray_to_string(encrypt_array),paddingBoolean)

    def encrypt_CFB_mode(self, message):
        IV = self.generate_IV()
        Y = IV
        array = Padding.do_padding(Translater.from_string_to_intarray(message), self.type_padding)
        paddingBoolean = array[1]
        encrypt_array=[]
        array = array[0]
        for t in range(0, len(array), 8):
            Y = Translater.xor_intarray_64(array[0 + t:8 + t],self.run(Y))
            encrypt_array += Y
        return ciphertext(Translater.from_intarray_to_string(encrypt_array),
                paddingBoolean, Translater.from_intarray_to_string(IV))

    def encrypt_CBC_mode(self, message):
        Y = IV = self.generate_IV()
        array = Padding.do_padding(Translater.from_string_to_intarray(message), self.type_padding)
        paddingBoolean = array[1]
        encrypt_array = []
        array = array[0]
        for t in range(0, len(array), 8):
            Y = self.run(Translater.xor_intarray_64(array[0 + t:8 + t],Y))
            encrypt_array += Y
        return ciphertext(Translater.from_intarray_to_string(encrypt_array),
                paddingBoolean, Translater.from_intarray_to_string(IV))

    def ecncrypt_CTR_mode(self, message):
        IV = self.generate_IV()
        array = Padding.do_padding(Translater.from_string_to_intarray(message), self.type_padding)
        paddingBoolean = array[1]
        encrypt_array = []
        array = array[0]
        S=self.run(IV)
        for t in range(0, len(array), 8):
            S=Translater.sum_intarray_64(S,1)
            encrypt_array += Translater.xor_intarray_64(array[0 + t:8 + t],self.run(S))
        return ciphertext(Translater.from_intarray_to_string(encrypt_array),
                          paddingBoolean, Translater.from_intarray_to_string(IV))


    def decrypt_ECB_mode(self, message,padding):
        array=Translater.from_string_to_intarray(message)
        decrypt_array=[]
        for t in range(0,len(array),8):
            decrypt_array+=self.run(array[0+t:8+t],True)
        decrypt_array = Translater.from_bytarray_to_intarray(decrypt_array)

        if padding:
           decrypt_array=Padding.undo_padding(decrypt_array,self.type_padding)
        return Translater.from_intarray_to_string(decrypt_array)

    def decrypt_CFB_mode(self, message, padding, IV):
        array = Translater.from_string_to_intarray(message)
        Y_prev = IV
        decrypt_array = []
        for t in range(0, len(array), 8):
            Y=array[0 + t:8 + t]
            decrypt_array +=Translater.xor_intarray_64(Y, self.run(Y_prev))
            Y_prev=Y
        if padding:
            decrypt_array = Padding.undo_padding(decrypt_array, self.type_padding)
        return Translater.from_intarray_to_string(decrypt_array)

    def decrypt_CBC_mode(self, message, padding, IV):
        array = Translater.from_string_to_intarray(message)
        decrypt_array = []
        Y_prev = IV
        for t in range(0, len(array), 8):
            Y = array[0 + t:8 + t]
            decrypt_array += Translater.xor_intarray_64(Y_prev, self.run(Y,True))
            Y_prev = Y


        if padding:
            decrypt_array = Padding.undo_padding(decrypt_array, self.type_padding)
        return Translater.from_intarray_to_string(decrypt_array)

    def decrypt_CTR_mode(self, message, padding, IV):
        array = Translater.from_string_to_intarray(message)
        decrypt_array = []
        S = self.run(IV)
        for t in range(0, len(array), 8):
            S = Translater.sum_intarray_64(S, 1)
            decrypt_array += Translater.xor_intarray_64(array[0 + t:8 + t], self.run(S))

        if padding:
            decrypt_array = Padding.undo_padding(decrypt_array, self.type_padding)
        return Translater.from_intarray_to_string(decrypt_array)

    def non_identical_rounds(self, D, roundn):
        """Type 1:  I = ((Kmi + D) <<< Kri)
                f = ((S1[Ia] ^ S2[Ib]) - S3[Ic]) + S4[Id]

            Type 2:  I = ((Kmi ^ D) <<< Kri)
                f = ((S1[Ia] - S2[Ib]) + S3[Ic]) ^ S4[Id]

            Type 3:  I = ((Kmi - D) <<< Kri)
                f = ((S1[Ia] + S2[Ib]) ^ S3[Ic]) - S4[Id]"""

        if roundn+1 in (1, 4, 7, 10, 13, 16):
            return self.f1(D, roundn)
        if roundn+1 in (2, 5, 8, 11, 14):
            return self.f2(D,roundn)
        return  self.f3(D,roundn)

    def f1(self, D, roundn):
        I = Translater.rotate_shift_left(Translater.mode_32(self.keyM[roundn] + D), Translater.get_last_bites(self.keyR[roundn],5))
        return Translater.mode_32(((S1[Translater.get_byte(I, 0x0)] ^ S2[Translater.get_byte(I, 0x1)]) -
                 S3[Translater.get_byte(I, 0x2)]) + S4[Translater.get_byte(I, 0x3)])

    def f2(self, D, roundn):
        I = Translater.rotate_shift_left(Translater.mode_32(self.keyM[roundn] ^ D), Translater.get_last_bites(self.keyR[roundn],5))
        return Translater.mode_32(((S1[Translater.get_byte(I, 0x0)] - S2[Translater.get_byte(I, 0x1)]) +
                S3[Translater.get_byte(I, 0x2)]) ^ S4[Translater.get_byte(I, 0x3)])

    def f3(self, D, roundn):
        I = Translater.rotate_shift_left(Translater.mode_32(self.keyM[roundn] - D), Translater.get_last_bites(self.keyR[roundn],5))
        return Translater.mode_32(((S1[Translater.get_byte(I, 0x0)] + S2[Translater.get_byte(I, 0x1)]) ^
                S3[Translater.get_byte(I, 0x2)]) - S4[Translater.get_byte(I, 0x3)])


    def run(self,message,reverse=False):
        """Function which encrypt or decrypt message(it depends on the value of var reverse(Boolean)), using Feistel's net."""
        L = message[:4]
        L = (L[0] << 24) + (L[1] << 16) + (L[2] << 8) + L[3]
        R = message[4:]
        R = (R[0] << 24) + (R[1] << 16) + (R[2] << 8) + R[3]
        L_next=R_next=0
        for i in range(0,16):
            L_next=R
            if reverse:
                R_next = L ^ self.non_identical_rounds(R,15-i)
            else:
                R_next=L^self.non_identical_rounds(R, i)
            L=L_next
            R=R_next
        return Translater.from_bytarray_to_intarray([R, L])



CS=CAST_128(mode=1)
x = (CS.encrypt("Function which encrypt or decrypt message(it "))
print x.line
print CS.decrypt(x.line,x.padding,x.IV)
