import paho.mqtt.client as mqtt
from datetime import datetime
from astral import LocationInfo
from astral.sun import sun


# MQTT_ADDRESS = '192.168.4.1'
# MQTT_ADDRESS = '192.168.2.102'
MQTT_ADDRESS = '127.0.0.1'
MQTT_USER = 'project'
MQTT_PASSWORD = 'admin'
MQTT_TOPIC = 'home/terminals/+/+'
MQTT_CONTROL_TOPIC = 'home/controlling'

lst_2_hum = [50, 50]


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    # print(msg.topic + ' ' + str(msg.payload))
    # print(msg.payload)
    topics(msg)
    publish(msg, client)


def topics(msg):  # name,value#name,value#name,value
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
    url_p2 = url_p2[:-1]

    print('\t' + url_p2)


def publish(msg, client):
    city = LocationInfo("Jerusalem", "Palestine", "Asia/Jerusalem")
    s = sun(city.observer, date=datetime.now())
    now_time = datetime.now()
    day_night = 0  # day = 0, night = 1

    if s['sunrise'].time() < now_time.time():
        if now_time.time() < s['sunset'].time():
            print(' --> day')
            day_night = 0
        else:
            print(' --> night')
            day_night = 1

    payload = str(msg.payload).strip("b'")
    arr_payload = payload.split("#")
    count = 0

    for i in arr_payload:
        temp_arr = arr_payload[count].split(",")
        count += 1

        if temp_arr[0].__eq__("aHum"):

            lst_2_hum[0] = lst_2_hum[1]
            lst_2_hum[1] = int(temp_arr[1])

            avg_hum = (lst_2_hum[0] + lst_2_hum[1]) / 2

            if day_night == 0:  # day
                if int(avg_hum) >= 90:
                    # publish on
                    client.publish(MQTT_CONTROL_TOPIC, 'on')
                    print('fan on')
                else:
                    # publish off
                    client.publish(MQTT_CONTROL_TOPIC, 'off')
                    print('fan off')

            elif day_night == 1:  # night
                if int(avg_hum) >= 75:
                    # publish on
                    client.publish(MQTT_CONTROL_TOPIC, 'on')
                    print('fan on')
                else:
                    # publish off
                    client.publish(MQTT_CONTROL_TOPIC, 'off')
                    print('fan off')


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
