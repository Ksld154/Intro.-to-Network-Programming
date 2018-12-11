import time
import sys
import stomp

class MyListener(object):
    def on_error(self, headers, message):
        print('received an error %s' % message)
    def on_message(self, headers, message):
        print('received a message %s' % message)

conn = stomp.Connection(host_and_ports=[('localhost', 61616)])
conn.set_listener('', MyListener())
conn.start()
conn.connect()
conn.subscribe(destination='/home/bitcycle/svn/cass/queue.test', ack='auto') # pylint: disable=E1120
conn.send('Test', destination='/home/bitcycle/svn/cass/queue.test')
time.sleep(2)
conn.disconnect()