import time
import threading
import Queue
import logging
import serial
import sys

logging.basicConfig(level=logging.DEBUG, format='%(asctime)s [%(levelname)s] (%(threadName)-10s) %(message)s')

class HandleHIDCode(threading.Thread):
	
        def __init__(self, req_q):
                super(HandleHIDCode, self).__init__()
                self.q = req_q
                self.stoprequest = threading.Event()

        def run(self):
                while not self.stoprequest.isSet():
                        try:
                                hid_message = self.q.get(True, 0.05)
                                logging.info("Got HID message '{msg}' from Queue".format(msg=hid_message))
                                self.process_hid_code(hid_message)

                        except Queue.Empty:
                                continue

	def process_hid_code(self, hid_code):
		logging.info("Processing HID code '{msg}'".format(msg=hid_code))

        def join(self, timeout=None):
                logging.info("Setting exit flag")
                self.stoprequest.set()
                super(HandleHIDCode, self).join(timeout)

class SerialHandler(threading.Thread):

        def __init__(self, req_q):
                super(SerialHandler, self).__init__()
                self.q = req_q
		self.ser = serial.Serial("/dev/ttyACM0", 9600)
                self.stoprequest = threading.Event()

        def run(self):
                while not self.stoprequest.isSet():
			data = self.ser.readline().rstrip("\r\n")
			logging.info("Got data '{d}' from serial port".format(d=data))
			self.q.put("{d}".format(d=data))
			
        def join(self, timeout=None):
                logging.info("Setting exit flag")
                self.stoprequest.set()
                super(SerialHandler, self).join(timeout)

def run_program():
        q = Queue.Queue()
        hid_thread = HandleHIDCode(req_q=q)
        ser_thread = SerialHandler(req_q=q)

        try:
                # HID code handling thread
                hid_thread.setDaemon(True)
                hid_thread.start()
                logging.info("HID handling thread has started")

                # Serial handling thread
                ser_thread.setDaemon(True)
                ser_thread.start()
                logging.info("Serial handling thread has started")

                # keep waiting
                while(True):
                        time.sleep(5)
                        logging.debug("Main thread running")

        except (KeyboardInterrupt, SystemExit):
                logging.info("Cleaning up... killing threads")
                ser_thread.join(3)
                hid_thread.join(3)
                sys.exit()

if __name__ == '__main__':
        logging.info("Main method, starting")
        run_program()

