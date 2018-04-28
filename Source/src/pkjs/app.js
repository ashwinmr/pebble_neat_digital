// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);
// Get message keys for obtaining clay settings
var messageKeys = require('message_keys');
// Global variable to hold all settings
var g_settings = {};

// Define icon enum (no enum in javascript so we use a container)
// Copy this to the C program
var icon_number = {
  ICON_DEFAULT:0,
  ICON_CLEAR_DAY:1,
  ICON_CLEAR_NIGHT:2,
  ICON_CLOUDY:3,
  ICON_FOG:4,
  ICON_PARTLYCLOUDY:5,
  ICON_PARTLYSUNNY:6,
  ICON_RAIN:7,
  ICON_SLEET:8,
  ICON_SNOW:9,
  ICON_TSTORMS:10,
  ICON_SUNRISE:11,
  ICON_SUNSET:12
};

// Funtion to get icon number from icon name
function get_icon_number (icon_name,current_hour,sunrise_hour,sunset_hour){
  switch(icon_name){
    case "partlycloudy":
    case "mostlycloudy":
      return icon_number.ICON_PARTLYCLOUDY;
    case "cloudy":
      return icon_number.ICON_CLOUDY;
    case "tstorms":
      return icon_number.ICON_TSTORMS;
    case "sunny":
      return icon_number.ICON_CLEAR_DAY;
    case "snow":
    case "flurries":
      return icon_number.ICON_SNOW;
    case "sleet":
      return icon_number.ICON_SLEET;
    case "rain":
      return icon_number.ICON_RAIN;
    case "partlysunny":
    case "mostlysunny":
      return icon_number.ICON_PARTLYSUNNY;
    case "hazy":
    case "fog":
      return icon_number.ICON_FOG;
    default:
      if(current_hour == sunrise_hour)
        return icon_number.ICON_SUNRISE;
      else if(current_hour == sunset_hour)
        return icon_number.ICON_SUNSET;
      else if(current_hour < sunrise_hour || current_hour > sunset_hour)
        return icon_number.ICON_CLEAR_NIGHT;
      else
        return icon_number.ICON_CLEAR_DAY;
  }
}

// Function to send settings to pebble
function send_settings(settings){
  Pebble.sendAppMessage(settings,function(e) {
          console.log("Settings sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending settings to Pebble!");
        }); 
}
      


// Function to send API request
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.open(type, url);
  xhr.send();
  xhr.onload = function () {
    callback(this.responseText);
  }; 
};

// Function to get the weather and send to watch
function getWeatherUrl(url){
  
  // Send an API request
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather and astronomy info
      var json = JSON.parse(responseText);
      
      // Temperature 
      var temp_unit =  g_settings[messageKeys.TEMPERATURE_UNIT];
      console.log("Temperature unit is "+temp_unit);
      var temp_type =  g_settings[messageKeys.TEMPERATURE_TYPE];
      console.log("Temperature type is "+temp_type);
      var temperature = 0; // Initialize temperature
      
      if(temp_unit == "C"){
        if(temp_type == "S"){
          temperature = Math.round(json.current_observation.temp_c);
        }
        else
          temperature = Math.round(json.current_observation.feelslike_c);
      }
      else{
        if(temp_type == "S"){
          temperature = Math.round(json.current_observation.temp_f);
        }
        else
          temperature = Math.round(json.current_observation.feelslike_f);
      }
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.current_observation.weather;      
      console.log("Conditions are " + conditions);
      
      // Current hour
      var current_hour = Math.round(json.moon_phase.current_time.hour);
      console.log("Current hour is " + current_hour);
      
      // Sunrise hour
      var sunrise_hour = Math.round(json.sun_phase.sunrise.hour);
      console.log("Sunrise hour is " + sunrise_hour);
      
      // Sunset hour
      var sunset_hour = Math.round(json.sun_phase.sunset.hour);
      console.log("Sunset hour is " + sunset_hour);
      
      // Icon
      var icon_name = json.current_observation.icon;
      console.log("Icon name is " + icon_name);
      var icon_number = get_icon_number(icon_name,current_hour,sunrise_hour,sunset_hour);
      console.log("Icon number is " + icon_number);
      
      // Add to our global settings dictionary
      // Should be same as those defined in settings
      g_settings[messageKeys.TEMPERATURE] = temperature;
      g_settings[messageKeys.CONDITIONS] = conditions;
      g_settings[messageKeys.ICON_NUMBER] = icon_number;
      
      // Send settings to pebble
      send_settings(g_settings);
    });   
}

