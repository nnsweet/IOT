
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "HeFeng.h"

HeFeng::HeFeng() {
}

void HeFeng::fans(HeFengCurrentData *data, String id) {  //获取粉丝数
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  String url = "https://api.bilibili.com/x/relation/stat?vmid=" + id;
  Serial.print("[HTTPS] begin...bilibili\n");
  if (https.begin(*client, url)) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument  jsonBuffer(2048);
        deserializeJson(jsonBuffer, payload);
        JsonObject root = jsonBuffer.as<JsonObject>();

        String follower = root["data"]["follower"];
        data->follower = follower;
        
        jsonBuffer.clear();
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      data->follower = "-1";
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
    data->follower = "-1";
  }
}

void HeFeng::doUpdateCurr(HeFengCurrentData *data, String key, String location) {  //获取天气

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  //String url = "https://free-api.heweather.net/s6/weather/now?lang=en&location=" + location + "&key=" + key;
  //https://devapi.qweather.com/v7/weather/now?lang=en&gzip=n&location=101040100&key=6ca77adf6dbf4c26896ec12117afc066
  String url = "https://devapi.qweather.com/v7/weather/now?lang=en&gzip=n&location=" + location + "&key=" + key;
  Serial.print("[HTTPS] begin...now\n");
  if (https.begin(*client, url)) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument  jsonBuffer(2048);
        deserializeJson(jsonBuffer, payload);
        JsonObject root = jsonBuffer.as<JsonObject>();

        String tmp = root["now"]["temp"];//温度
        data->tmp = tmp;
        String fl = root["now"]["feelsLike"];//体感温度
        data->fl = fl;
        String hum = root["now"]["humidity"];//湿度
        data->hum = hum;
        String wind_sc = root["now"]["windScale"];//风力
        data->wind_sc = wind_sc;
        String cond_code = root["now"]["icon"];//天气图标
        String meteoconIcon = getMeteoconIcon(cond_code);
        String cond_txt = root["now"]["text"];//天气
        data->cond_txt = cond_txt;
        data->iconMeteoCon = meteoconIcon;

        jsonBuffer.clear();
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      data->tmp = "-1";
      data->fl = "-1";
      data->hum = "-1";
      data->wind_sc = "-1";
      data->cond_txt = "no network";
      data->iconMeteoCon = ")";
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
    data->tmp = "-1";
    data->fl = "-1";
    data->hum = "-1";
    data->wind_sc = "-1";
    data->cond_txt = "no network";
    data->iconMeteoCon = ")";
  }

}

void HeFeng::doUpdateFore(HeFengForeData *data, String key, String location) {  //获取预报

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  //String url = "https://free-api.heweather.net/s6/weather/forecast?lang=en&location=" + location + "&key=" + key;
  String url = "https://devapi.qweather.com/v7/weather/3d?lang=en&gzip=n&location=" + location + "&key=" + key;
  Serial.print("[HTTPS] begin...forecast\n");
  if (https.begin(*client, url)) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument  jsonBuffer(4096);//8192
        deserializeJson(jsonBuffer, payload);
        JsonObject root = jsonBuffer.as<JsonObject>();
        int i;
        for (i = 0; i < 3; i++) {
          String datestr = root["daily"][i]["fxDate"];
          data[i].datestr = datestr.substring(5, datestr.length());
          String tmp_min = root["daily"][i]["tempMin"];
          data[i].tmp_min = tmp_min;
          String tmp_max = root["daily"][i]["tempMax"];
          data[i].tmp_max = tmp_max;
          String cond_code = root["daily"][i]["iconDay"];
          String meteoconIcon = getMeteoconIcon(cond_code);
          data[i].iconMeteoCon = meteoconIcon;
        }
        
        jsonBuffer.clear();
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      int i;
      for (i = 0; i < 3; i++) {
        data[i].tmp_min = "-1";
        data[i].tmp_max = "-1";
        data[i].datestr = "N/A";
        data[i].iconMeteoCon = ")";
      }
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
    int i;
    for (i = 0; i < 3; i++) {
      data[i].tmp_min = "-1";
      data[i].tmp_max = "-1";
      data[i].datestr = "N/A";
      data[i].iconMeteoCon = ")";
    }
  }

}

