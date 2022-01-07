import socket
from contextlib import closing
from slack_sdk.webhook import WebhookClient
import time, datetime

UDP_IP="0.0.0.0"
UDP_PORT=9000

sock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind((UDP_IP,UDP_PORT))

with closing(sock):
    while True:
        data,addr = sock.recvfrom(1024)
        #print("Send from ESP",data)
        print("Detect!")
        today = datetime.datetime.fromtimestamp(time.time())
        strtime = today.strftime('%Y/%m/%d %H:%M:%S')
        url="webhookのURL"
        webhook = WebhookClient(url)
        webhook.send(text="Detect! " + str(strtime))
