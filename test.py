import json
import socket

# Define server address and port
server_address = '127.0.0.1'
server_port = 65432

# Create a socket object
print(f"{socket.AF_INET=}, {socket.SOCK_STREAM=}")
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to the server
    client_socket.connect((server_address, server_port))
    print(f"Connected to server at {server_address}:{server_port}")

    # Send data to the server
    message_data = {
        "action": "get_data",
        "data": "Hello, server!"
    }
    message = json.dumps(message_data)
    client_socket.sendall(message.encode('utf-8'))
    print(f"Sent: {message}")

    # Receive response from the server
    response = client_socket.recv(16384)
    print(f"Received: {response=}")
    print(f"Received: {response.decode('utf-8')}")

finally:
    # Close the connection
    client_socket.close()
    print("Connection closed.")
