#include <pebble.h>

// Uncomment for demo mode
// #define DEMO_MODE

#define UI_COLOR 0x55ff55
#define NUM_NOTCHES 5
#define NUM_WEATHER_ICONS 6 
#define NUM_DAY_ICONS 7
#define MAX_CHARS 40
#define SETTINGS_KEY 1

#define Y_OFFSET (PBL_DISPLAY_HEIGHT - 180) / 2
#define X_OFFSET (PBL_DISPLAY_WIDTH - 180) / 2

static Window *s_main_window;

static TextLayer *s_hour_layer, *s_minute_layer, *s_colon_layer,
  *s_date_layer, *s_day_layer;
static BitmapLayer *s_bt_icon_layer, *s_weather_icon_layer,
  *s_ouroboros_layer, *s_day_icon_layer;
static Layer *s_battery_layer, *s_temperature_layer;

static GFont s_time_font, s_date_font, s_day_font;
static GBitmap *s_bt_icon_conn_bitmap, *s_bt_icon_disc_bitmap, 
  *s_weather_icon_bitmap, *s_ouroboros_bitmap, *s_day_icon_bitmap;

static uint8_t s_battery_level;
static bool s_charging, s_date_set, s_weather_loaded;
static int s_weather_minutes_elapsed;

typedef enum weather {
  SUNNY,
  PARTLYCLOUDY,
  CLOUDY,
  RAINY,
  SNOWY,
  STORMY
} Weather;

// Define settings struct
typedef struct ClaySettings {
  int TEMPERATURE;                   // Current temperature
  Weather CONDITIONS;                // Current weather conditions
  bool UseCurrentLocation;           // use GPS for weather?
  int WeatherCheckRate;              // how often to check weather
  char Latitude[MAX_CHARS];          // latitude when not using GPS
  char Longitude[MAX_CHARS];         // longitude when not using GPS
  char OpenWeatherAPIKey[MAX_CHARS]; // API key for open weather
  bool AmericanDate;                 // use American date format (Jan 01)?
  bool VibrateOnDisc;                // vibrate on bluetooth disconnect?
  // Colors
  GColor ColorBackground;
  GColor ColorOuroboros;
  GColor ColorWeatherIcon;
  GColor ColorDay;
  GColor ColorDayIcon;
  GColor ColorBatteryBackground;
  GColor ColorBatteryOk;
  GColor ColorBatteryLow;
  GColor ColorBatteryCharging;
  GColor ColorTime;
  GColor ColorDate;
  GColor ColorWeatherHot;
  GColor ColorWeatherVeryHot;
  GColor ColorWeatherCold;
  GColor ColorWeatherVeryCold;
  GColor ColorWeatherBackground;
  GColor ColorWeatherNeedle;
  GColor ColorBluetoothConnected;
  GColor ColorBluetoothDisconnected;
} ClaySettings;

static ClaySettings settings;

static const uint32_t DAY_ICONS[] = { 
  RESOURCE_ID_IMAGE_SUNDAY,
  RESOURCE_ID_IMAGE_MONDAY,
  RESOURCE_ID_IMAGE_TUESDAY,
  RESOURCE_ID_IMAGE_WEDNESDAY,
  RESOURCE_ID_IMAGE_THURSDAY,
  RESOURCE_ID_IMAGE_FRIDAY,
  RESOURCE_ID_IMAGE_SATURDAY
};

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_WEATHER_SUNNY,  
  RESOURCE_ID_IMAGE_WEATHER_PARTLYCLOUDY,
  RESOURCE_ID_IMAGE_WEATHER_CLOUDY,
  RESOURCE_ID_IMAGE_WEATHER_RAINY,
  RESOURCE_ID_IMAGE_WEATHER_SNOWY,
  RESOURCE_ID_IMAGE_WEATHER_STORMY
};

#ifdef DEMO_MODE
// For manual demo
#define DEMO_BATTERY 70
#define DEMO_CHARGING false
#define DEMO_TEMPERATURE 60
#define DEMO_CONDITIONS SNOWY
#define DEMO_BLUETOOTH true
#define DEMO_DAY_ICON 4
#define DEMO_HOUR "00"
#define DEMO_MINUTE "00"
#define DEMO_DATE "May 30"
#define DEMO_DAY "Wed"

