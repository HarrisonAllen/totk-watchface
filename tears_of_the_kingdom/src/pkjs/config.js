module.exports = [
    {
      "type": "heading",
      "defaultValue": "Tears of the Kingdom Configuration"
    },
    { 
      "type": "text",
      "defaultValue": "Set up your Purah Pad here"
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Vibration"
        },
        {
          "type": "toggle",
          "messageKey": "VibrateOnDisc",
          "label": "Vibrate on bluetooth disconnect",
          "defaultValue": true
        }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Time and Date"
        },
        {
          "type": "toggle",
          "messageKey": "AmericanDate",
          "label": "Use American date format",
          "defaultValue": true,
          "description": "Set false for '01 Jan', true for 'Jan 01'"
        }
      ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Weather"
            },
            {
                "type": "input",
                "messageKey": "OpenWeatherAPIKey",
                "defaultValue": "",
                "label": "Open Weather API Key",
                "description": "Leave blank to use my personal key, but it may stop working in the future.",
                "attributes": {
                    "type": "text"
                }
            },
            {
                  "type": "slider",
                  "label": "Weather update rate (in minutes):",
                  "messageKey": "WeatherCheckRate",
                  "defaultValue": "30",
                  "min": 15,
                  "max": 120,
                  "step": 15
              },
              {
                  "type": "toggle",
                  "messageKey": "UseCurrentLocation",
                  "label": "Use current location",
                  "defaultValue": true
              },
              {
                  "type": "input",
                  "messageKey": "Latitude",
                  "defaultValue": "",
                  "label": "Manual Latitude",
                  "attributes": {
                      "placeholder": "eg: 42.36",
                      "type": "number"
                  }
              },
              {
                  "type": "input",
                  "messageKey": "Longitude",
                  "defaultValue": "",
                  "label": "Manual Longitude",
                  "attributes": {
                      "placeholder": "eg: -71.10",
                      "type": "number"
                  }
              },
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "UI Colors"
            },
            {
              "type": "color",
              "messageKey": "ColorBackground",
              "defaultValue": "0x000000",
              "label": "Background Color"
            },
            {
              "type": "color",
              "messageKey": "ColorOuroboros",
              "defaultValue": "0x55FF55",
              "label": "Ouroboros Color"
            },
            {
              "type": "color",
              "messageKey": "ColorTime",
              "defaultValue": "0x55FF55",
              "label": "Time Text Color"
            },
            {
              "type": "color",
              "messageKey": "ColorDate",
              "defaultValue": "0x55FF55",
              "label": "Date Text Color"
            },
            {
              "type": "color",
              "messageKey": "ColorDay",
              "defaultValue": "0x55FF55",
              "label": "Day of the Week Text Color"
            },
            {
              "type": "color",
              "messageKey": "ColorDayIcon",
              "defaultValue": "0x55FF55",
              "label": "Day of the Week Icon Color"
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Battery Colors"
            },
            {
              "type": "color",
              "messageKey": "ColorBatteryBackground",
              "defaultValue": "0x555555",
              "label": "Battery Background Color"
            },
            {
              "type": "color",
              "messageKey": "ColorBatteryOk",
              "defaultValue": "0x00FF00",
              "label": "Battery > 10% Color"
            },
            {
              "type": "color",
              "messageKey": "ColorBatteryLow",
              "defaultValue": "0xFF0000",
              "label": "Battery <= 10% Color"
            },
            {
              "type": "color",
              "messageKey": "ColorBatteryCharging",
              "defaultValue": "0xFFFF00",
              "label": "Battery Charging Color"
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Weather Colors"
            },
            {
              "type": "color",
              "messageKey": "ColorWeatherIcon",
              "defaultValue": "0xFFFFFF",
              "label": "Weather Icon Color"
            },
            {
              "type": "color",
              "messageKey": "ColorWeatherVeryCold",
              "defaultValue": "0xAAFFFF",
              "label": "Temperature: Very Cold Color"
            },
            {
              "type": "color",
              "messageKey": "ColorWeatherCold",
              "defaultValue": "0x00FFFF",
              "label": "Temperature: Cold Color"
            },
            {
              "type": "color",
              "messageKey": "ColorWeatherHot",
              "defaultValue": "0xFFAA00",
              "label": "Temperature: Hot Color"
            },
            {
              "type": "color",
              "messageKey": "ColorWeatherVeryHot",
              "defaultValue": "0xFF5500",
              "label": "Temperature: Very Hot Color"
            },
            {
              "type": "color",
              "messageKey": "ColorWeatherBackground",
              "defaultValue": "0x555555",
              "label": "Temperature Background Color"
            },
            {
              "type": "color",
              "messageKey": "ColorWeatherNeedle",
              "defaultValue": "0xFFFFFF",
              "label": "Temperature Needle Color"
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Bluetooth Colors"
            },
            {
              "type": "color",
              "messageKey": "ColorBluetoothConnected",
              "defaultValue": "0x55FFFF",
              "label": "Bluetooth Connected Color"
            },
            {
              "type": "color",
              "messageKey": "ColorBluetoothDisconnected",
              "defaultValue": "0x555555",
              "label": "Bluetooth Disconnected Color"
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Submit"
    }
  ];
  