// Function to get the weather without the URL
function setSettings(){

  var api_key_WU = g_settings[messageKeys.API_KEY_WU];
  console.log("WU Api key is " + api_key_WU);
  
  // Get weather settings only if API key is set
  if(api_key_WU){
      var location_source =  g_settings[messageKeys.LOCATION_SOURCE];
      console.log("Location source is " + location_source);
      if(location_source == "S"){
        var location_state_code =  g_settings[messageKeys.LOCATION_STATE_CODE];
        console.log("State code is "+location_state_code);
        var location_city =  g_settings[messageKeys.LOCATION_CITY];
        console.log("City is "+location_city);
        var url = "http://api.wunderground.com/api/" + api_key_WU + "/conditions/astronomy/q/" + location_state_code + "/" + location_city + ".json";
        console.log("Url is " +url);
        getWeatherUrl(url);
      }
      else{
        navigator.geolocation.getCurrentPosition(function(pos){
        var url = "http://api.wunderground.com/api/" + api_key_WU + "/conditions/astronomy/q/" + pos.coords.latitude + "," + pos.coords.longitude + ".json";
        console.log("Url is " +url);
        getWeatherUrl(url);
        }, // Success callback function
        function(err){console.log("Error requesting location!");}, // Failure callback function
        {timeout: 15000, maximumAge: 60000}
        );
      }
    }
  else{
    // Otherwise, just send the basic settings like color
    send_settings(g_settings);
  }
}

// Listen for when an AppMessage is received
// This waits for the c program to send a message before it gets the weather and sends back the result
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received from pebble!");
    setSettings();
  }                     
);

// Listen for when the watchface is opened and get the weather
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS is ready!");
        
    // Get the global settings from local storage
    // This is done because, when you reload the watchface, the whole program is reloaded.
    // So all settings from clay_settings will be lost until the settings page is used in the phone again.
    g_settings = JSON.parse(localStorage.getItem("g_settings"));
    
    // If there is no g_settings in local storage, set default values
    if(!g_settings){
      g_settings={
        "COLOR_TIME_BG":0xFFFFFF,
        "COLOR_WEATHER_BG" : 0x000000,
        "COLOR_BATTERY_BAR" : 0xFFFFFF,
        "COLOR_TIME_STROKE" : 0x000000,
        "COLOR_WEATHER_STROKE" : 0xFFFFFF,
        "DATE_FORMAT" :  "%a %m/%d",
        "VIBRATE_DISCONNECT":0,
        "UPDATE_INTERVAL":"20",
        "SHOW_CONDITIONS" : 1,
        "TEMPERATURE_TYPE":"S",
        "TEMPERATURE_UNIT":"C",
        "LOCATION_SOURCE":"A",
      };
    }
    setSettings();
  }
);

// This event occurs when we use the watchface settings page on the phone and close it
Pebble.addEventListener('webviewclosed', function(e) {
  if (e && !e.response) {
    return;
  }

  // Get the keys and values from each config item
  g_settings = clay.getSettings(e.response);
  
  // Store this to local storage for when the window is reloaded and app setting page is not used again
  // Local storage does not allow you to store objects. It turns them to strings. 
  // So stringigy while storing, and JSON parse while retreiving.
  localStorage.setItem("g_settings",JSON.stringify(g_settings));
  
  // Get the weather which will use this updated settings dictionary
  setSettings();
  
});
