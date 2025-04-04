# Python 3

import sys
import socketserver
from http.server import SimpleHTTPRequestHandler

DIRECTORY = "build-web"

class WasmHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):        
        # Include additional response headers to allow for use of the 
        # SharedArrayBuffer.
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        SimpleHTTPRequestHandler.end_headers(self)


# Python 3.7.5 adds in the WebAssembly Media Type. If this is an older
# version, add in the Media Type.
if sys.version_info < (3, 7, 5):
    WasmHandler.extensions_map['.wasm'] = 'application/wasm'


if __name__ == '__main__':
    PORT = 8000
    with socketserver.TCPServer(("", PORT), WasmHandler) as httpd:
        print("Listening on port {}. Press Ctrl+C to stop.".format(PORT))
        httpd.serve_forever()
