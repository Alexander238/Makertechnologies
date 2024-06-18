#include <AccelStepper.h>
#include <map>
#include <WiFi.h>
#include "time.h"
#include <WebServer.h>

/*** TIME-VARIABLES ***/
const char* ssid[] = {"WLAN-Kabel Gast", "FH-Kiel-IoT-NAT", "Lucas"};
const char* password[] = {"sandman1998", "!FH-NAT-001!", "3141592654"};
int maxAttemptsToConnectToWifi = 10;
const char *ap_ssid = "Flap Display";  // Access Point SSID
const char *ap_password = "";       // Access Point Password
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;
char currentTimezone[100] = "CET-1CEST,M3.5.0,M10.5.0/3"; // Initial timezone

/*** MOTOR-VARIABLES ***/
const float fullRevolution = 4097.0;
const float SteppDegree = fullRevolution / 360;
AccelStepper stepper1(AccelStepper::HALF4WIRE, 23, 21, 22, 19);
AccelStepper stepper2(AccelStepper::HALF4WIRE, 18, 17, 5, 16);
AccelStepper stepper3(AccelStepper::HALF4WIRE, 4, 2, 0, 15);
AccelStepper stepper4(AccelStepper::HALF4WIRE, 27, 12, 14, 13);

/*** SENSOR-VARIABLES ***/
float sensor_1, sensor_2, sensor_3, sensor_4;
bool sensor_1_magnet, sensor_2_magnet, sensor_3_magnet, sensor_4_magnet;
const int hallPin_1 = 35;
const int hallPin_2 = 34;
const int hallPin_3 = 39;
const int hallPin_4 = 36;

/*** BUTTONS-VARIABLES ***/
int buttonPin = 32;
int buttonValue = 0;

int powerButton = 1680;
int modeButton = 310;
int startButton = 1100;
int tolerance = 100;

/*** LED ***/
int led1 = 33;
int led2 = 25;

/*** GENERAL-VARIABLES ***/
int maxSpeed = 1000;
int acceleration = 500;
int speed = -400;
int startPosition = 0;
std::map<String, int> motorNumbers = {
  { "HoursLeft", 0 },
  { "HoursRight", 0 },
  { "MinutesLeft", 0 },
  { "MinutesRight", 0 }
};
int letters[11] = { 6, 7, 8, 9, 10, 0, 1, 2, 3, 4, 5 };
int letterIndex = 0;
std::map<String, float> lastDegree = {
  { "HoursLeft", 0 },
  { "HoursRight", 0 },
  { "MinutesLeft", 0 },
  { "MinutesRight", 0 }
};
std::map<String, int> lastNumber = {
  { "HoursLeft", 5 },
  { "HoursRight", 5 },
  { "MinutesLeft", 5 },
  { "MinutesRight", 5 }
};
float cumulativeDegree = 0;
long timeLastChecked = 0;
int timeInterval = 1000;
AccelStepper *reversedSteppers[] = { &stepper2 };
char timeHour[3], timeMinute[3];

// Create an instance of the web server on port 80
WebServer server(80);

// HTML page with form to update timezone
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>FLAP-DISPLAY</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
    body {
      font-size: 15pt;
      text-align: center;
    }
    h1 {
      font-size: 30pt;
    }
    input,
    select {
      font-size: 15pt;
      width: "100%";
      margin-top: 20px;
      text-align: center;
    }
    form {
      width: "100%";
    }
    </style>
