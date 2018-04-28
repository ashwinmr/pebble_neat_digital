#include <pebble.h>

// Global variables
Window* g_window; // Window pointer

struct tm* g_time;
bool g_bluetooth_status; // Bluetooth status, connected or not

TextLayer* g_time_block; // Time block pointer
GRect g_time_block_rect; // Time block bounding area

TextLayer* g_date_block; // Date block pointer
GRect g_date_block_rect; // Date block bounding area

TextLayer* g_temp_block; // Temperature block pointer
GRect g_temp_block_rect; // Temperature block bounding area

TextLayer* g_cond_block; // Conditions block pointer
GRect g_cond_block_rect; // Conditions block bounding area

GBitmap* g_icon_image; // icon image pointer
BitmapLayer* g_icon_block; // icon block pointer
GRect g_icon_block_rect; // icon block bounding area

enum ICON_NUMBER {
    ICON_DEFAULT,
    ICON_CLEAR_DAY,
    ICON_CLEAR_NIGHT,
    ICON_CLOUDY,
    ICON_FOG,
    ICON_PARTLYCLOUDY,
    ICON_PARTLYSUNNY,
    ICON_RAIN,
    ICON_SLEET,
    ICON_SNOW,
    ICON_TSTORMS,
    ICON_SUNRISE,
    ICON_SUNSET
};

Layer* g_battery_block; // Battery block pointer
int g_battery_level; // Battery percentage
GRect g_battery_block_rect; // Battery block bounding area

// Define clay settings structure for persintent storage
typedef struct PersistSettings {
  GColor color_time_bg;
  GColor color_weather_bg;
  GColor color_battery_bar;
  GColor color_time_stroke;
  GColor color_weather_stroke;
  char date_format[15];
  bool vibrate_disconnect;
  int update_interval;
  bool show_conditions;
  char time[10];
  char date[10];
  char temperature[8];
  char conditions[32];
  int icon_number;
} PersistSettings;

// An instance of the struct
static PersistSettings persist_settings;

// Function to initialize all globals
void init_globals(){
  
  // Set default persistent settings
  persist_settings.color_time_bg = GColorWhite;
  persist_settings.color_weather_bg = GColorBlack;
  persist_settings.color_battery_bar = GColorWhite;
  persist_settings.color_time_stroke = GColorBlack;
  persist_settings.color_weather_stroke = GColorWhite;
  snprintf(persist_settings.date_format, sizeof(persist_settings.date_format), "%%a %%m/%%d");
  persist_settings.vibrate_disconnect = false;
  persist_settings.update_interval = 20;
  persist_settings.show_conditions = true;
  snprintf(persist_settings.time, sizeof(persist_settings.time), " ");
  snprintf(persist_settings.date, sizeof(persist_settings.date), " ");
  snprintf(persist_settings.conditions, sizeof(persist_settings.conditions), " ");
  snprintf(persist_settings.temperature, sizeof(persist_settings.temperature)," ");
  persist_settings.icon_number = ICON_DEFAULT;
  
  // Read settings from persistent storage to override defaults if they exist
  // Always check if data exists to prevent crash while reading
  if (persist_exists(MESSAGE_KEY_SETTINGS)) {
    // Read persisted value
    persist_read_data(MESSAGE_KEY_SETTINGS, &persist_settings, sizeof(persist_settings));
  }
  
  // Global time initialize
  time_t now = time(NULL);
  g_time = localtime(&now);
  
  // Battery block
  g_battery_block_rect = GRect(0,0,144,10); 

  // Time block
  g_time_block_rect = GRect(0,20,144,55); 

  // Date block
  g_date_block_rect = GRect(0,63,144,30); 

  // Icon Block
  g_icon_block_rect = GRect(5,102,72,40);

  // Temperature block
  g_temp_block_rect = GRect(72,104,72,40); 

  // Conditions block
  g_cond_block_rect = GRect(0,137,144,28); 

}


// Funciton called to update the battery block
static void battery_update_proc(Layer *layer, GContext *ctx){
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)g_battery_level / 100.0F) * (float)bounds.size.w);

  // Draw the bar
  graphics_context_set_fill_color(ctx,persist_settings.color_battery_bar);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h),8, GCornersRight);
}