// For using the cycles below
#define DEMO_CYCLE false
#define DEMO_CYCLE_POS 3

// Demo values
static char s_demo_hours[][8] = {
  "05",
  "8",
  "14",
  "20",
  "11",
  "03",
  "1"
};

static char s_demo_minutes[][8] = {
  "33",
  "22",
  "01",
  "00",
  "59",
  "15",
  "11"
};
static char s_demo_dates[][8] = {
  "Mar 22",
  "Jul 05",
  "Dec 31",
  "Feb 18",
  "Nov 28",
  "Jan 01",
  "Aug 15"
};

static bool s_demo_bluetooth[] = {
  true,
  true,
  true,
  false,
  false,
  false,
  true
};

static uint8_t s_demo_battery[] = {
  80,
  50,
  10,
  100,
  70,
  30,
  90
};

static bool s_demo_charging[] = {
  false,
  false,
  false,
  false,
  true,
  true,
  false
};

static Weather s_demo_weather[] = {
  SUNNY,
  PARTLYCLOUDY,
  CLOUDY,
  STORMY,
  RAINY,
  SNOWY,
  PARTLYCLOUDY
};

static int s_demo_temperature[] = {
  100,
  80,
  60,
  40,
  90,
  0,
  75
};

static int s_demo_day_icons[] = {
  0,
  1,
  2,
  3,
  4,
  5,
  6
};

static char s_demo_days[][8] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};
#endif

static int TEMP_NOTCHES[] = {30, 50, 70, 85, 100};
static int TEMP_ANGLE_NOTCHES[] = {-130, -60, 0, 60, 130};

static void update_date();

// lerp between points a and b with percent c
static int lerp(int a, int b, float c){
  return (int)(a * (1.0 - c) + b * c);
}

// unlerp between points a and b with value c
static float unlerp(int a, int b, int c){
  int b_n = b - a; // normalized b
  int c_n = c - a; // normalized c
  return ((float)c_n) / ((float)(b_n));
}

// update the batter display layer
static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  s_charging = state.is_charging;

  layer_mark_dirty(s_battery_layer); // tells system to re-render at next opportunity
}

#if defined(PBL_BW)
static void dither(Layer *layer, GContext *ctx) {
  // Dither it up
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  GRect frame = layer_get_frame(layer);
  for (uint16_t y = frame.origin.y; y < frame.origin.y + frame.size.h; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
    for (uint16_t x = frame.origin.x; x < frame.origin.x + frame.size.w; x++) {
      uint8_t pixel_color = ((x + (y & 1)) & 1);
      uint16_t byte = (x >> 3); // x / 8
      uint8_t bit = x & 7; // x % 8
      uint8_t *byte_mod = &info.data[byte];
      if (!(*byte_mod & (1 << bit))) {
        continue;
      }
      *byte_mod ^= (-pixel_color ^ *byte_mod) & (1 << bit);
    }
  }
  graphics_release_frame_buffer(ctx, fb);
}
#endif

// display battery as stamina bar
static void battery_update_proc(Layer *layer, GContext *ctx) {
#ifdef DEMO_MODE
  uint8_t cur_battery = DEMO_CYCLE ? s_demo_battery[DEMO_CYCLE_POS] : DEMO_BATTERY;
  bool charging = DEMO_CYCLE ? s_demo_charging[DEMO_CYCLE_POS] : DEMO_CHARGING;
#else
  uint8_t cur_battery = s_battery_level;
  bool charging =  s_charging;
#endif

  GRect bounds = layer_get_bounds(layer);
  GColor curColor = PBL_IF_BW_ELSE(GColorWhite, settings.ColorBatteryOk); // green when battery is healthy
  if (cur_battery <= 10){ // red when running out of "stamina"
    curColor = PBL_IF_BW_ELSE(GColorWhite, settings.ColorBatteryLow);
  }
  if (charging){ // yellow when charging (values are weird when charging tho)
    curColor = PBL_IF_BW_ELSE(GColorWhite, settings.ColorBatteryCharging);
  }

  int length = (cur_battery * TRIG_MAX_ANGLE) / 100; // get percent around circle

  // fill grey background
  if (charging) {
    graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorWhite, settings.ColorBatteryBackground));
  } else {
    graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorBlack, settings.ColorBatteryBackground));
  }
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 7, 0, TRIG_MAX_ANGLE);
#if defined(PBL_BW)
  dither(layer, ctx);
