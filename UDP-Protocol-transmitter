import socket

UDP_IP = "192.168.1.198"  # replace with the IP address of your ESP32
UDP_PORT = 8080
MESSAGE = "Hello, ESP32!"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(MESSAGE.encode(), (UDP_IP, UDP_PORT))
print("Message sent to ESP32")