</head>
<body>
    <h1>%CURRENT-TIME%</h1>
    <p id="tz">%TIMEZONE%</p>
    <form action="/update">
        <select name="timezone" id="timezone"></select><br>
        <input type="submit" value="Submit" id="submit">
    </form>

    <script>
    const timezoneSelect = document.getElementById("timezone");
    const timezones = {
      "Africa/Accra": "GMT0",
      "Africa/Abidjan": "GMT0",
      "Africa/Addis Ababa": "EAT-3",
      "Africa/Algiers": "CET-1",
      "Africa/Asmara": "EAT-3",
      "Africa/Bamako": "GMT0",
      "Africa/Bangui": "WAT-1",
      "Africa/Banjul": "GMT0",
      "Africa/Bissau": "GMT0",
      "Africa/Blantyre": "CAT-2",
      "Africa/Brazzaville": "WAT-1",
      "Africa/Bujumbura": "CAT-2",
      "Africa/Casablanca": "WET0",
      "Africa/Ceuta": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Africa/Conakry": "GMT0",
      "Africa/Dakar": "GMT0",
      "Africa/Dar es Salaam": "EAT-3",
      "Africa/Djibouti": "EAT-3",
      "Africa/Douala": "WAT-1",
      "Africa/El Aaiun": "WET0",
      "Africa/Freetown": "GMT0",
      "Africa/Gaborone": "CAT-2",
      "Africa/Harare": "CAT-2",
      "Africa/Johannesburg": "SAST-2",
      "Africa/Kampala": "EAT-3",
      "Africa/Khartoum": "EAT-3",
      "Africa/Kigali": "CAT-2",
      "Africa/Kinshasa": "WAT-1",
      "Africa/Lagos": "WAT-1",
      "Africa/Libreville": "WAT-1",
      "Africa/Lome": "GMT0",
      "Africa/Luanda": "WAT-1",
      "Africa/Lubumbashi": "CAT-2",
      "Africa/Lusaka": "CAT-2",
      "Africa/Malabo": "WAT-1",
      "Africa/Maputo": "CAT-2",
      "Africa/Maseru": "SAST-2",
      "Africa/Mbabane": "SAST-2",
      "Africa/Mogadishu": "EAT-3",
      "Africa/Monrovia": "GMT0",
      "Africa/Nairobi": "EAT-3",
      "Africa/Ndjamena": "WAT-1",
      "Africa/Niamey": "WAT-1",
      "Africa/Nouakchott": "GMT0",
      "Africa/Ouagadougou": "GMT0",
      "Africa/Porto-Novo": "WAT-1",
      "Africa/Sao Tome": "GMT0",
      "Africa/Tripoli": "EET-2",
      "Africa/Tunis": "CET-1",
      "Africa/Windhoek": "WAT-1WAST,M9.1.0,M4.1.0",
      "America/Adak": "HAST10HADT,M3.2.0,M11.1.0",
      "America/Anchorage": "AKST9AKDT,M3.2.0,M11.1.0",
      "America/Anguilla": "AST4",
      "America/Antigua": "AST4",
      "America/Araguaina": "BRT3",
      "America/Argentina/Buenos Aires": "ART3",
      "America/Argentina/Catamarca": "ART3",
      "America/Argentina/Cordoba": "ART3",
      "America/Argentina/Jujuy": "ART3",
      "America/Argentina/La Rioja": "ART3",
      "America/Argentina/Mendoza": "ART3",
      "America/Argentina/Rio Gallegos": "ART3",
      "America/Argentina/Salta": "ART3",
      "America/Argentina/San Juan": "ART3",
      "America/Argentina/Tucuman": "ART3",
      "America/Argentina/Ushuaia": "ART3",
      "America/Aruba": "AST4",
      "America/Asuncion": "PYT4PYST,M10.1.0/0,M4.2.0/0",
      "America/Atikokan": "EST5",
      "America/Bahia": "BRT3",
      "America/Barbados": "AST4",
      "America/Belem": "BRT3",
      "America/Belize": "CST6",
      "America/Blanc-Sablon": "AST4",
      "America/Boa Vista": "AMT4",
      "America/Bogota": "COT5",
      "America/Boise": "MST7MDT,M3.2.0,M11.1.0",
      "America/Cambridge Bay": "MST7MDT,M3.2.0,M11.1.0",
      "America/Campo Grande": "AMT4AMST,M10.3.0/0,M2.3.0/0",
      "America/Cancun": "CST6CDT,M4.1.0,M10.5.0",
      "America/Caracas": "VET4:30",
      "America/Cayenne": "GFT3",
      "America/Cayman": "EST5",
      "America/Chicago": "CST6CDT,M3.2.0,M11.1.0",
      "America/Chihuahua": "MST7MDT,M4.1.0,M10.5.0",
      "America/Costa Rica": "CST6",
      "America/Cuiaba": "AMT4AMST,M10.3.0/0,M2.3.0/0",
      "America/Curacao": "AST4",
      "America/Danmarkshavn": "GMT0",
      "America/Dawson": "PST8PDT,M3.2.0,M11.1.0",
      "America/Dawson Creek": "MST7",
      "America/Denver": "MST7MDT,M3.2.0,M11.1.0",
      "America/Detroit": "EST5EDT,M3.2.0,M11.1.0",
      "America/Dominica": "AST4",
      "America/Edmonton": "MST7MDT,M3.2.0,M11.1.0",
      "America/Eirunepe": "AMT4",
      "America/El Salvador": "CST6",
      "America/Fortaleza": "BRT3",
      "America/Glace Bay": "AST4ADT,M3.2.0,M11.1.0",
      "America/Goose Bay": "AST4ADT,M3.2.0/0:01,M11.1.0/0:01",
      "America/Grand Turk": "EST5EDT,M3.2.0,M11.1.0",
      "America/Grenada": "AST4",
      "America/Guadeloupe": "AST4",
      "America/Guatemala": "CST6",
      "America/Guayaquil": "ECT5",
      "America/Guyana": "GYT4",
      "America/Halifax": "AST4ADT,M3.2.0,M11.1.0",
      "America/Havana": "CST5CDT,M3.2.0/0,M10.5.0/1",
      "America/Hermosillo": "MST7",
      "America/Indiana/Indianapolis": "EST5EDT,M3.2.0,M11.1.0",
      "America/Indiana/Knox": "CST6CDT,M3.2.0,M11.1.0",
      "America/Indiana/Marengo": "EST5EDT,M3.2.0,M11.1.0",
      "America/Indiana/Petersburg": "EST5EDT,M3.2.0,M11.1.0",
      "America/Indiana/Tell City": "CST6CDT,M3.2.0,M11.1.0",
      "America/Indiana/Vevay": "EST5EDT,M3.2.0,M11.1.0",
      "America/Indiana/Vincennes": "EST5EDT,M3.2.0,M11.1.0",
      "America/Indiana/Winamac": "EST5EDT,M3.2.0,M11.1.0",
      "America/Inuvik": "MST7MDT,M3.2.0,M11.1.0",
      "America/Iqaluit": "EST5EDT,M3.2.0,M11.1.0",
      "America/Jamaica": "EST5",
      "America/Juneau": "AKST9AKDT,M3.2.0,M11.1.0",
      "America/Kentucky/Louisville": "EST5EDT,M3.2.0,M11.1.0",
      "America/Kentucky/Monticello": "EST5EDT,M3.2.0,M11.1.0",
      "America/La Paz": "BOT4",
      "America/Lima": "PET5",
      "America/Los Angeles": "PST8PDT,M3.2.0,M11.1.0",
      "America/Maceio": "BRT3",
      "America/Managua": "CST6",
      "America/Manaus": "AMT4",
      "America/Marigot": "AST4",
      "America/Martinique": "AST4",
      "America/Matamoros": "CST6CDT,M3.2.0,M11.1.0",
      "America/Mazatlan": "MST7MDT,M4.1.0,M10.5.0",
      "America/Menominee": "CST6CDT,M3.2.0,M11.1.0",
      "America/Merida": "CST6CDT,M4.1.0,M10.5.0",
      "America/Mexico City": "CST6CDT,M4.1.0,M10.5.0",
      "America/Miquelon": "PMST3PMDT,M3.2.0,M11.1.0",
      "America/Moncton": "AST4ADT,M3.2.0,M11.1.0",
      "America/Monterrey": "CST6CDT,M4.1.0,M10.5.0",
      "America/Montevideo": "UYT3UYST,M10.1.0,M3.2.0",
      "America/Montreal": "EST5EDT,M3.2.0,M11.1.0",
      "America/Montserrat": "AST4",
      "America/Nassau": "EST5EDT,M3.2.0,M11.1.0",
      "America/New York": "EST5EDT,M3.2.0,M11.1.0",
      "America/Nipigon": "EST5EDT,M3.2.0,M11.1.0",
      "America/Nome": "AKST9AKDT,M3.2.0,M11.1.0",
      "America/Noronha": "FNT2",
      "America/North Dakota/Center": "CST6CDT,M3.2.0,M11.1.0",
      "America/North Dakota/New Salem": "CST6CDT,M3.2.0,M11.1.0",
      "America/Ojinaga": "MST7MDT,M3.2.0,M11.1.0",
      "America/Panama": "EST5",
      "America/Pangnirtung": "EST5EDT,M3.2.0,M11.1.0",
      "America/Paramaribo": "SRT3",
      "America/Phoenix": "MST7",
      "America/Port of Spain": "AST4",
      "America/Port-au-Prince": "EST5",
      "America/Porto Velho": "AMT4",
      "America/Puerto Rico": "AST4",
      "America/Rainy River": "CST6CDT,M3.2.0,M11.1.0",
      "America/Rankin Inlet": "CST6CDT,M3.2.0,M11.1.0",
      "America/Recife": "BRT3",
      "America/Regina": "CST6",
      "America/Rio Branco": "AMT4",
      "America/Santa Isabel": "PST8PDT,M4.1.0,M10.5.0",
      "America/Santarem": "BRT3",
      "America/Santo Domingo": "AST4",
      "America/Sao Paulo": "BRT3BRST,M10.3.0/0,M2.3.0/0",
      "America/Scoresbysund": "EGT1EGST,M3.5.0/0,M10.5.0/1",
      "America/Shiprock": "MST7MDT,M3.2.0,M11.1.0",
      "America/St Barthelemy": "AST4",
      "America/St Johns": "NST3:30NDT,M3.2.0/0:01,M11.1.0/0:01",
      "America/St Kitts": "AST4",
      "America/St Lucia": "AST4",
      "America/St Thomas": "AST4",
      "America/St Vincent": "AST4",
      "America/Swift Current": "CST6",
      "America/Tegucigalpa": "CST6",
      "America/Thule": "AST4ADT,M3.2.0,M11.1.0",
      "America/Thunder Bay": "EST5EDT,M3.2.0,M11.1.0",
      "America/Tijuana": "PST8PDT,M3.2.0,M11.1.0",
      "America/Toronto": "EST5EDT,M3.2.0,M11.1.0",
      "America/Tortola": "AST4",
      "America/Vancouver": "PST8PDT,M3.2.0,M11.1.0",
      "America/Whitehorse": "PST8PDT,M3.2.0,M11.1.0",
      "America/Winnipeg": "CST6CDT,M3.2.0,M11.1.0",
      "America/Yakutat": "AKST9AKDT,M3.2.0,M11.1.0",
      "America/Yellowknife": "MST7MDT,M3.2.0,M11.1.0",
      "Antarctica/Casey": "WST-8",
      "Antarctica/Davis": "DAVT-7",
      "Antarctica/DumontDUrville": "DDUT-10",
      "Antarctica/Macquarie": "MIST-11",
      "Antarctica/Mawson": "MAWT-5",
      "Antarctica/McMurdo": "NZST-12NZDT,M9.5.0,M4.1.0/3",
      "Antarctica/Rothera": "ROTT3",
      "Antarctica/South Pole": "NZST-12NZDT,M9.5.0,M4.1.0/3",
      "Antarctica/Syowa": "SYOT-3",
      "Antarctica/Vostok": "VOST-6",
      "Arctic/Longyearbyen": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Asia/Aden": "AST-3",
      "Asia/Almaty": "ALMT-6",
      "Asia/Anadyr": "ANAT-11ANAST,M3.5.0,M10.5.0/3",
      "Asia/Aqtau": "AQTT-5",
      "Asia/Aqtobe": "AQTT-5",
      "Asia/Ashgabat": "TMT-5",
      "Asia/Baghdad": "AST-3",
      "Asia/Bahrain": "AST-3",
      "Asia/Baku": "AZT-4AZST,M3.5.0/4,M10.5.0/5",
      "Asia/Bangkok": "ICT-7",
      "Asia/Beirut": "EET-2EEST,M3.5.0/0,M10.5.0/0",
      "Asia/Bishkek": "KGT-6",
      "Asia/Brunei": "BNT-8",
      "Asia/Choibalsan": "CHOT-8",
      "Asia/Chongqing": "CST-8",
      "Asia/Colombo": "IST-5:30",
      "Asia/Damascus": "EET-2EEST,M4.1.5/0,M10.5.5/0",
      "Asia/Dhaka": "BDT-6",
      "Asia/Dili": "TLT-9",
      "Asia/Dubai": "GST-4",
      "Asia/Dushanbe": "TJT-5",
      "Asia/Gaza": "EET-2EEST,M3.5.6/0:01,M9.1.5",
      "Asia/Harbin": "CST-8",
      "Asia/Ho Chi Minh": "ICT-7",
      "Asia/Hong Kong": "HKT-8",
      "Asia/Hovd": "HOVT-7",
      "Asia/Irkutsk": "IRKT-8IRKST,M3.5.0,M10.5.0/3",
      "Asia/Jakarta": "WIT-7",
      "Asia/Jayapura": "EIT-9",
      "Asia/Kabul": "AFT-4:30",
      "Asia/Kamchatka": "PETT-11PETST,M3.5.0,M10.5.0/3",
      "Asia/Karachi": "PKT-5",
      "Asia/Kashgar": "CST-8",
      "Asia/Kathmandu": "NPT-5:45",
      "Asia/Kolkata": "IST-5:30",
      "Asia/Krasnoyarsk": "KRAT-7KRAST,M3.5.0,M10.5.0/3",
      "Asia/Kuala Lumpur": "MYT-8",
      "Asia/Kuching": "MYT-8",
      "Asia/Kuwait": "AST-3",
      "Asia/Macau": "CST-8",
      "Asia/Magadan": "MAGT-11MAGST,M3.5.0,M10.5.0/3",
      "Asia/Makassar": "CIT-8",
      "Asia/Manila": "PHT-8",
      "Asia/Muscat": "GST-4",
      "Asia/Nicosia": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Asia/Novokuznetsk": "NOVT-6NOVST,M3.5.0,M10.5.0/3",
      "Asia/Novosibirsk": "NOVT-6NOVST,M3.5.0,M10.5.0/3",
      "Asia/Omsk": "OMST-7",
      "Asia/Oral": "ORAT-5",
      "Asia/Phnom Penh": "ICT-7",
      "Asia/Pontianak": "WIT-7",
      "Asia/Pyongyang": "KST-9",
      "Asia/Qatar": "AST-3",
      "Asia/Qyzylorda": "QYZT-6",
      "Asia/Rangoon": "MMT-6:30",
      "Asia/Riyadh": "AST-3",
      "Asia/Sakhalin": "SAKT-10SAKST,M3.5.0,M10.5.0/3",
      "Asia/Samarkand": "UZT-5",
      "Asia/Seoul": "KST-9",
      "Asia/Shanghai": "CST-8",
      "Asia/Singapore": "SGT-8",
      "Asia/Taipei": "CST-8",
      "Asia/Tashkent": "UZT-5",
      "Asia/Tbilisi": "GET-4",
      "Asia/Tehran": "IRST-3:30IRDT,80/0,264/0",
      "Asia/Thimphu": "BTT-6",
      "Asia/Tokyo": "JST-9",
      "Asia/Ulaanbaatar": "ULAT-8",
      "Asia/Urumqi": "CST-8",
      "Asia/Vientiane": "ICT-7",
      "Asia/Vladivostok": "VLAT-10VLAST,M3.5.0,M10.5.0/3",
      "Asia/Yakutsk": "YAKT-9YAKST,M3.5.0,M10.5.0/3",
      "Asia/Yekaterinburg": "YEKT-5YEKST,M3.5.0,M10.5.0/3",
      "Asia/Yerevan": "AMT-4AMST,M3.5.0,M10.5.0/3",
      "Atlantic/Azores": "AZOT1AZOST,M3.5.0/0,M10.5.0/1",
      "Atlantic/Bermuda": "AST4ADT,M3.2.0,M11.1.0",
      "Atlantic/Canary": "WET0WEST,M3.5.0/1,M10.5.0",
      "Atlantic/Cape Verde": "CVT1",
      "Atlantic/Faroe": "WET0WEST,M3.5.0/1,M10.5.0",
      "Atlantic/Madeira": "WET0WEST,M3.5.0/1,M10.5.0",
      "Atlantic/Reykjavik": "GMT0",
      "Atlantic/South Georgia": "GST2",
      "Atlantic/St Helena": "GMT0",
      "Atlantic/Stanley": "FKT4FKST,M9.1.0,M4.3.0",
      "Australia/Adelaide": "CST-9:30CST,M10.1.0,M4.1.0/3",
      "Australia/Brisbane": "EST-10",
      "Australia/Broken Hill": "CST-9:30CST,M10.1.0,M4.1.0/3",
      "Australia/Currie": "EST-10EST,M10.1.0,M4.1.0/3",
      "Australia/Darwin": "CST-9:30",
      "Australia/Eucla": "CWST-8:45",
      "Australia/Hobart": "EST-10EST,M10.1.0,M4.1.0/3",
      "Australia/Lindeman": "EST-10",
      "Australia/Lord Howe": "LHST-10:30LHST-11,M10.1.0,M4.1.0",
      "Australia/Melbourne": "EST-10EST,M10.1.0,M4.1.0/3",
      "Australia/Perth": "WST-8",
      "Australia/Sydney": "EST-10EST,M10.1.0,M4.1.0/3",
      "Europe/Amsterdam": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Andorra": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Athens": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Belgrade": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Berlin": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Bratislava": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Brussels": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Bucharest": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Budapest": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Chisinau": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Copenhagen": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Dublin": "GMT0IST,M3.5.0/1,M10.5.0",
      "Europe/Gibraltar": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Guernsey": "GMT0BST,M3.5.0/1,M10.5.0",
      "Europe/Helsinki": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Isle of Man": "GMT0BST,M3.5.0/1,M10.5.0",
      "Europe/Istanbul": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Jersey": "GMT0BST,M3.5.0/1,M10.5.0",
      "Europe/Kaliningrad": "EET-2EEST,M3.5.0,M10.5.0/3",
      "Europe/Kiev": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Lisbon": "WET0WEST,M3.5.0/1,M10.5.0",
      "Europe/Ljubljana": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/London": "GMT0BST,M3.5.0/1,M10.5.0",
      "Europe/Luxembourg": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Madrid": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Malta": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Mariehamn": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Minsk": "EET-2EEST,M3.5.0,M10.5.0/3",
      "Europe/Monaco": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Moscow": "MSK-4",
      "Europe/Oslo": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Paris": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Podgorica": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Prague": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Riga": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Rome": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Samara": "SAMT-3SAMST,M3.5.0,M10.5.0/3",
      "Europe/San Marino": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Sarajevo": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Simferopol": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Skopje": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Sofia": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Stockholm": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Tallinn": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Tirane": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Uzhgorod": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Vaduz": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Vatican": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Vienna": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Vilnius": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Volgograd": "VOLT-3VOLST,M3.5.0,M10.5.0/3",
      "Europe/Warsaw": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Zagreb": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Europe/Zaporozhye": "EET-2EEST,M3.5.0/3,M10.5.0/4",
      "Europe/Zurich": "CET-1CEST,M3.5.0,M10.5.0/3",
      "Indian/Antananarivo": "EAT-3",
      "Indian/Chagos": "IOT-6",
      "Indian/Christmas": "CXT-7",
      "Indian/Cocos": "CCT-6:30",
      "Indian/Comoro": "EAT-3",
      "Indian/Kerguelen": "TFT-5",
      "Indian/Mahe": "SCT-4",
      "Indian/Maldives": "MVT-5",
      "Indian/Mauritius": "MUT-4",
      "Indian/Mayotte": "EAT-3",
      "Indian/Reunion": "RET-4",
      "Pacific/Apia": "WST11",
      "Pacific/Auckland": "NZST-12NZDT,M9.5.0,M4.1.0/3",
      "Pacific/Chatham": "CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45",
      "Pacific/Efate": "VUT-11",
      "Pacific/Enderbury": "PHOT-13",
      "Pacific/Fakaofo": "TKT10",
      "Pacific/Fiji": "FJT-12",
      "Pacific/Funafuti": "TVT-12",
      "Pacific/Galapagos": "GALT6",
      "Pacific/Gambier": "GAMT9",
      "Pacific/Guadalcanal": "SBT-11",
      "Pacific/Guam": "ChST-10",
      "Pacific/Honolulu": "HST10",
      "Pacific/Johnston": "HST10",
      "Pacific/Kiritimati": "LINT-14",
      "Pacific/Kosrae": "KOST-11",
      "Pacific/Kwajalein": "MHT-12",
      "Pacific/Majuro": "MHT-12",
      "Pacific/Marquesas": "MART9:30",
      "Pacific/Midway": "SST11",
      "Pacific/Nauru": "NRT-12",
      "Pacific/Niue": "NUT11",
      "Pacific/Norfolk": "NFT-11:30",
      "Pacific/Noumea": "NCT-11",
      "Pacific/Pago Pago": "SST11",
      "Pacific/Palau": "PWT-9",
      "Pacific/Pitcairn": "PST8",
      "Pacific/Ponape": "PONT-11",
      "Pacific/Port Moresby": "PGT-10",
      "Pacific/Rarotonga": "CKT10",
      "Pacific/Saipan": "ChST-10",
      "Pacific/Tahiti": "TAHT10",
      "Pacific/Tarawa": "GILT-12",
      "Pacific/Tongatapu": "TOT-13",
      "Pacific/Truk": "TRUT-10",
      "Pacific/Wake": "WAKT-12",
      "Pacific/Wallis": "WFT-12"
    };

    for (let tz in timezones) {
      const option = document.createElement("option");
      option.value = timezones[tz];
      option.innerText = tz;
      if(tz === "Europe/Berlin") {
        option.selected = true;
      }
      timezoneSelect.appendChild(option);
    }

    </script>