#endif

  // then fill up stamina bar counterclockwise
  graphics_context_set_fill_color(ctx, curColor);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 7, TRIG_MAX_ANGLE-length, TRIG_MAX_ANGLE);

#if defined(PBL_BW)
  if (!charging && cur_battery <= 10) {
    dither(layer, ctx);
  }
#endif
}

// bluetooth status change
static void bluetooth_callback(bool connected) {
  // change sheikah sensor to represent bluetooth connection
#ifdef DEMO_MODE
  connected = (DEMO_CYCLE ? s_demo_bluetooth[DEMO_CYCLE_POS] : DEMO_BLUETOOTH);
#endif

  bitmap_layer_set_bitmap(s_bt_icon_layer, connected ? s_bt_icon_conn_bitmap : s_bt_icon_disc_bitmap);

  if (!connected) {
    if (settings.VibrateOnDisc) {
      vibes_double_pulse();
    }
  }
}

// default clay settings
static void default_settings() {  
  settings.UseCurrentLocation = true;     // use GPS for weather
  settings.WeatherCheckRate = 30;         // check every 30 mins
  strcpy(settings.Latitude, "42.36");     // MIT latitude
  strcpy(settings.Latitude, "-71.1");     // MIT longitude
  strcpy(settings.OpenWeatherAPIKey, ""); // Blank Key
  settings.TEMPERATURE = TEMP_NOTCHES[2]; // average temperature
  settings.CONDITIONS = PARTLYCLOUDY;     // average weather
  settings.AmericanDate = true;           // Jan 01 by default
  settings.VibrateOnDisc = true;          // vibrate by default
  
  // Colors
  settings.ColorBackground = GColorBlack;
  settings.ColorOuroboros = GColorScreaminGreen;
  settings.ColorDay = GColorScreaminGreen;
  settings.ColorDayIcon = GColorScreaminGreen;
  settings.ColorBatteryBackground = GColorDarkGray;
  settings.ColorBatteryOk = GColorGreen;
  settings.ColorBatteryLow = GColorRed;
  settings.ColorBatteryCharging = GColorYellow;
  settings.ColorTime = GColorScreaminGreen;
  settings.ColorDate = GColorScreaminGreen;
  settings.ColorWeatherIcon = GColorWhite;
  settings.ColorWeatherHot = GColorChromeYellow;
  settings.ColorWeatherVeryHot = GColorOrange;
  settings.ColorWeatherCold = GColorCyan;
  settings.ColorWeatherVeryCold = GColorCeleste;
  settings.ColorWeatherBackground = GColorDarkGray;
  settings.ColorWeatherNeedle = GColorWhite;
  settings.ColorBluetoothConnected = GColorElectricBlue;
  settings.ColorBluetoothDisconnected = GColorDarkGray;
}

static void refresh_colors() {
#if defined(PBL_COLOR)
  // Change colors
  GColor *palette;
  palette = gbitmap_get_palette(s_ouroboros_bitmap);
  palette[0] = settings.ColorBackground;
  palette[1] = settings.ColorOuroboros;
  
  palette = gbitmap_get_palette(s_weather_icon_bitmap);
  palette[0] = settings.ColorBackground;
  palette[1] = settings.ColorWeatherIcon;
  
  palette = gbitmap_get_palette(s_bt_icon_conn_bitmap);
  palette[0] = settings.ColorBackground;
  palette[1] = settings.ColorBluetoothConnected;
  
  palette = gbitmap_get_palette(s_bt_icon_disc_bitmap);
  palette[0] = settings.ColorBackground;
  palette[1] = settings.ColorBluetoothDisconnected;
  
  palette = gbitmap_get_palette(s_day_icon_bitmap);
  palette[0] = settings.ColorBackground;
  palette[1] = settings.ColorDayIcon;

  text_layer_set_text_color(s_day_layer, settings.ColorDay);

  text_layer_set_text_color(s_hour_layer, settings.ColorTime);
  text_layer_set_text_color(s_colon_layer, settings.ColorTime);
  text_layer_set_text_color(s_minute_layer, settings.ColorTime);
  
  text_layer_set_text_color(s_date_layer, settings.ColorDate);
#endif
}