// Callback function which gets called when battery status changes
static void battery_callback(BatteryChargeState state) {
  g_battery_level = state.charge_percent; // Record the new battery level
  layer_mark_dirty(g_battery_block); // Set layer to be redrawn
}


// Function to update the displayed time and date
static void update_time_date(){

  if(g_bluetooth_status){
    strftime(persist_settings.time, sizeof(persist_settings.time), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M",g_time);
  }
  else{
    strftime(persist_settings.time, sizeof(persist_settings.time), clock_is_24h_style() ?
                                          "%H:%M !" : "%I:%M !",g_time);
  }
  
  // Display this time on the time text layer
  text_layer_set_text(g_time_block,persist_settings.time);
  
  strftime(persist_settings.date, sizeof(persist_settings.date),persist_settings.date_format,g_time);
  // Display this date on the date text layer
  text_layer_set_text(g_date_block,persist_settings.date);
}

// Callback function which gets called when a minute is passed
static void minute_callback(struct tm *tick_time, TimeUnits units_changed) {
  
  // Update global time
  time_t now = time(NULL);
  g_time = localtime(&now);
  
  update_time_date();
  
  // Get weather update every 15 minutes
  if(tick_time->tm_min % persist_settings.update_interval == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter,0,0); // Send blank message
    dict_write_end(iter);

    // Send the message!
    app_message_outbox_send();
  }
}

// Callback function which gets called when bluetooth conneciton updates
static void bluetooth_callback(bool connected) {
  g_bluetooth_status = connected;
  if(persist_settings.vibrate_disconnect){
    if(!connected)
      vibes_double_pulse(); // Vibrate if disconnected
  }
  update_time_date(); // The time_date block shows bluetooth
}


// Function to update the icon
void update_icon(int icon_number){
  
  // Clear memory if used
  if(g_icon_image)
    gbitmap_destroy(g_icon_image);
  
  // Find the correct icon
  switch(icon_number){
    case ICON_CLEAR_DAY:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEAR_DAY);
    break;
    
    case ICON_CLEAR_NIGHT:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEAR_NIGHT);
    break;
    
    case ICON_CLOUDY:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDY);
    break;
    
    case ICON_FOG:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOG);
    break;
    
    case ICON_PARTLYCLOUDY:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PARTLYCLOUDY);
    break;
    
    case ICON_PARTLYSUNNY:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PARTLYSUNNY);
    break;
    
    case ICON_RAIN:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN);
    break;
    
    case ICON_SLEET:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SLEET);
    break;
    
    case ICON_SNOW:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SNOW);
    break;
    
    case ICON_TSTORMS:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TSTORMS);
    break;
    
    case ICON_SUNRISE:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUNRISE);
    break;
    
    case ICON_SUNSET:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUNSET);
    break;
    
    default:
    g_icon_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEAR_DAY);
  }
  
  // Modify image by determining whether color is supported
  GColor* palette = gbitmap_get_palette(g_icon_image);
  
  #if defined(PBL_COLOR)
  palette[0] = persist_settings.color_weather_stroke; // Foreground color
  palette[1] = GColorClear; // Background color
  #elif defined(PBL_BW)
  palette[0] = GColorBlack; // Foreground color
  palette[1] = GColorWhite; // Background color
  #endif

  bitmap_layer_set_bitmap(g_icon_block,g_icon_image); // Automatically marks dirty
}


