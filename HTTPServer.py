from http.server import BaseHTTPRequestHandler, HTTPServer
import logging
import json
import os
from os import listdir
from os.path import isfile, join
from Crypto import *

users_auth ={"max": "bfd1b193a3e3f1b85b2f813b5924cf4f4f68f74dcc0fc49aa3b24831ee42b262",
             "user": "2bf5d688ce9abd019bcf1af9e107ea2e3757f226f321bbb0f41f753694c75255"}

class Service:
    def __init__(self, level, public_key, NANS):
        self.path = "/home/levitan/Store/"
        self.auth = False
        self.NANS_start = NANS
        self.NANS = NANS + 1
        self.level = level
        self.public_key = public_key
        self.session_key = generate_session_key()
        print(self.session_key)

    def check_NANS(self, NANS):
        val = NANS == self.NANS
        self.NANS += 1
        return val

    def check_time(self, NANS):
        if NANS > self.NANS_start + 5:
            self.regenerate_session_key()
            self.NANS_start = NANS
            return False
        return True

    def regenerate_session_key(self):
        self.session_key = generate_session_key()

    def get_session_key(self):
        return encrypt_asm(self.level, self.public_key, self.session_key, str(self.NANS))

    def authorization(self, login, hash):
        hash = str(decrypt(hash, str(self.NANS), self.session_key).hex())
        hash_ = users_auth.get(login, None)
        if hash_ is None or hash_ != hash:
            val = False
        else:
            val = True

        if val is True:
            self.auth = True
            self.regenerate_session_key()
            return True
        else:
            return False

    def get_list(self):
        try:
            return ';'.join([f for f in listdir(self.path) if isfile(join(self.path, f))])
        except _:
            return None

    def get_file(self, filename):
        try:
            file = open(self.path + '/' + filename, 'rb')
            if file is None:
                return None
            data = file.read()
            file.close()
            return data
        except _:
            return None


service_map = {}


class HTTPRequestHandler(BaseHTTPRequestHandler):
    def _set_response(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def _set_regen(self):
        self.send_response(300)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def _set_error(self, error_str="error"):
        self.send_response(204)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def do_GET(self):
        logging.info("GET request, \nPath: %s\nHeaders:\n%s\n", str(self.path), str(self.headers))
        self._set_response()
        self.wfile.write("GET request for {}".format(self.path).encode('UTF-8'))

    def do_POST(self):
        content_len = int(self.headers['Content-Length'])
        content_type = self.headers['Content-type']
        content_id = self.headers['Authorization']
        post_data = self.rfile.read(content_len)
        logging.info("POST request, \nPath: %s\nHeaders:\n%sBody:\n%s\n",
                     str(self.path), str(self.headers), post_data.decode('UTF-8'))
        data = json.loads(post_data)
        self.payload(content_id, content_type, data)

    # PAYLOAD

    def payload(self, content_id, content_type, params):
        func = {
            'init': self.init_msg,
            'auth': self.auth_msg,
            'list': self.list_msg,
            'file': self.file_msg,
            'regenerate': self.regenerate_msg
        }.get(content_type)
        response = func(content_id, params)
        logging.info("Response {}".format(str(response)))
        self.wfile.write(str(response).encode('UTF-8'))

    def init_msg(self, content_id,  params):
        if params.get('level-security', None) is None or params.get('public-key', None) is None \
                                                        or params.get('NANS', None) is None:
            self._set_error()
            return
        service = service_map.get(content_id, None)
        if service is None:
            service = Service(level=int(params.get('level-security', 128)),
                              public_key=bytes.fromhex(params.get('public-key', None)),
                              NANS=params.get('NANS'))
            service_map.update({content_id: service})

        self._set_response()
        return service.get_session_key().hex()

    def auth_msg(self, content_id, params):
        service = service_map.get(content_id, None)
        if params.get('login', None) is None or params.get('hash', None) is None or params.get('NANS', None) is None\
                or service is None or service.check_NANS(params.get('NANS')) is False:
            self._set_error()
            return

        if service.authorization(login=params.get('login'), hash=bytes.fromhex(params.get('hash'))) is False:
            self._set_error()
            return ""
        else:
            self._set_response()
            return service.get_session_key().hex()

    def list_msg(self, content_id, params):
        service = service_map.get(content_id, None)
        if service is None or service.auth is False or params.get('NANS', None) is None \
                or service.check_NANS(params.get('NANS')) is False:
            self._set_error()
            return

        if service.check_time(params.get('NANS')) is False:
            self._set_regen()
            return service.get_session_key().hex()

        var = service.get_list()
        if var is None:
            self._set_error()
            return
        self._set_response()
        return encrypt(var, str(service.NANS), service.session_key).hex()

    def file_msg(self, content_id, params):
        service = service_map.get(content_id, None)
        if service is None or service.auth is False or params.get('filename', None) is None \
                or params.get('NANS', None) is None or service.check_NANS(params.get('NANS')) is False:
            self._set_error()
            return

        if service.check_time(params.get('NANS')) is False:
            self._set_regen()
            return service.get_session_key().hex()

        var = service.get_file(params.get('filename'))

        if var is None:
            self._set_error()
            return

        self._set_response()
        return encrypt(var, str(service.NANS), service.session_key).hex()

    def regenerate_msg(self, content_id, params):
        service = service_map.get(content_id, None)
        if service is None or service.auth is False or params.get('NANS', None) is None \
                or service.check_NANS(params.get('NANS')) is False:
            self._set_error()
            return
        service.regenerate_session_key()
        self._set_response()
        return service.get_session_key().hex()


def run(server_class=HTTPServer, handler_class=HTTPRequestHandler, port=3000):
    logging.basicConfig(level=logging.INFO)
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    logging.info("Starting httpd...\n")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    logging.info("Stopping httpd...\n")


if __name__ == '__main__':
    from sys import argv

    if len(argv) == 2:
        run(port=int(argv[1]))
    else:
        run()
