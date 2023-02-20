import socket
import time

# Initialize the client socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('localhost', 8080))

# Initialize variables
seq_num = 0
window_size = 4
messages = ["Hello", "World", "How", "Are", "You", "Today", "Fine", "Thank", "You", "Goodbye", None]

while messages:
    # Send packets within the window size
    for i in range(window_size):
        if not messages:
            break
        message = messages.pop(0)
        packet = f"{seq_num},{message}".encode()
        client_socket.send(packet)
        print(f"Sent packet with sequence number {seq_num}")
        seq_num += 1

    # Wait for an acknowledgment packet
    while True:
        try:
            ack_packet = client_socket.recv(1024)
            ack_num = int(ack_packet.decode())
            print(f"Received ACK for sequence number {ack_num}")
            break
        except ValueError:
            pass

    # Slide the window
    while ack_num >= seq_num - window_size:
        if not messages:
            break
        message = messages.pop(0)
        packet = f"{seq_num},{message}".encode()
        client_socket.send(packet)
        print(f"Sent packet with sequence number {seq_num}")
        seq_num += 1

# Close the client socket
client_socket.close()
