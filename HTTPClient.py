import http.client
import json
import uuid
from Crypto import *


class Client:
    def __init__(self, level=128, port=3000):
        self.conn = http.client.HTTPConnection("127.0.0.1:" + str(port))
        self.level = level
        self.private_key, self.public_key = generate_keys(self.level)
        self.session_key = None
        self.auth_id = str(uuid.uuid1())
        self.NANS = uuid.uuid4().__int__()

    def send_message(self, params, content_type):
        headers = {'Content-type': content_type, 'Authorization': self.auth_id}
        params.update({'NANS': self.NANS})
        json_foo = json.dumps(params)
        self.NANS += 1
        self.conn.request('POST', '', str(json_foo), headers)
        return self.conn.getresponse()

    def init_msg(self):
        params = {'level-security': self.level, 'public-key': self.public_key.hex()}
        response = self.send_message(params, 'init')
        if int(response.status) == 200:
            self.session_key = decrypt_asm(self.level, self.private_key,
                                           bytes.fromhex(response.read().decode('UTF-8')), str(self.NANS))
            return True
        else:
            print("Error status: "+response.status)
            return False

    def auth_msg(self, login, password):
        params = {'login': login, 'hash': encrypt(hash(password), str(self.NANS), self.session_key).hex()}
        response = self.send_message(params, 'auth')
        if int(response.status) == 200:
            self.session_key = decrypt_asm(self.level, self.private_key,
                                           bytes.fromhex(response.read().decode('UTF-8')), str(self.NANS))
            return True
        else:
            print(response.status)
            return False

    def list_msg(self):
        params = {}
        response = self.send_message(params, 'list')
        files = []
        if int(response.status) == 200:
            files = str(decrypt(bytes.fromhex(response.read().decode('UTF-8')),
                                              str(self.NANS), self.session_key).decode('UTF-8')).split(';')
            return files

        elif int(response.status) == 300:
            self.session_key = decrypt_asm(self.level, self.private_key,
                                           bytes.fromhex(response.read().decode('UTF-8')), str(self.NANS))
            return self.list_msg()

        else:
            print("Error status: " + response.status)
            return files

    def file_msg(self, filename):
        params = {'filename': filename}
        response = self.send_message(params, 'file')
        data = None
        if int(response.status) == 200:
            data = str(decrypt(bytes.fromhex(response.read().decode('UTF-8')),
                                              str(self.NANS), self.session_key).decode('UTF-8'))
            return data

        elif int(response.status) == 300:
            self.session_key = decrypt_asm(self.level, self.private_key,
                                           bytes.fromhex(response.read().decode('UTF-8')), str(self.NANS))

            return self.file_msg(filename)

        else:
            print("Error status: " + response.status)
            return data

    def regenerate_msg(self):
        params = {}
        response = self.send_message(params, 'regenerate')
        if int(response.status) == 200:
            self.session_key = decrypt_asm(self.level, self.private_key,
                                           bytes.fromhex(response.read().decode('UTF-8')), str(self.NANS))
            return True

        elif int(response.status) == 300:
            self.session_key = decrypt_asm(self.level, self.private_key,
                                           bytes.fromhex(response.read().decode('UTF-8')), str(self.NANS))

        else:
            print("Error status: " + response.status)
            return False


if __name__ == '__main__':
    from sys import argv
    print("Connecting.\tPlease wait...")
    if len(argv) == 3:
        client = Client(level=int(argv[1]), port=int(argv[2]))
    else:
        client = Client()

    if client.init_msg():
        print("Connection established\n")
    else:
        print("Connection can't established\n")
        exit()

    print("Please input your login and password:")
    login = input("login: ")
    password = input("password: ")
    if client.auth_msg(login, password):
        print("Successfull authorization!\n")
    else:
        print("Authorization failed!\n")
        exit()

    # User api
    while True:
        print("Input command:\n"
              "\t1. Get list of available files.\n"
              "\t2. Get file.\n"
              "\t3. Re-generate session key.\n"
              "\t4. Exit.")
        try:
            choose = int(input())
        except ValueError:
            print("Please enter the right value(integer)\n")
            continue

        if choose == 1:
            files = client.list_msg()
            print("List of available files:")
            print('\n'.join(files))
            print()
            continue

        if choose == 2:
            filename = input("filename: ")
            data = client.file_msg(filename)
            print(str(filename) + "\n" + data)
            print()
            continue

        if choose == 3:
            res = client.regenerate_msg()
            print("Regenerate keys is successfull done")
            print()
            continue

        if choose == 4:
            print("GoodBye !!")
            exit()

        print("Please enter the right value(integer)\n")