// Window handler for loading
void main_window_load(Window* window){
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create batteyr block
  {
    g_battery_block = layer_create(g_battery_block_rect);
    layer_set_update_proc(g_battery_block, battery_update_proc); // Only works with stock layer
    layer_add_child(window_layer,g_battery_block);
  }
  
  // Create time block
  {
    g_time_block = text_layer_create(g_time_block_rect);
    text_layer_set_background_color(g_time_block,persist_settings.color_time_bg);
    text_layer_set_text_color(g_time_block,persist_settings.color_time_stroke);
    text_layer_set_text(g_time_block,persist_settings.time);
    text_layer_set_text_alignment(g_time_block, GTextAlignmentCenter);
    text_layer_set_font(g_time_block, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    layer_add_child(window_layer,text_layer_get_layer(g_time_block));
  }
  
  // Create date block
  {
    g_date_block = text_layer_create(g_date_block_rect);
    text_layer_set_background_color(g_date_block,persist_settings.color_time_bg);
    text_layer_set_text_color(g_date_block,persist_settings.color_time_stroke);
    text_layer_set_text(g_date_block,persist_settings.date);
    text_layer_set_text_alignment(g_date_block, GTextAlignmentCenter);
    text_layer_set_font(g_date_block, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    layer_add_child(window_layer,text_layer_get_layer(g_date_block));
  }
  
  // Create conditions block
  {
    g_cond_block = text_layer_create(g_cond_block_rect);
    text_layer_set_background_color(g_cond_block,GColorClear);
    text_layer_set_text_color(g_cond_block,persist_settings.color_weather_stroke);
    text_layer_set_text(g_cond_block,persist_settings.conditions);
    text_layer_set_text_alignment(g_cond_block, GTextAlignmentCenter);
    text_layer_set_font(g_cond_block, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    layer_add_child(window_layer,text_layer_get_layer(g_cond_block));
  }
  
  // Create icon block
  {
    // Create the layer
    g_icon_block = bitmap_layer_create(g_icon_block_rect);
    bitmap_layer_set_compositing_mode(g_icon_block,GCompOpSet); // For transperency in Basalt
    bitmap_layer_set_alignment(g_icon_block, GAlignCenter);
    
    update_icon(persist_settings.icon_number); // Create the icon
    layer_add_child(window_layer,bitmap_layer_get_layer(g_icon_block));
  }
  
  // Create tmeperature block
  {
    g_temp_block = text_layer_create(g_temp_block_rect);
    text_layer_set_background_color(g_temp_block,GColorClear);
    text_layer_set_text_color(g_temp_block,persist_settings.color_weather_stroke);
    text_layer_set_text(g_temp_block,persist_settings.temperature);
    text_layer_set_text_alignment(g_temp_block, GTextAlignmentCenter);
    text_layer_set_font(g_temp_block, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer,text_layer_get_layer(g_temp_block));
  }
}

// Window handler for unloading
void main_window_unload(Window *window){
  
  // Destroy batteyr block
  layer_destroy(g_battery_block);
  
  // Destroy time block
  text_layer_destroy(g_time_block);
  
  // Destroy date block
  text_layer_destroy(g_date_block);
  
  // Destroy conditions blocl
  text_layer_destroy(g_cond_block);
  
  // Destroy icon block
  gbitmap_destroy(g_icon_image); // Destroy icon block
  bitmap_layer_destroy(g_icon_block); // Destroy PDC image

  // destroy temperature block
  text_layer_destroy(g_temp_block);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  // Read tuples for data
  // They are prefixed with "MESSAGE_KEY_" and automatically included in pebble.h
  
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  if(temp_tuple ){
    snprintf(persist_settings.temperature, sizeof(persist_settings.temperature), "%d°", (int)temp_tuple->value->int32);
    text_layer_set_text(g_temp_block,persist_settings.temperature);
  }
  
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  if(conditions_tuple){
    snprintf(persist_settings.conditions, sizeof(persist_settings.conditions), "%s",conditions_tuple->value->cstring);
    text_layer_set_text(g_cond_block,persist_settings.conditions);
  }
  
  Tuple *icon_tuple = dict_find(iterator, MESSAGE_KEY_ICON_NUMBER); // Get the icon name
  if(icon_tuple){
    if(persist_settings.icon_number !=  (int)icon_tuple->value->int32){
      persist_settings.icon_number =  (int)icon_tuple->value->int32; // Update the global current icon number
      update_icon(persist_settings.icon_number);
    }
  }
  
  Tuple *show_conditions_tuple = dict_find(iterator, MESSAGE_KEY_SHOW_CONDITIONS);
  if(show_conditions_tuple){
    persist_settings.show_conditions = show_conditions_tuple->value->int32==1; // get bool value
    layer_set_hidden(text_layer_get_layer(g_cond_block),!persist_settings.show_conditions); // Hide or show conditions layer
  }
  
  Tuple *vibrate_disconnect_tuple = dict_find(iterator, MESSAGE_KEY_VIBRATE_DISCONNECT);
  if(show_conditions_tuple){
    persist_settings.vibrate_disconnect = vibrate_disconnect_tuple->value->int32==1; // get bool value
  }
  
  Tuple *update_interval_tuple = dict_find(iterator, MESSAGE_KEY_UPDATE_INTERVAL);
  if(update_interval_tuple){
    persist_settings.update_interval = atoi(update_interval_tuple->value->cstring); // Convert string to int
  }
  
  Tuple *color_time_bg_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_TIME_BG);
  if(color_time_bg_tuple) {
    persist_settings.color_time_bg = GColorFromHEX(color_time_bg_tuple->value->int32);
    text_layer_set_background_color(g_time_block,persist_settings.color_time_bg);
    text_layer_set_background_color(g_date_block,persist_settings.color_time_bg);
  }
  
  Tuple *color_time_stroke_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_TIME_STROKE);
  if(color_time_stroke_tuple) {
    persist_settings.color_time_stroke = GColorFromHEX(color_time_stroke_tuple->value->int32);
    text_layer_set_text_color(g_time_block,persist_settings.color_time_stroke);
    text_layer_set_text_color(g_date_block,persist_settings.color_time_stroke);
  }
  
  Tuple *color_weather_bg_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_WEATHER_BG);
  if(color_weather_bg_tuple) {
    persist_settings.color_weather_bg = GColorFromHEX(color_weather_bg_tuple->value->int32);
    window_set_background_color(g_window,persist_settings.color_weather_bg); // Set background color
  }
  
  Tuple *color_weather_stroke_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_WEATHER_STROKE);
  if(color_weather_stroke_tuple) {
    persist_settings.color_weather_stroke = GColorFromHEX(color_weather_stroke_tuple->value->int32);
    text_layer_set_text_color(g_temp_block,persist_settings.color_weather_stroke);
    text_layer_set_text_color(g_cond_block,persist_settings.color_weather_stroke);
    update_icon(persist_settings.icon_number);
  }
  
  Tuple *color_battery_bar_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_BATTERY_BAR);
  if(color_battery_bar_tuple) {
    persist_settings.color_battery_bar = GColorFromHEX(color_battery_bar_tuple->value->int32);
    layer_mark_dirty(g_battery_block); // Set layer to be redrawn
  }
  
  // Date format
  Tuple *date_format_tuple = dict_find(iterator, MESSAGE_KEY_DATE_FORMAT);
  if(date_format_tuple) {
    snprintf(persist_settings.date_format, sizeof(persist_settings.date_format), "%s", date_format_tuple->value->cstring);
    update_time_date();
  }
  
  // Save all clay settings
  persist_write_data(MESSAGE_KEY_SETTINGS, &persist_settings, sizeof(persist_settings));
}

// Other callback functions
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


void init(){
  
  g_window = window_create(); // Create main window element and assign to global
  window_set_background_color(g_window,persist_settings.color_weather_bg); // Set background color
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(g_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the Window on the watch, with animated=true
  window_stack_push(g_window, true);
  
  // Register app message callback function
  app_message_register_inbox_received(inbox_received_callback);
  
  // Register other callbacks
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Register the minute_callback function 
  tick_timer_service_subscribe(MINUTE_UNIT, minute_callback);
  
  // Ensure that the date and time are displayed from the start
  minute_callback(g_time,MINUTE_UNIT);
  
  // Register the battery_callback function
  battery_state_service_subscribe(battery_callback);
  
  // Ensure that the battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register the bluetooth_callback function
  connection_service_subscribe((ConnectionHandlers) {
  .pebble_app_connection_handler = bluetooth_callback});
  
  // Show the currect state of the BT conneciton form the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
}

void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(g_window); // Destroy Window
}

int main(void){
  init_globals();
  init();
  app_event_loop();
  deinit();
}