// update display after reading from clay/weather
static void update_display() {
  // redraw the temperature
  layer_mark_dirty(s_temperature_layer);

  // update the date format
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  update_date(tick_time);

  // update the weather icon
  gbitmap_destroy(s_weather_icon_bitmap);
#ifdef DEMO_MODE
  s_weather_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[DEMO_CYCLE ? s_demo_weather[DEMO_CYCLE_POS] : DEMO_CONDITIONS]);
#else
  s_weather_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[settings.CONDITIONS]);
#endif
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);

  refresh_colors();
}

// Read settings from persistent storage
static void load_settings() {
  // Load the default settings
  default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  update_display();
}

static void request_weather() {
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result == APP_MSG_OK) {
    // tell the app whether to use current location, celsius, and also the lat and lon
    dict_write_uint8(iter, MESSAGE_KEY_UseCurrentLocation, settings.UseCurrentLocation);
    dict_write_cstring(iter, MESSAGE_KEY_Latitude, settings.Latitude);
    dict_write_cstring(iter, MESSAGE_KEY_Longitude, settings.Longitude);
    dict_write_cstring(iter, MESSAGE_KEY_OpenWeatherAPIKey, settings.OpenWeatherAPIKey);

    // Send the message
    result = app_message_outbox_send();
  }
}

// Received data! Either for weather or settings
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Use current location
  Tuple *use_current_location_t = dict_find(iterator, MESSAGE_KEY_UseCurrentLocation);
  if(use_current_location_t) {
    settings.UseCurrentLocation = use_current_location_t->value->int32 == 1;
  }

  // Weather update rate
  Tuple *weather_check_rate_t = dict_find(iterator, MESSAGE_KEY_WeatherCheckRate);
  if(weather_check_rate_t) {
    settings.WeatherCheckRate = weather_check_rate_t->value->int32;
  }

  // Open Weather API Key
  Tuple *open_weather_api_key_t = dict_find(iterator, MESSAGE_KEY_OpenWeatherAPIKey);
  if(open_weather_api_key_t) {
    strcpy(settings.OpenWeatherAPIKey,open_weather_api_key_t->value->cstring);
  }

  // Manual Latitude
  Tuple *latitude_t = dict_find(iterator, MESSAGE_KEY_Latitude);
  if(latitude_t) {
    strcpy(settings.Latitude,latitude_t->value->cstring);
  }

  // Manual Longitude
  Tuple *longitude_t = dict_find(iterator, MESSAGE_KEY_Longitude);
  if(longitude_t) {
    strcpy(settings.Longitude,longitude_t->value->cstring);
  }

  // Current temperature and weather conditions
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // if weather data is available, use it
  if (temp_tuple && conditions_tuple) {
    settings.TEMPERATURE = (int)temp_tuple->value->int32;
    settings.CONDITIONS = (Weather)conditions_tuple->value->int32;
    s_weather_loaded = true;
  } else { // we weren't given weather, so either settings were updated or we were poked. Request it now
    request_weather();
  }

  // American date format?
  Tuple *american_date_t = dict_find(iterator, MESSAGE_KEY_AmericanDate);
  if(american_date_t) {
    settings.AmericanDate = american_date_t->value->int32 == 1;
  }

  Tuple *vibrate_on_disc_t = dict_find(iterator, MESSAGE_KEY_VibrateOnDisc);
  if(vibrate_on_disc_t) {
    settings.VibrateOnDisc = vibrate_on_disc_t->value->int32 == 1;
  }

  Tuple *color_t;

  color_t = dict_find(iterator, MESSAGE_KEY_ColorBackground);
  if(color_t) {
    settings.ColorBackground = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorOuroboros);
  if(color_t) {
    settings.ColorOuroboros = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorWeatherIcon);
  if(color_t) {
    settings.ColorWeatherIcon = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorDay);
  if(color_t) {
    settings.ColorDay = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorDayIcon);
  if(color_t) {
    settings.ColorDayIcon = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorBatteryBackground);
  if(color_t) {
    settings.ColorBatteryBackground = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorBatteryOk);
  if(color_t) {
    settings.ColorBatteryOk = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorBatteryLow);
  if(color_t) {
    settings.ColorBatteryLow = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorBatteryCharging);
  if(color_t) {
    settings.ColorBatteryCharging = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorTime);
  if(color_t) {
    settings.ColorTime = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorDate);
  if(color_t) {
    settings.ColorDate = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorWeatherHot);
  if(color_t) {
    settings.ColorWeatherHot = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorWeatherVeryHot);
  if(color_t) {
    settings.ColorWeatherVeryHot = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorWeatherCold);
  if(color_t) {
    settings.ColorWeatherCold = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorWeatherVeryCold);
  if(color_t) {
    settings.ColorWeatherVeryCold = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorWeatherBackground);
  if(color_t) {
    settings.ColorWeatherBackground = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorWeatherNeedle);
  if(color_t) {
    settings.ColorWeatherNeedle = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorBluetoothConnected);
  if(color_t) {
    settings.ColorBluetoothConnected = GColorFromHEX(color_t->value->int32);
  }

  color_t = dict_find(iterator, MESSAGE_KEY_ColorBluetoothDisconnected);
  if(color_t) {
    settings.ColorBluetoothDisconnected = GColorFromHEX(color_t->value->int32);
  }

  save_settings(); // save the new settings! Current weather included
}

