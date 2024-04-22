import serial
import uinput
import pynput.keyboard
import time

ser = serial.Serial('/dev/rfcomm1', 115200, timeout = 10)

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
			data = ser.read(1)
			if (data):
				print("HELLO THERE", data)
			if data == b'\xff':		
				print("GENERAL KENOBI  ", data)
				break

		# Read 3 bytes from UART
		data = ser.read(4)
		print("Data lido :  ", data)
		print("Dicionario :  ", data[0])
	
		if data[0] == 1:
			axis, value = parse_data(data[1:4])
			move_mouse(axis, value)
			# print(f"Received data joy stick:")
			# print(f"axis: {axis}, value: {value}")
		elif data[0] == 2:
			value = int.from_bytes(data[1], byteorder='little', signed=True)
		elif data[0] == 3:
			# print("Data lido :  ", data)
			# print("Dicionario :  ", data[0])
			# value = int.from_bytes(data[1], byteorder='little', signed=True)
			# print("Apertou no espaço")
			axis, value = parse_data(data[1:4])
			if axis == 0:
				teclado.emit(uinput.KEY_SPACE, value)
			elif axis == 1:
				teclado.emit(uinput.KEY_E, value)
			elif axis == 2:
				teclado.emit(uinput.KEY_LEFTCTRL, value)
			elif axis == 3:
				teclado.emit(uinput.KEY_LEFTSHIFT, value)



			

except KeyboardInterrupt:
	print("Program terminated by user")
except Exception as e:
	print(f"An error occurred: {e}")
finally:
	ser.close()
