#include <Arduino.h>
#include <M5stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// If you don't use secrets.h comment out this
#define externalSecrets

#ifdef externalSecrets
	#include <secrets.h>
#else
	const char *ssid = "";
	const char *password = "";
#endif

const char *apiServer = "https://github-contributions-api.herokuapp.com/3rdJCG/count";

void setup()
{
	M5.begin(true, false, true);
	M5.Lcd.clear(BLACK);
	M5.Lcd.setTextColor(WHITE);
	M5.Lcd.setTextSize(2);
	M5.Lcd.setCursor(0, 0);
	M5.Lcd.setTextDatum(0);
	M5.Lcd.println("github-contributions-m5stack");
	M5.Lcd.print("connecting");

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		M5.Lcd.print(".");
	}
	M5.Lcd.println("Done!");
}

void loop()
{
	if ((WiFi.status() == WL_CONNECTED))
	{

		HTTPClient http;
		http.begin(apiServer);

		int httpCode = http.GET();

		if (httpCode > 0)
		{
			String payload = http.getString();
			
			// M5.Lcd.println(payload);
			// delay(1000);

			DynamicJsonDocument doc(65535);
			
			deserializeJson(doc, payload);
			
			JsonObject obj = doc.as<JsonObject>();
			JsonObject buf = obj[String("data")][String("2020")][String("7")];
			String contributions_count = buf[String("25")];

			M5.Lcd.clear(BLACK);
			M5.Lcd.setTextSize(8);
			M5.Lcd.setCursor(120, 100);
			M5.Lcd.println(contributions_count);
		}
		else
		{
			M5.Lcd.println("Bad HTTP responce");
			return;
		}

		http.end();
		delay(3000);
	}
}