// Message failed to receive
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

// Message failed to send
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

// Message sent successfully
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// Get the temperature gauge angle
static int get_temp_angle(int temperature){
  if (temperature <= TEMP_NOTCHES[0]){ // colder than cold, hit max left
    return TEMP_ANGLE_NOTCHES[0];
  }
  if (temperature >= TEMP_NOTCHES[NUM_NOTCHES-1]){ // hotter than hot, hit max right
    return TEMP_ANGLE_NOTCHES[NUM_NOTCHES-1];
  }

  static uint8_t i;
  for (i = 1; i < NUM_NOTCHES; i++){
    if (temperature < TEMP_NOTCHES[i]){
      // converts temperature to angles
      return (lerp(TEMP_ANGLE_NOTCHES[i-1],TEMP_ANGLE_NOTCHES[i],unlerp(TEMP_NOTCHES[i-1],TEMP_NOTCHES[i],temperature)));
    }
  }
  return 0;
}

// update the temperature gauge
static void temperature_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

#ifdef DEMO_MODE
  int temperature = DEMO_CYCLE ? s_demo_temperature[DEMO_CYCLE_POS] : DEMO_TEMPERATURE;
#else
  int temperature = settings.TEMPERATURE;
#endif

  // clear out the meter
  graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorBlack, settings.ColorBackground));
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 10, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
#if defined(PBL_COLOR)
  if (temperature <= TEMP_NOTCHES[0]) { // turn the whole meter ice cold
    graphics_context_set_fill_color(ctx, settings.ColorWeatherVeryCold);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
  } else if (temperature >= TEMP_NOTCHES[NUM_NOTCHES-1]) { // turn the whole meter fiery hot
    graphics_context_set_fill_color(ctx, settings.ColorWeatherVeryHot);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
  } else {
    // meter background
    graphics_context_set_fill_color(ctx, settings.ColorWeatherBackground);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));

    // cold section
    graphics_context_set_fill_color(ctx, settings.ColorWeatherCold);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(-60));

    // hot section
    graphics_context_set_fill_color(ctx, settings.ColorWeatherHot);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(60), DEG_TO_TRIGANGLE(130));

    if (temperature <= TEMP_NOTCHES[1]){ // add cold trim to the meter
      graphics_context_set_fill_color(ctx, settings.ColorWeatherCold);
      if (temperature <= (TEMP_NOTCHES[1] + TEMP_NOTCHES[0]) / 2) // larger if colder
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 2, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
      else
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
    } else if (temperature >= TEMP_NOTCHES[NUM_NOTCHES-2]){ // add hot trim to the meter
      graphics_context_set_fill_color(ctx, settings.ColorWeatherHot);
      if (temperature >= (TEMP_NOTCHES[NUM_NOTCHES-2] + TEMP_NOTCHES[NUM_NOTCHES-1])/2) // larger if hotter
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 2, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
      else
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
    }
  }
