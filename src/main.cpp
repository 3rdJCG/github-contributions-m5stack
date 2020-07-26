#include <Arduino.h>
#include <M5Stack.h>
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
const String github_username = "";
#endif

const String apiServer = "https://github-contributions-api.herokuapp.com/";

const char *ntpServer = "ntp.jst.mfeed.ad.jp";
const long gmtOffset_sec = 9 * 3600;
const int daylightOffset_sec = 0;

const uint8_t grassColors[5][3] = {{235, 237, 240},
								   {155, 233, 168},
								   {64, 196, 99},
								   {48, 180, 78},
								   {33, 170, 57}};

void displayMode(uint8_t mode);

void setup()
{
	M5.begin(true, false, true);
	displayMode(0);
	M5.Lcd.println("github-contributions-m5stack");
	M5.Lcd.print("connecting");

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		M5.Lcd.print(".");
	}
	M5.Lcd.println("Done!");

	M5.Lcd.setBrightness(200);
}

void loop()
{
	if ((WiFi.status() == WL_CONNECTED))
	{
		configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
		time_t t;
		struct tm *tm;

		t = time(NULL);
		tm = localtime(&t);

		HTTPClient http;
		http.begin(apiServer + github_username + "/count");

		int httpCode = http.GET();

		if (httpCode > 0)
		{
			String payload = http.getString();

			DynamicJsonDocument doc(65535);

			deserializeJson(doc, payload);

			JsonObject obj = doc.as<JsonObject>();
			JsonObject buf = obj[String("data")][String(tm->tm_year + 1900)][String(tm->tm_mon + 1)];
			String contributions_count = buf[String(tm->tm_mday)];

			// Back colors setting
			uint8_t grassColorDepth = 0;
			if (contributions_count.toInt() <= 0)
			{
				grassColorDepth = 0;
			}
			else if (contributions_count.toInt() < 3)
			{
				grassColorDepth = 1;
			}
			else if (contributions_count.toInt() < 5)
			{
				grassColorDepth = 2;
			}
			else if (contributions_count.toInt() < 10)
			{
				grassColorDepth = 3;
			}
			else if (contributions_count.toInt() >= 10)
			{
				grassColorDepth = 4;
			}
			else
			{
				grassColorDepth = 0;
			}

			// Text color setting
			if (grassColorDepth <= 2)
			{
				M5.Lcd.setTextColor(M5.Lcd.color565(20, 20, 20));
			}
			else
			{
				M5.Lcd.setTextColor(M5.Lcd.color565(230, 230, 230));
			}

			M5.Lcd.clear(M5.Lcd.color565(grassColors[grassColorDepth][0], grassColors[grassColorDepth][1], grassColors[grassColorDepth][2]));
			M5.Lcd.setTextSize(2);
			M5.Lcd.drawCentreString("Today's Contributions", 160, 10, 2);
			M5.Lcd.drawCentreString(github_username, 160, 200, 2);
			M5.Lcd.setTextSize(10);
			M5.Lcd.setTextDatum(4);
			M5.Lcd.drawCentreString(contributions_count, 160, 65, 2);
		}
		else
		{
			displayMode(0);
			M5.Lcd.setTextColor(YELLOW);
			M5.Lcd.println("Bad HTTP responce");
			return;
		}

		http.end();
		delay(10000);
	}
	else
	{
		displayMode(0);
		M5.Lcd.setTextColor(YELLOW);
		M5.Lcd.println("Connection error");
		M5.Lcd.setTextColor(WHITE);
		M5.Lcd.println("Please check WiFi connection");
		
		delay(10000);
	}
}

void displayMode(uint8_t mode)
{
	switch (mode)
	{
	case 0: // Bash like mode
		M5.Lcd.clear(BLACK);
		M5.Lcd.setTextColor(WHITE);
		M5.Lcd.setTextSize(1);
		M5.Lcd.setCursor(0, 0);
		M5.Lcd.setTextDatum(0);
		break;

	default:
		break;
	}
}