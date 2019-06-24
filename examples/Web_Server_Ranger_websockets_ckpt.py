#!/home/nam01/anaconda3/envs/ipy36/bin/python
# ----------------------------------------------------------------------------
# MIT License
# Moves the robot around

from __future__ import print_function
from _thread import start_new_thread
from flask import Flask, jsonify, abort, request, make_response, url_for, flash
from flask import render_template, redirect, session
import flask_pycreate2

from flask_socketio import SocketIO, join_room, emit, send
import time
import sys

server_ip_dict = [
    {'id': 0,
     'address': '192.168.1.104', #'10.0.0.14', #'10.0.0.11', #'192.168.1.104', #
     'transport': 'udp'},
    {'id': 1,
     'address': '192.168.1.114', #'10.0.0.13', #'192.168.1.114', #
     'transport': 'udp'},
	{'id': 2,
     'address': '192.168.1.136',
     'transport': 'udp'},
	{'id': 3,
     'address': '192.168.1.145',
     'transport': 'udp'}
]

bot = [None] * len(server_ip_dict)

#Creating Flask instance
app = Flask(__name__, template_folder='Templates')
app.secret_key = "mBot Ranger"

my_socketio = SocketIO(app)

@my_socketio.on('connect')
def test_connect():
	print('WS Server: Client connected')

@my_socketio.on('disconnect')
def test_disconnect():
	print('WS Server: Client disconnected')

@my_socketio.on('message')
def handle_message(message):
	print('WS Server: ' + str(message))
	emit('broadcast message',str(message), broadcast = True)

@app.route('/<int:device_id>/config_bot/',methods = ['POST', 'GET'])
def config_bot(device_id):

	global bot
	global server_ip_dict
	
	if request.method == 'POST':

		# Instantiate a Create2 Bot based on supported device_id number

		for item in server_ip_dict:
			if item['id'] == device_id:
				if item['transport'] == request.form.get('transport'):
				
					config = {}
					config["transport"] = item['transport']
					config["server_ip_addr"] = item['address']
					config["robot"] = request.form.get('robot')
					config["logfile_name"] = 'mBot_Ranger_U_' + str(device_id) + '_'

					if (bot[device_id] is not None):
						bot[device_id].drive_stop()
						time.sleep(0.1)
						bot[device_id].SCI.close()
						time.sleep(0.1)
						bot[device_id].logging = False # To stop the existing read_thread thread					
						time.sleep(2.0)
						bot[device_id] = None
						bot[device_id] = flask_pycreate2.Create2(config)
						bot[device_id].logging = True						
						bot[device_id].start()
						bot[device_id].safe()
						try:
							start_new_thread(read_thread ,(bot,device_id,))
						except:
							print("Error: unable to start thread id %d" % device_id)
					else:
						bot[device_id] = flask_pycreate2.Create2(config)
						bot[device_id].logging = True
						bot[device_id].start()
						bot[device_id].safe()
						try:
							start_new_thread(read_thread ,(bot,device_id,))
						except:
							print("Error: unable to start thread id %d" % device_id)
						
	return "Request received! " + str(request.form) + ' ID %d' % device_id					

def read_thread(bot,device_id):

	import socketio

	sio = socketio.Client()
	
	@sio.event	
	def connect():
		print('Client Id %d connected' % device_id)

	@sio.event
	def disconnect():
		print('Client ID %d disconnected' % device_id)
		
	#@sio.on('broadcast message')
	#def on_broadcast(data):
	#	print('Client ID %d received Broadcast!' % device_id)

	def send_message(data):
		#print('Echo message received ', data) #Debugging
		sio.send(data)

	sio.connect('http://127.0.0.1:5000')
	data_read = ""

	while bot[device_id].logging == True:
		try:
			data_read = bot[device_id].SCI.read(bot[device_id].SCI.buffersize)
			if(data_read != -1):
				send_message(str(data_read) + ": Id %d" % device_id)
		except:
			print("Error in read_thread Id %d" % device_id)
			
	sio.disconnect()

@app.route('/<int:device_id>/move_forw/',methods = ['POST'])
def move_forw(device_id):

	global bot

	if request.method == 'POST':
		
		vel = int(request.form.get('velocity'))

		if bot[device_id] is not None:
			bot[device_id].drive_stop()
			if (vel < 0):
				vel = abs(vel)
			
			bot[device_id].drive_straight(vel) # Move forwards Non-Stop

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/move_back/',methods = ['POST'])
def move_back(device_id):

	global bot

	if request.method == 'POST':
		
		vel = int(request.form.get('velocity'))

		if bot[device_id] is not None:
			bot[device_id].drive_stop()
			if (vel > 0):
				vel = -1 *vel
			
			bot[device_id].drive_straight(vel) # Move backwards Non-Stop

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/turn_left/',methods = ['POST'])
def turn_left(device_id):

	global bot

	if request.method == 'POST':
		
		vel = int(request.form.get('velocity'))

		if bot[device_id] is not None:
			bot[device_id].drive_stop()
			if (vel < 0):
				vel = abs(vel)
			
			bot[device_id].drive_turn(vel, 1) # Turn CCW Non-Stop

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/turn_right/',methods = ['POST'])
def turn_right(device_id):

	global bot

	if request.method == 'POST':
		
		vel = int(request.form.get('velocity'))

		if bot[device_id] is not None:
			bot[device_id].drive_stop()
			if (vel < 0):
				vel = abs(vel)
			
			bot[device_id].drive_turn(vel, -1) # Turn CW Non-Stop
					
	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/move_dist/',methods = ['POST'])
def move_dist(device_id):

	global bot

	if request.method == 'POST':
		
		dist_execute_inches = int(request.form.get('distance')) # Move forward in inches
		vel = int(request.form.get('velocity'))

		if bot[device_id] is not None:
			bot[device_id].drive_distance_Ranger(dist_execute_inches,vel)

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/turn_angle/',methods = ['POST'])
def turn_angle(device_id):

	global bot

	if request.method == 'POST':

		angle_execute_degrees = int(request.form.get('angle')) # Turn angle in degrees
		vel = int(request.form.get('velocity'))

		if bot[device_id] is not None:
			bot[device_id].turn_angle_Ranger(angle_execute_degrees,vel)

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/stop_bot/',methods = ['POST'])
def stop_bot(device_id):

	global bot

	if request.method == 'POST':

		if bot[device_id] is not None:
			bot[device_id].drive_stop()

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/line_follow/',methods = ['POST'])
def line_follow(device_id):

	global bot

	if request.method == 'POST':
		
		if bot[device_id] is not None:
			bot[device_id].line_follow_Ranger()
			current_time = time.time()

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
@app.route('/<int:device_id>/move_ckpt/',methods = ['POST'])
def move_ckpt(device_id):

	global bot

	if request.method == 'POST':

		checkpoint = int(request.form.get('checkpoint'))
	
		if bot[device_id] is not None:
			bot[device_id].move_ckpt_Ranger(checkpoint)

	return "Request received! " + str(request.form) + ' ID %d' % device_id				
			
		
if __name__ == "__main__":

	my_socketio.run(app, host = '0.0.0.0', port = 5000, debug = True)
	#my_socketio.run(app, host = '0.0.0.0', port = 5000)
