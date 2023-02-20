import socket

# Initialize the server socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('localhost', 8080))
server_socket.listen()

# Wait for a client to connect
print("Waiting for a client to connect...")
client_socket, client_address = server_socket.accept()
print(f"Client {client_address} has connected.")

# Initialize variables
expected_seq_num = 0
window_size = 4
buffer = []

while True:
    # Receive data from client
    data = client_socket.recv(1024)
    if not data:
        break
    seq_num, message = data.decode().split(",")
    seq_num = int(seq_num)

    # If the received packet has the expected sequence number
    if seq_num == expected_seq_num:
        print(f"Received packet with sequence number {seq_num}")
        expected_seq_num += 1
        buffer.append(message)

        # Send an acknowledgment packet
        ack_packet = str(expected_seq_num).encode()
        client_socket.send(ack_packet)
        print(f"Sent ACK for sequence number {expected_seq_num}")

        # Flush buffer
        while buffer and buffer[0] is not None:
            print(f"Delivered message: {buffer.pop(0)}")

    # If the received packet has a sequence number outside of the window
    elif seq_num < expected_seq_num - window_size or seq_num >= expected_seq_num + window_size:
        print(f"Discarded packet with out-of-order sequence number {seq_num}")
        ack_packet = str(expected_seq_num).encode()
        client_socket.send(ack_packet)
        print(f"Sent duplicate ACK for sequence number {expected_seq_num}")

    # If the received packet has a sequence number within the window, but not the expected sequence number
    else:
        print(f"Discarded packet with sequence number {seq_num}")
        ack_packet = str(expected_seq_num).encode()
        client_socket.send(ack_packet)
        print(f"Sent duplicate ACK for sequence number {expected_seq_num}")

# Close the server socket
server_socket.close()