</body>
</html>
)rawliteral";

/* 
  Overflow Problem weil zu hoch gezählt?
  ==> Float Overflow mit unserer Geschwindigkeit nach ca. 1.960716 x 10^31 Jahren 
  ==> Kein Problem
*/

// TODO: Counter, durch Modi auswählbar
// ==> Doch eher schlecht, Delay beim Zählen

void setup(void) {
  Serial.begin(115200);

  pinMode(hallPin_1, INPUT);
  pinMode(hallPin_2, INPUT);
  pinMode(hallPin_3, INPUT);
  pinMode(hallPin_4, INPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  delay(2000);

  setupAccessPoint();
  setupWifi();
  digitalWrite(led1, LOW);

  setupMotors();

  updateTime();
  

  server.on("/", handleRoot);
  server.on("/update", handleUpdateTimezone);
  
  server.begin();

}

void loop(void) {
  buttonValue = analogRead(buttonPin);
  int pressedButton = buttonPressed();

  if (pressedButton == 1) {
    // Power-Button
    ESP.restart();
  }
  else if (pressedButton == 2) {
    // Mode-Button
    //digitalWrite(led1, HIGH);
    //digitalWrite(led2, HIGH);
  } 
  else if (pressedButton == 3) {
    // Start-Button
    //digitalWrite(led1, LOW);
    //digitalWrite(led2, LOW);
  } else {
    // No button pressed
  }

  if (!sensor_1_magnet) {
    sensor_1 = digitalRead(hallPin_1);
    calibrateMotor(&stepper1, &sensor_1, &sensor_1_magnet);
  }
  else if (!sensor_2_magnet) {
    sensor_2 = digitalRead(hallPin_2);
    calibrateMotor(&stepper2, &sensor_2, &sensor_2_magnet);
  }
  else if(!sensor_3_magnet) {
    sensor_3 = digitalRead(hallPin_3);
    calibrateMotor(&stepper3, &sensor_3, &sensor_3_magnet);
  }
  else if(!sensor_4_magnet) {
    sensor_4 = digitalRead(hallPin_4);
    calibrateMotor(&stepper4, &sensor_4, &sensor_4_magnet);
  } 
  else {
    digitalWrite(led2, LOW);
    if (millis() - timeLastChecked > timeInterval) {
      updateTime();
      timeLastChecked = millis();
    } else {
      rotateToNumber(&stepper4, "MinutesRight");
      rotateToNumber(&stepper3, "MinutesLeft");
      rotateToNumber(&stepper2, "HoursRight");
      rotateToNumber(&stepper1, "HoursLeft");
    }
  }
  
  // Handle all client requests.
  server.handleClient();
}

void setupAccessPoint() {
  Serial.print("Setting up AP with SSID: ");
  Serial.println(ap_ssid);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setupWifi() {
  int i = 0;  
  int numberOfNetworks = sizeof(ssid) / sizeof(ssid[0]);

  while(WiFi.status() != WL_CONNECTED && i < numberOfNetworks) {
    Serial.print("Attempting to connect to ");
    Serial.println(ssid[i]);
    WiFi.begin(ssid[i], password[i]);
    int j = 0;
    while (WiFi.status() != WL_CONNECTED && j < maxAttemptsToConnectToWifi) {
      delay(500);
      Serial.print(".");
      j++;
    }

    i++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
  } else {
      Serial.println("Could not connect to any of the specified networks");
  }

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  setTimezone(currentTimezone);
  updateTime();
}

void setupMotors() {
  stepper1.setMaxSpeed(maxSpeed);
  stepper1.setAcceleration(acceleration);
  stepper1.setSpeed(speed);
  stepper1.setCurrentPosition(startPosition);

  stepper2.setMaxSpeed(maxSpeed);
  stepper2.setAcceleration(acceleration);
  stepper2.setSpeed(speed);
  stepper2.setCurrentPosition(startPosition);

  stepper3.setMaxSpeed(maxSpeed);
  stepper3.setAcceleration(acceleration);
  stepper3.setSpeed(speed);
  stepper3.setCurrentPosition(startPosition);

  stepper4.setMaxSpeed(maxSpeed);
  stepper4.setAcceleration(acceleration);
  stepper4.setSpeed(speed);
  stepper4.setCurrentPosition(startPosition);
}

int buttonPressed() {
  if(buttonValue <= powerButton + tolerance && buttonValue >= powerButton - tolerance) return 1;
  if(buttonValue <= modeButton + tolerance && buttonValue >= modeButton - tolerance) return 2;
  if(buttonValue <= startButton + tolerance && buttonValue >= startButton - tolerance) return 3;
  return -1;
}

void calibrateMotor(AccelStepper *stepper, float *sensor, bool *sensor_magnet) {
  if (*sensor == LOW) {
    *sensor_magnet = true;
    Serial.println("Magnet detected, position set to 0");
    stepper->setCurrentPosition(0);

    if (isStepperInReverseList(stepper)) {
      adjustPosition(stepper, -33);
    } else if (stepper == &stepper4) {
      adjustPosition(stepper, 30);
    } else {
      adjustPosition(stepper, 15);
    } 
  } else {
    // Keep the motor running
    if (isStepperInReverseList(stepper)) {
      stepper->setSpeed(-speed * 4);  // turn backwards
    } else {
      stepper->setSpeed(speed * 4);
    }

    stepper->runSpeed();
  }
}

float calcStep(float degree) {
  float direction = -1;  // -1 => clockwise, 1 => counterclockwise
  return degree * SteppDegree * direction;
}

void adjustPosition(AccelStepper *stepper, int degreeToAdjustBy) {
  stepper->moveTo(calcStep(degreeToAdjustBy));
  while (stepper->distanceToGo() != 0) {
    stepper->run();
  }
  stepper->setCurrentPosition(0);
}

bool isStepperInReverseList(AccelStepper *stepper) {
  for (int i = 0; i < sizeof(reversedSteppers) / sizeof(reversedSteppers[0]); i++) {
    if (stepper == reversedSteppers[i]) {
      return true;
    }
  }
  return false;
}

/*** WEB-SERVER ***/

void handleRoot() {
  updateTime();
  String htmlPage = HTML_PAGE;
  htmlPage.replace("%TIMEZONE%", currentTimezone);
  htmlPage.replace("%CURRENT-TIME%", String(timeHour)+":"+String(timeMinute));
  server.send(200, "text/html", htmlPage);
}

void handleUpdateTimezone() {
  if (server.hasArg("timezone")) {
    String newTimezone = server.arg("timezone");
    newTimezone.toCharArray(currentTimezone, 100);
    setTimezone(currentTimezone);
    Serial.print("Changed timezone to ");
    Serial.println(currentTimezone);
    updateTime();
  }
  handleRoot();
}


/*** Functions for time ***/

void setTimezone(const char *timezone) {
  Serial.print("Setting Timezone to ");
  Serial.println(timezone);
  setenv("TZ", timezone, 1);
  tzset();
}

void updateTime() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println("Updating time");

  strftime(timeHour, 3, "%H", &timeinfo);
  strftime(timeMinute, 3, "%M", &timeinfo);

  Serial.print("Current time: ");
  Serial.print(timeHour);
  Serial.print(":");
  Serial.println(timeMinute);

  motorNumbers["HoursLeft"] = timeHour[0] - '0';
  motorNumbers["HoursRight"] = timeHour[1] - '0';
  motorNumbers["MinutesLeft"] = timeMinute[0] - '0';
  motorNumbers["MinutesRight"] = timeMinute[1] - '0';

}


/*** Functions to turn flaps ***/

void rotateToNumber(AccelStepper *stepper, String position) {
  int number = motorNumbers[position];
  if (number < 11) {
    Serial.print("Moving to: ");
    Serial.println(number);

    int numberToTurnBy = number - lastNumber[position];
    if (numberToTurnBy < 0) {
      numberToTurnBy = 11 + numberToTurnBy;
    }

    float degree = lastDegree[position] + (360.0 / 11 * numberToTurnBy);
    lastDegree[position] = degree;
    lastNumber[position] = number;

    if (isStepperInReverseList(stepper)) {
      stepper->moveTo(-calcStep(degree));  // turn backwards, if stepper==stepper4
    } else {
      stepper->moveTo(calcStep(degree));
    }

    while (stepper->distanceToGo() != 0) {
      stepper->run();
    }
  }
}

void rotatingFlaps(AccelStepper *stepper, int speed) {
  if (isStepperInReverseList(stepper)) {
      stepper->setSpeed(speed);  // turn backwards, if stepper==stepper4
    } else {
      stepper->setSpeed(-speed);
    }

    stepper->runSpeed();
}

/*
void rotateThroughAllFlaps() {
  while(letterIndex < 11) {
    rotateToNumber(&stepper1, letterIndex);
    
    letterIndex++;
    delay(1000);
  }
  
  // while(1) {} // Stop motor from moving.
}

void readSerialInputToTurnFlaps() {
  if (Serial.available() > 0) {
    int input = Serial.read();
    if(input >= 48 && input <= 57) {
      rotateToNumber(&stepper1, input-48);
    }
    else if(input == 32) {
      rotateToNumber(&stepper1, 10);
    }
  }
}
*/
