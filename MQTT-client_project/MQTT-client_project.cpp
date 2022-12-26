#define CURL_STATICLIB
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <mosquitto.h>
#include "curl/curl.h"
#include <string>
#include <wincrypt.h>

using namespace std;
int main()
{
	string dataString;
	string data; //zmienna testowa - GIT

	//Wysyłanie na serwer mosquitto
	int rc;
	const string MQTT_SERVER = "127.0.0.1";
	const int MQTT_PORT = 1883;
	mosquitto_lib_init();
	struct mosquitto* mosq;

	string topic = "topic";

	mosq = mosquitto_new("Client", true, NULL);

	rc = mosquitto_connect(mosq, MQTT_SERVER.c_str(), MQTT_PORT, 60);
	if (rc != 0) {
		cout << "Client couldn't connect to broker" << endl;
		return 1;
	}
	else {
		cout << "Connected with broker!" << endl;
	}


	if (mosquitto_publish(mosq, NULL, topic.c_str(), dataString.size(), dataString.c_str(), 0, false) == MOSQ_ERR_SUCCESS) {
		cout << "send" << endl;
	}
	else
	{
		cout << "Error" << endl;
	}

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);

	return 1;

	//dodać JSONA 
	//test zmian GIT
	//test zmian GIT v2
}
