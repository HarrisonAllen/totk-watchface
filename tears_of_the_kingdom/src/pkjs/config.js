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
        "type": "submit",
        "defaultValue": "Submit"
    }
  ];
  