#else
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(-130), DEG_TO_TRIGANGLE(130));
#endif

  // draw the ticks
  graphics_context_set_stroke_color(ctx, PBL_IF_BW_ELSE(GColorBlack, settings.ColorBackground));
  graphics_context_set_stroke_width(ctx, 1);
  static uint8_t i;
  for (i = 0; i < 9; i++){
    graphics_draw_line(ctx, center, 
    gpoint_from_polar(bounds, GOvalScaleModeFillCircle, DEG_TO_TRIGANGLE(-120+30*i)));
  }

  // draw the needle
  GRect small_bounds = grect_crop(bounds, 3);
  int temp_angle = get_temp_angle(temperature);

  graphics_context_set_stroke_color(ctx, PBL_IF_BW_ELSE(GColorBlack, settings.ColorBackground));
  graphics_context_set_stroke_width(ctx, 4);
  graphics_draw_line(ctx, center, 
                     gpoint_from_polar(small_bounds, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(temp_angle)));

  graphics_context_set_stroke_color(ctx, PBL_IF_BW_ELSE(GColorWhite, settings.ColorWeatherNeedle));
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, center, 
                     gpoint_from_polar(small_bounds, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(temp_angle)));
  graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorWhite, settings.ColorWeatherNeedle));
  graphics_fill_circle(ctx, center, 2);
}

// setup the display
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Watchface Resources
  // ouroboros
  s_ouroboros_layer = bitmap_layer_create(GRect(0 + X_OFFSET, 0 + Y_OFFSET, 180, 180));
  s_ouroboros_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_OUROBOROS);
  bitmap_layer_set_bitmap(s_ouroboros_layer, s_ouroboros_bitmap);
  bitmap_layer_set_compositing_mode(s_ouroboros_layer, GCompOpSet);
  
  // time
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BOTW_48));
  
  s_hour_layer = text_layer_create(GRect(3, 66 + Y_OFFSET, bounds.size.w / 2 - 7, 48));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, PBL_IF_BW_ELSE(GColorWhite, settings.ColorTime));
  text_layer_set_font(s_hour_layer, s_time_font);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentRight);

  s_colon_layer = text_layer_create(GRect(1, 62 + Y_OFFSET, bounds.size.w, 48));
  text_layer_set_background_color(s_colon_layer, GColorClear);
  text_layer_set_text_color(s_colon_layer, PBL_IF_BW_ELSE(GColorWhite, settings.ColorTime));
  text_layer_set_font(s_colon_layer, s_time_font);
  text_layer_set_text_alignment(s_colon_layer, GTextAlignmentCenter);

  s_minute_layer = text_layer_create(GRect(bounds.size.w / 2 + 7, 66 + Y_OFFSET, bounds.size.w / 2, 48));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, PBL_IF_BW_ELSE(GColorWhite, settings.ColorTime));
  text_layer_set_font(s_minute_layer, s_time_font);
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentLeft);

  // date
  s_date_layer = text_layer_create(GRect(0, 112 + Y_OFFSET, bounds.size.w, 28));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BOTW_28));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, PBL_IF_BW_ELSE(GColorWhite, settings.ColorDate));
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // day
  s_day_layer = text_layer_create(GRect(80 + X_OFFSET, 50 + Y_OFFSET, 40, 22));
  s_day_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BOTW_22));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, PBL_IF_BW_ELSE(GColorWhite, settings.ColorDay));
  text_layer_set_font(s_day_layer, s_day_font);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);

  // bluetooth icon
  s_bt_icon_layer = bitmap_layer_create(GRect(36 + X_OFFSET, 51 + Y_OFFSET, 25, 24));
  s_bt_icon_conn_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_CONN);
  s_bt_icon_disc_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_DISC);
  // update bluetooth icon
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);

  // battery stamina wheel
  s_battery_layer = layer_create(GRect(121 + X_OFFSET, 52 + Y_OFFSET, 22, 22));
  // assign update procedure to battery layer
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  // weather icon
  s_weather_icon_layer = bitmap_layer_create(GRect(94 + X_OFFSET, 28 + Y_OFFSET, 24, 24));
  s_weather_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[rand() % NUM_WEATHER_ICONS]);
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_weather_icon_layer, GCompOpSet);

  // temperature
  s_temperature_layer = layer_create(GRect(63 + X_OFFSET, 31 + Y_OFFSET, 22, 22));
  layer_set_update_proc(s_temperature_layer, temperature_update_proc);

  // day icon
  s_day_icon_layer = bitmap_layer_create(GRect(64 + X_OFFSET, 53 + Y_OFFSET, 14, 20));
  s_day_icon_bitmap = gbitmap_create_with_resource(DAY_ICONS[rand() % NUM_DAY_ICONS]);
  bitmap_layer_set_bitmap(s_day_icon_layer, s_day_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_day_icon_layer, GCompOpSet);

  layer_add_child(window_layer, bitmap_layer_get_layer(s_ouroboros_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_colon_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
  layer_add_child(window_layer, s_battery_layer);
  layer_add_child(window_layer, s_temperature_layer);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_weather_icon_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_icon_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_day_icon_layer));

  update_display(); // and update the display to fill in everything
}

