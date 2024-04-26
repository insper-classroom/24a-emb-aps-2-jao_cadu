import serial
import uinput
import pynput.keyboard
import time

ser = serial.Serial('/dev/rfcomm0', 115200, timeout=100)
# ser = serial.Serial('/dev/ttyACM0', 115200)


# Create new mouse device
device = uinput.Device([
	uinput.BTN_LEFT,
	uinput.BTN_RIGHT,
	uinput.REL_X,
	uinput.REL_Y,
])

#create a teclado
teclado = uinput.Device([
	uinput.KEY_SPACE,
	uinput.KEY_E,
	uinput.KEY_LEFTSHIFT, 
    uinput.KEY_LEFTCTRL,
	uinput.KEY_A,
	uinput.KEY_W,
	uinput.KEY_S,
	uinput.KEY_D,
])


def parse_data(data):
	axis = data[0]  # 0 for X, 1 for Y
	value = int.from_bytes(data[1:3], byteorder='little', signed=True)
	# print(f"Received data: {data}")
	# print(f"axis: {axis}, value: {value}")
	return axis, value
	


def move_mouse(axis, value):
	if axis == 0:    # X-axis
		device.emit(uinput.REL_X, value)
	elif axis == 1:  # Y-axis
		device.emit(uinput.REL_Y, value)


try:
	# sync package
	
	while True:
		print('Waiting for sync package...')
		while True:
			# print("entrou")
			data=ser.read(1)
			# print("ola")
			# print(data)
			# print(data)
			if data == b'\xff':		
				# print("GENERAL KENOBI  ", data)
				break

		# Read 3 bytes from UART
		data = ser.read(4)
		if (len(data) < 4):
			continue
		# print("Data lido :  ", data)
		# print("Dicionario :  ", data[0])
	
		if data[0] !=1:
			print("Data lido :  \n", data)
			print("Dicionario :  \n", data[0])
				
		if data[0] == 1:
			axis, value = parse_data(data[1:4])
			move_mouse(axis, value)
			# print(f"Received data joy stick:")
			# print(f"axis: {axis}, value: {value}")
		elif data[0] == 2:
			axis, value = parse_data(data[1:4])
			# value = int.from_bytes(data[1], byteorder='little', signed=True)
		elif data[0] == 3:
			print("Data lido :  ", data)
			print("Dicionario :  ", data[0])
			# value = int.from_bytes(data[1], byteorder='little', signed=True)
			# print("Apertou no espaço")
			axis, value = parse_data(data[1:4])
			if axis == 0:
				teclado.emit(uinput.KEY_SPACE, value)
			elif axis == 1:
				teclado.emit(uinput.KEY_E, value)
			elif axis == 2:
				teclado.emit(uinput.KEY_LEFTSHIFT, value)
			elif axis == 3:
				teclado.emit(uinput.KEY_LEFTCTRL, value)
			elif axis == 4:
				teclado.emit(uinput.KEY_A, value)
			elif axis == 5:
				teclado.emit(uinput.KEY_W, value)
			elif axis == 6:
				teclado.emit(uinput.KEY_S, value)
			elif axis == 7:
				teclado.emit(uinput.KEY_D, value)
			elif axis == 8:
				print("value: ",value)
				device.emit(uinput.BTN_LEFT, value)


			

except KeyboardInterrupt:
	print("Program terminated by user")
except Exception as e:
	print(f"An error occurred: {e}")
finally:
	ser.close()
