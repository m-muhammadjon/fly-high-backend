import socket
import struct
import json

def connect_to_server():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', 65432))
    return sock

def send_to_server(sock, data):
    # Send the custom JSON data
    sock.sendall(data.encode())

    # Read the 4-byte length header
    header = sock.recv(4)
    if len(header) != 4:
        raise Exception("Failed to receive valid length header")
    total_length = struct.unpack('!I', header)[0]
    print(f"Expected response length: {total_length}")

    # Read the full data in larger chunks
    response = b""
    while len(response) < total_length:
        chunk = sock.recv(min(16384, total_length - len(response)))
        if not chunk:
            raise Exception("Connection closed before all data was received")
        response += chunk

    return json.loads(response.decode())

def input_and_send():
    # Get custom JSON data from input
    data = input("Enter JSON data to send to the server: ")

    try:
        # Validate if it's a valid JSON
        json_data = json.loads(data)
    except json.JSONDecodeError:
        print("Invalid JSON data. Please try again.")
        return

    # Connect to the server and send data
    sock = connect_to_server()
    response = send_to_server(sock, json.dumps(json_data))
    print(f"Response from server: {response}")
    sock.close()  # Close the socket after communication
