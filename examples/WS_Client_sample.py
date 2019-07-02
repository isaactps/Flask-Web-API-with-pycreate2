import socketio
import sys
import time

sio = socketio.Client()
device_id = 50

@sio.event	
def connect():
	print('Client Id %d connected' % device_id)

@sio.event
def disconnect():
	print('Client ID %d disconnected' % device_id)
	
@sio.on('broadcast message')
def on_broadcast(data):
	print("Client ID %d: %s" % (device_id,data))

def send_message(data):
	sio.send(data)

try:
	sio.connect('http://10.0.0.7:5000')
	#sio.connect('http://192.168.1.75:5000')

except Exception:
	print("Error in connecting to WS Server")
	exit(0)	

data_read = "Hello from Client Id %d" % device_id

while True:
	try:
		#send_message(data_read)
		time.sleep(5.0)		
	
	except KeyboardInterrupt:
		print("Keyboard Interrupt detected")
		break

sio.disconnect()
print("shutting down ... bye")