// unload everything!
static void main_window_unload(Window *window) {
  // unload regular layers
  if (s_battery_layer != NULL)
    layer_destroy(s_battery_layer);
  if (s_temperature_layer != NULL)
    layer_destroy(s_temperature_layer);

  // unload text layers
  if (s_hour_layer != NULL)
    text_layer_destroy(s_hour_layer);
  if (s_colon_layer != NULL)
    text_layer_destroy(s_colon_layer);
  if (s_minute_layer != NULL)
    text_layer_destroy(s_minute_layer);
  if (s_date_layer != NULL)
    text_layer_destroy(s_date_layer);
  if (s_day_layer != NULL)
    text_layer_destroy(s_day_layer);

  // unload custom fonts
  if (s_time_font != NULL)
    fonts_unload_custom_font(s_time_font);
  if (s_date_font != NULL)
    fonts_unload_custom_font(s_date_font);
  if (s_day_font != NULL)
    fonts_unload_custom_font(s_day_font);

  // unload bitmap layers
  if (s_ouroboros_layer != NULL)
    bitmap_layer_destroy(s_ouroboros_layer);
  if (s_bt_icon_layer != NULL)
    bitmap_layer_destroy(s_bt_icon_layer);
  if (s_weather_icon_layer != NULL)
    bitmap_layer_destroy(s_weather_icon_layer);
  if (s_day_icon_layer != NULL)
    bitmap_layer_destroy(s_day_icon_layer);

  // unload gbitmaps
  if (s_ouroboros_bitmap != NULL)
    gbitmap_destroy(s_ouroboros_bitmap);
  if (s_bt_icon_conn_bitmap != NULL)
    gbitmap_destroy(s_bt_icon_conn_bitmap);
  if (s_bt_icon_disc_bitmap != NULL)
    gbitmap_destroy(s_bt_icon_disc_bitmap);
  if (s_weather_icon_bitmap != NULL)
    gbitmap_destroy(s_weather_icon_bitmap);
  if (s_day_icon_bitmap != NULL)
    gbitmap_destroy(s_day_icon_bitmap);
}

