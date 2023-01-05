#define CURL_STATICLIB
#include <iostream>
#include <ostream>
#include <cstdlib>
#include <mosquitto.h>
#include "curl/curl.h"
#include <string>
#include <wincrypt.h>
#include <nlohmann/json.hpp>
#ifdef _DEBUG
#pragma comment (lib, "curl/libcurl_a_debug.lib")
#else
#pragma comment (lib, "curl/libcurl_a.lib")
#endif
#pragma comment (lib, "Crypt32.lib")
#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "advapi32.lib")
using json = nlohmann::json;

size_t weather_write_data(void* ptr, size_t size, size_t nmemb, void* str);
size_t curl_callback(void* contents, size_t size, size_t nmemb, void* userp);

using namespace std;
int main()
{
	CURL* curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	string const url = "https://danepubliczne.imgw.pl/api/data/synop/id/12560";
	json weather_data;
	string dataString;
	string data; 

	curl = curl_easy_init();
	if (!curl) 
	{
		cout << "Error: cannot initialize CURL." << endl;
		return 1;
	}
	res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (res != CURLE_OK)
	{
		cout << "Cannot set curl url. Error code: " << res << endl;
		return 1;
	}
	res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	if (res != CURLE_OK) 
	{
		cout << "Cannot set curl follow location flag. Error code: " << res << endl;
		return 1;
	}
	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback); // weather_write_data); // --> string > JSON
	if (res != CURLE_OK) 
	{
		cout << "Cannot set the weather write function. Error code: " << res << endl;
		return 1;
	}
	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &weather_data); // &dataString); // --> string > JSON
	if (res != CURLE_OK) 
	{
		cout << "Cannot set the curl write data. Error code: " << res << endl;
		return 1;
	}
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) 
	{
		cout << "Cannot perform curl. Error code: " << res << endl;
		return 1;
	}
	//Wysyłanie na serwer mosquitto
	int rc;
	const string MQTT_SERVER = "127.0.0.1";
	const int MQTT_PORT = 1883;
	mosquitto_lib_init();
	struct mosquitto* mosq;
	string topic = "id = 12560 (Katowice)";
	dataString = weather_data.dump();
	mosq = mosquitto_new("Client", true, NULL);
	if (!mosq) {
		cout << "Error: Cannot create Mosquitto client. Quitting." << endl;
		return 1;
	}

	rc = mosquitto_connect(mosq, MQTT_SERVER.c_str(), MQTT_PORT, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		cout << "Error: Client couldn't connect to broker. Error code: " << rc << endl;
		mosquitto_destroy(mosq);
		return 1;
	}
	else {
		cout << "Connected with broker!" << endl;
		cout << "Data published: " << endl << weather_data << endl;
	}

	if (mosquitto_publish(mosq, NULL, topic.c_str(), dataString.size(), dataString.c_str(), 0, false) != MOSQ_ERR_SUCCESS) {
		cout << "Error: Cannot publish data to MQTT broker." << endl;
		mosquitto_destroy(mosq);
		return 1;
	}
	else {
		cout << "MQTT: Published correctly" << endl;
	}

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	return 0;
}
size_t weather_write_data(void* ptr, size_t size, size_t nmemb, void* str) {
	string* s = static_cast<string*>(str);
	copy((char*)ptr, (char*)ptr + (size + nmemb), back_inserter(*s));
	return size * nmemb;
}
size_t curl_callback(void* contents, size_t size, size_t nmemb, void* userp) {
	json* obj = (json*)userp;
	size_t realsize = size * nmemb;
	string data((char*)contents, realsize);
	try {
		json new_data = json::parse(data);
		*obj = new_data;
	}
	catch (const json::parse_error& e) {
		cerr << "Error parsing JSON data: " << e.what() << endl;
	}
	return realsize;
}
