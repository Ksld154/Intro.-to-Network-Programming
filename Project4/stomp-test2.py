import time
import sys

import stomp

class MyListener(stomp.ConnectionListener):
    def on_error(self, headers, message):
        print('received an error "%s"' % message)
    def on_message(self, headers, message):
        print('received a message "%s"' % message)

conn = stomp.Connection()
conn.set_listener('', MyListener())
# conn.set_listener('', stomp.listener.PrintingListener())
conn.start()
conn.connect('admin', 'password', wait=False)
# conn.connect('admin', 'password', wait=True)


conn.subscribe(destination='/queue/test', id=1, ack='auto')

conn.send(body='hello,garfield!', destination='/queue/test')

time.sleep(2)
conn.disconnect()