// update the time display
static void update_time() {
#ifdef DEMO_MODE
  text_layer_set_text(s_hour_layer, DEMO_CYCLE ? s_demo_hours[DEMO_CYCLE_POS] : DEMO_HOUR);
  text_layer_set_text(s_colon_layer, ":");
  text_layer_set_text(s_minute_layer, DEMO_CYCLE ? s_demo_minutes[DEMO_CYCLE_POS] : DEMO_MINUTE);
#else
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // put hours and minutes into buffer
  static char s_hour_buffer[8];
  strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ?
                                        "%H" : "%I", tick_time);
  text_layer_set_text(s_hour_layer, s_hour_buffer); 
  
  text_layer_set_text(s_colon_layer, ":");

  static char s_minute_buffer[8];
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
  text_layer_set_text(s_minute_layer, s_minute_buffer);  
#endif
}

static void update_date(struct tm *tick_time){
#ifdef DEMO_MODE
  if (DEMO_CYCLE) {
    text_layer_set_text(s_date_layer, s_demo_dates[DEMO_CYCLE_POS]);
    text_layer_set_text(s_day_layer, s_demo_days[DEMO_CYCLE_POS]);
    
    gbitmap_destroy(s_day_icon_bitmap);
    s_day_icon_bitmap = gbitmap_create_with_resource(DAY_ICONS[s_demo_day_icons[DEMO_CYCLE_POS]]);
    bitmap_layer_set_bitmap(s_day_icon_layer, s_day_icon_bitmap);
    
  } else {
    text_layer_set_text(s_date_layer, DEMO_DATE);
    text_layer_set_text(s_day_layer, DEMO_DAY);
    
    gbitmap_destroy(s_day_icon_bitmap);
    s_day_icon_bitmap = gbitmap_create_with_resource(DAY_ICONS[DEMO_DAY_ICON]);
    bitmap_layer_set_bitmap(s_day_icon_layer, s_day_icon_bitmap);
  }
#else
  static char s_date_buffer[8];
  if (settings.AmericanDate) {
    strftime(s_date_buffer, sizeof(s_date_buffer), "%b %d", tick_time); // displayed as "Jan 01"
  } else {
    strftime(s_date_buffer, sizeof(s_date_buffer), "%d %b", tick_time); // displayed as "01 Jan"
  }

  text_layer_set_text(s_date_layer, s_date_buffer);

  static char s_day_buffer[8];
  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", tick_time);
  text_layer_set_text(s_day_layer, s_day_buffer);

  gbitmap_destroy(s_day_icon_bitmap);
  s_day_icon_bitmap = gbitmap_create_with_resource(DAY_ICONS[tick_time->tm_wday]);
  bitmap_layer_set_bitmap(s_day_icon_layer, s_day_icon_bitmap);
#endif

  refresh_colors();
}

// this fires every minute
static void tick_handler(struct tm *tick_time, TimeUnits units_changes) {
  update_time(); // display the time

  // update the weather
  s_weather_minutes_elapsed++;
  if (s_weather_minutes_elapsed >= settings.WeatherCheckRate) { // time to check the weather
    // get the weather
    request_weather();

    s_weather_minutes_elapsed = 0;
  } 

  // update date on first call, or at midnight (00:00)
  if (!s_date_set || (tick_time->tm_min == 0 && tick_time->tm_hour == 0)) {
    update_date(tick_time);
    // if this is the last to load, then animate!
    s_date_set = true;
  }
}

// classic init, you know we need it
static void init() {
  load_settings();

  // setup window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_background_color(s_main_window, PBL_IF_BW_ELSE(GColorBlack, settings.ColorBackground));

  window_stack_push(s_main_window, true);

  // set up tick_handler to run every minute
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // want to display time and date at the start
  update_time();
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  update_date(tick_time);

  // callback for battery level updates
  battery_state_service_subscribe(battery_callback);
  // display battery at the start
  battery_callback(battery_state_service_peek());

  // callback for bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // Register callbacks for settings/weather updates
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 1024; // maaaaybe overkill, but 128 isn't enough
  const int outbox_size = 1024;
  app_message_open(inbox_size, outbox_size);
}

// classic deinit, you know we need this too
static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
