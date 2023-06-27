import socket

def send_chunked_request(host, port):
    # Datos de la solicitud multichunk
    chunks = [
        b'Chunked',
        b'Encoding',
        b'Example',
        b''
    ]

    # Conexión con el servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))

    # Construcción y envío de la solicitud multichunk
    request = 'POST /data HTTP/1.1\r\n'
    request += 'Host: {}\r\n'.format(host)
    request += 'Transfer-Encoding: chunked\r\n'
    request += '\r\n'

    for chunk in chunks:
        chunk_size = hex(len(chunk))[2:] + '\r\n'
        request += chunk_size.encode() + chunk + b'\r\n'

    request += '0\r\n\r\n'.encode()

    client_socket.sendall(request.encode())

    # Recepción de la respuesta del servidor
    response = b''
    while True:
        data = client_socket.recv(1024)
        if not data:
            break
        response += data

    print(response.decode())

    # Cierre de la conexión
    client_socket.close()

# Ejemplo de uso
host = 'localhost'
port = 80
send_chunked_request(host, port)

