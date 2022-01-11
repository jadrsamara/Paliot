import paho.mqtt.client as mqtt
import urllib

# MQTT_ADDRESS = '192.168.4.1'
# MQTT_ADDRESS = '192.168.2.102'
MQTT_ADDRESS = '127.0.0.1'
MQTT_USER = 'project'
MQTT_PASSWORD = 'admin'
MQTT_TOPIC = 'home/terminals/+/+'


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    # print(msg.topic + ' ' + str(msg.payload))
    # print(msg.payload)
    topics(msg)


def topics(msg):
    arr = msg.topic.split("/")
    # print(arr[2], str(msg.payload).strip("b'"))

    payload = str(msg.payload).strip("b'")
    arr_payload = payload.split("#")
    count = 0
    url_p2 = ""
    for i in arr_payload:
        temp_arr = arr_payload[count].split(",")
        url_p2 += temp_arr[0] + '=' + temp_arr[1] + '&'
        count += 1
    url_p2 = 'nm=' + arr[2] + '&' + url_p2[:-1]

    print('\t'+url_p2)

    # Link to add values:
    # https://script.google.com/macros/s/AKfycby6DwotYTfLVbRZGFVE-njuEibdFJGNtu8bgRztoDgN7CEpft418BNHJn8o1BTm_uTPXw/exec?nm=dev1&snsr=snsr1&typ=typ1&val=1

    url = 'https://script.google.com/macros/s/AKfycby6DwotYTfLVbRZGFVE'\
          '-njuEibdFJGNtu8bgRztoDgN7CEpft418BNHJn8o1BTm_uTPXw/exec?' + url_p2

    web_url = urllib.request.urlopen(url)

    print("\tresult code:\n\t\t"+str(web_url.read()).strip("b'")+"\n")


def main():
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()
