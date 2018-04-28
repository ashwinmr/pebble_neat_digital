module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Appearance"
      },
      {
        "type": "color",
        "messageKey": "COLOR_TIME_BG",
        "defaultValue": "0xFFFFFF",
        "label": "Time Background"
      },
      {
        "type": "color",
        "messageKey": "COLOR_WEATHER_BG",
        "defaultValue": "0x000000",
        "label": "Weather Background"
      },
      {
        "type": "color",
        "messageKey": "COLOR_BATTERY_BAR",
        "defaultValue": "0xFFFFFF",
        "label": "Battery Bar"
      },
      {
        "type": "color",
        "messageKey": "COLOR_TIME_STROKE",
        "defaultValue": "0x000000",
        "label": "Time Stroke"
      },
      {
        "type": "color",
        "messageKey": "COLOR_WEATHER_STROKE",
        "defaultValue": "0xFFFFFF",
        "label": "Weather Stroke"
      },
      {
        "type": "select",
        "messageKey": "DATE_FORMAT",
        "defaultValue": "%a %m/%d",
        "label": "Date Format",
        "options": [
          { 
            "label": "Day MO/DA", 
            "value": "%a %m/%d", 
          },
          { 
            "label": "Day DA/MO",
            "value": "%a %d/%m",
          },
        ]
      },
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Settings"
      },
      {
      "type": "toggle",
      "messageKey": "VIBRATE_DISCONNECT",
      "label": "Vibrate on disconnect",
      "defaultValue": 0
      },
      {
        "type": "input",
        "messageKey": "API_KEY_WU",
        "defaultValue": "",
        "label": "Weather Underground API Key",
          "attributes": {
            "placeholder": "eg: 12345ab67cd89ef0",
            "limit": 18,
            "type": "key"
          }
      },
      {
        "type": "select",
        "messageKey": "UPDATE_INTERVAL",
        "defaultValue": "20",
        "label": "Update Interval",
        "options": [
          { 
            "label": "5 Min", 
            "value": "5", 
          },
          { 
            "label": "10 Min",
            "value": "10",
          },
          { 
            "label": "15 Min",
            "value": "15", 
          },
          { 
            "label": "20 Min",
            "value": "20", 
          },
          { 
            "label": "30 Min",
            "value": "30", 
          },
          { 
            "label": "60 Min",
            "value": "60", 
          }
        ]
      },
      {
      "type": "toggle",
      "messageKey": "SHOW_CONDITIONS",
      "label": "Show Conditions",
      "defaultValue": 1
      },
      {
        "type": "radiogroup",
        "messageKey": "TEMPERATURE_TYPE",
        "label": "Temperature Type",
        "defaultValue": "S",
        "options": [
          { 
            "label": "Standard", 
            "value": "S" 
          },
          { 
            "label": "Feels like", 
            "value": "FL" 
          },
        ]
      },
      {
        "type": "radiogroup",
        "messageKey": "TEMPERATURE_UNIT",
        "label": "Temperature Unit",
        "defaultValue": "C",
        "options": [
          { 
            "label": "C", 
            "value": "C" 
          },
          { 
            "label": "F", 
            "value": "F" 
          },
        ]
      },
      {
        "type": "radiogroup",
        "messageKey": "LOCATION_SOURCE",
        "label": "Location Source",
        "defaultValue": "A",
        "options": [
          { 
            "label": "Automatic (GPS)", 
            "value": "A" 
          },
          { 
            "label": "Set", 
            "value": "S" 
          },
        ]
      },
      {
        "type": "input",
        "messageKey": "LOCATION_STATE_CODE",
        "defaultValue": "",
        "label": "Location State Code",
          "attributes": {
            "placeholder": "eg: CA",
            "limit": 4,
            "type": "location"
          }
      },
      {
        "type": "input",
        "messageKey": "LOCATION_CITY",
        "defaultValue": "",
        "label": "Location City",
          "attributes": {
            "placeholder": "eg: Los Angeles",
            "limit": 20,
            "type": "location"
          }
      },
    ]
  },
      {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Donate"
      },
      {
        "type": "text",
        "defaultValue": "If you like this watchface, buy me coffee ?",
      },
      {
        "type": "text",
        "defaultValue": '<form action="https://www.paypal.com/cgi-bin/webscr" method="post" target="_top"><input type="hidden" name="cmd" value="_s-xclick"><input type="hidden" name="encrypted" value="-----BEGIN PKCS7-----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-----END PKCS7-----"><input type="image" src="https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online!"><img alt="" border="0" src="https://www.paypalobjects.com/en_US/i/scr/pixel.gif" width="1" height="1"></form>',
      },
      ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
