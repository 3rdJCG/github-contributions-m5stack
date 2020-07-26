#include <Arduino.h>
#include <M5stack.h>
#include <WiFi.h>
#include <time.h>
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

const char *ntpServer = "ntp.jst.mfeed.ad.jp";
const long gmtOffset_sec = 9 * 3600;
const int daylightOffset_sec = 0;

void setup()
{
	M5.begin(true, false, true);
	M5.Lcd.clear(BLACK);
	M5.Lcd.setTextColor(WHITE);
	M5.Lcd.setTextSize(1);
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
		configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
		time_t t;
		struct tm *tm;
		static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};

		t = time(NULL);
		tm = localtime(&t);

		M5.Lcd.clear(BLACK);
		M5.Lcd.setTextColor(WHITE);
		M5.Lcd.setTextSize(2);
		M5.Lcd.setCursor(0, 0);
		M5.Lcd.printf(" %04d/%02d/%02d(%s) %02d:%02d:%02d\n",
					  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
					  wd[tm->tm_wday],
					  tm->tm_hour, tm->tm_min, tm->tm_sec);

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
			JsonObject buf = obj[String("data")][String(tm->tm_year + 1900)][String(tm->tm_mon + 1)];
			String contributions_count = buf[String(tm->tm_mday)];

			// M5.Lcd.clear(BLACK);
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