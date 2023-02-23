import socket
import time
import string
import random

def id_generator(size=10, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))

# Initialize the client socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('localhost', 8080))

# Initialize variables
seq_num = 0
window_size = int(input('Tamanho janela: '))
n_messages = int(input('Numero de mensagens: '))
messages = []

for i in range(n_messages):
    messages.append(id_generator())


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

    """ # Wait for an acknowledgment packet
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
        seq_num += 1 """

# Close the client socket
client_socket.close()