String HeFeng::getMeteoconIcon(String cond_code) {  //获取天气图标  见 https://dev.qweather.com/docs/start/icons/
  if (cond_code == "100" || cond_code == "150" || cond_code == "9006") {//晴 Sunny/Clear
    return "B";
  }
  if (cond_code == "101") {//多云 Cloudy
    return "Y";
  }
  if (cond_code == "102") {//少云 Few Clouds
    return "N";
  }
  if (cond_code == "103" || cond_code == "153") {//晴间多云 Partly Cloudy/
    return "H";
  }
  if (cond_code == "104" || cond_code == "154") {//阴 Overcast
    return "D";
  }
  if (cond_code == "300" || cond_code == "301") {//阵雨 Shower Rain 301-强阵雨 Heavy Shower Rain
    return "T";
  }
  if (cond_code == "302" || cond_code == "303") {//302-雷阵雨  Thundershower / 303-强雷阵雨
    return "P";
  }
  if (cond_code == "304" || cond_code == "313" || cond_code == "404" || cond_code == "405" || cond_code == "406") {
    //304-雷阵雨伴有冰雹 Freezing Rain
    //313-冻雨 Freezing Rain
    //404-雨夹雪 Sleet
    //405-雨雪天气 Rain And Snow
    //406-阵雨夹雪  Shower Snow
    return "X";
  }
  if (cond_code == "305" || cond_code == "308" || cond_code == "309" || cond_code == "314" || cond_code == "399") {
    //305-小雨 Light Rain
    //308-极端降雨 Extreme Rain
    //309-毛毛雨/细雨 Drizzle Rain
    //314-小到中雨 Light to moderate rain
    //399-雨 Light to moderate rain
    return "Q";
  }
  if (cond_code == "306" || cond_code == "307" || cond_code == "310" || cond_code == "311" || cond_code == "312" || cond_code == "315" || cond_code == "316" || cond_code == "317" || cond_code == "318") {
    //306-中雨 Moderate Rain
    //307-大雨 Heavy Rain
    //310-暴雨  Storm
    //311-大暴雨 Heavy Storm
    //312-特大暴雨 Severe Storm
    //315-中到大雨 Moderate to heavy rain
    //316-大到暴雨 Heavy rain to storm
    //317-暴雨到大暴雨 Storm to heavy storm
    //318-大暴雨到特大暴雨 Heavy to severe storm
    return "R";
  }
  if (cond_code == "400" || cond_code == "408") {
    //400-小雪 Light Snow
    //408-小到中雪 Light to moderate snow
    return "U";
  }
  if (cond_code == "401" || cond_code == "402" || cond_code == "403" || cond_code == "409" || cond_code == "410") {
    //401-中雪 Moderate Snow
    //402-大雪 Heavy Snow
    //403-暴雪 Snowstorm
    //409-中到大雪 Moderate to heavy snow
    //410-大到暴雪 Heavy snow to snowstorm
    return "W";
  }
  if (cond_code == "407") {
    //407-阵雪 Snow Flurry
    return "V";
  }
  if (cond_code == "499" || cond_code == "901") {
    //499-雪 Snow
    //901-冷 Cold
    return "G";
  }
  if (cond_code == "500") {
    //500-薄雾 Mist
    return "E";
  }
  if (cond_code == "501" || cond_code == "509" || cond_code == "510" || cond_code == "514" || cond_code == "515") {
    //501-雾 Foggy
    return "M";
  }
  if (cond_code == "502" || cond_code == "511" || cond_code == "512" || cond_code == "513") {
    //502-霾 Haze
    return "L";
  }
  if (cond_code == "503" || cond_code == "504" || cond_code == "507" || cond_code == "508") {
    //503-扬沙 Sand
    return "F";
  }
  
  if (cond_code == "999") {//未知
    return ")";
  }
  if (cond_code == "213") {
    return "O";
  }
  if (cond_code == "200" || cond_code == "201" || cond_code == "202" || cond_code == "203" || cond_code == "204" || cond_code == "205" || cond_code == "206" || cond_code == "207" || cond_code == "208" || cond_code == "209" || cond_code == "210" || cond_code == "211" || cond_code == "212") {
    return "S";
  }
  return ")";
}
