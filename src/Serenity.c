#include "Serenity.h"
#include "pebble.h"

static Window *s_window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer, *s_bt_layer;
static TextLayer *s_date_label, *s_count_label, *s_battery_label, *s_weather_label, *s_loc_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;
static GPath *s_triangle,*s_bt_path;

static GColor s_background_color,s_forground_color;

static char s_num_buffer[4], s_day_buffer[6], s_count_buffer[14], s_date_buffer[10],s_battery_buffer[5], s_weather_buffer[20], s_location_buffer[15];

static struct tm then;
static int s_current_temp;
static char s_current_conditions[15];

static void bluetooth_callback(bool connected) {
    // Show icon if disconnected
    layer_set_hidden(s_bt_layer,connected);
 
    // Issue a vibrating alert
    if (!connected) {
        vibes_double_pulse();
    }
}

static void update_text_layers() {
    /*just cause I can't think of a better way to do this*/
    text_layer_set_background_color(s_date_label, s_background_color);
    text_layer_set_text_color(s_date_label, s_forground_color);
    layer_set_hidden(text_layer_get_layer(s_date_label),(global_config.showdate == 0));
    
    text_layer_set_background_color(s_loc_label, s_background_color);
    text_layer_set_text_color(s_loc_label, s_forground_color);
    if (global_config.showweather == 1) {
        layer_set_hidden(text_layer_get_layer(s_loc_label),(global_config.showlocation == 0));
    } else {
        layer_set_hidden(text_layer_get_layer(s_loc_label),true);
    }

    text_layer_set_background_color(s_count_label, s_background_color);
    text_layer_set_text_color(s_count_label, s_forground_color);
    layer_set_hidden(text_layer_get_layer(s_count_label),(global_config.countformat == FMT_BLANK));

    
    text_layer_set_background_color(s_battery_label,s_background_color);
    text_layer_set_text_color(s_battery_label, s_forground_color);
    layer_set_hidden(text_layer_get_layer(s_battery_label),(global_config.battery == 0));
   
    text_layer_set_background_color(s_weather_label,s_background_color);
    text_layer_set_text_color(s_weather_label, s_forground_color);
    layer_set_hidden(text_layer_get_layer(s_weather_label),(global_config.showweather == 0));
}
    

static void bg_update_proc(Layer *layer, GContext *ctx) {
    int i;
    const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
    const int y_offset = PBL_IF_ROUND_ELSE(6, 0);

    s_background_color = ((global_config.white == 0) ? GColorBlack : GColorWhite);
    s_forground_color = ((global_config.white == 0) ? GColorWhite : GColorBlack);

    graphics_context_set_fill_color(ctx, s_background_color);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
    
    if (global_config.showtriangle == 1) {
        graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorBlueMoon,s_forground_color));
        graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(GColorBlueMoon,s_forground_color));
        gpath_draw_outline(ctx, s_triangle);
    }

    graphics_context_set_fill_color(ctx, s_forground_color);
    for (i = 0; i < NUM_CLOCK_TICKS_WHITE; ++i) {
        gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
        gpath_draw_filled(ctx, s_tick_paths[i]);
    }
    
    graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorRed,s_forground_color));
    for (; i < NUM_CLOCK_TICKS_RED; ++i) {
        gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
        gpath_draw_filled(ctx, s_tick_paths[i]);
    }
}

static void bt_update_proc(Layer *layer, GContext *ctx) {
    //    layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
        graphics_context_set_stroke_width(ctx,3);
        graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorRed,s_forground_color));
        graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(GColorRed,s_forground_color));
        gpath_draw_outline_open(ctx, s_bt_path);
//    bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, bounds.size.w / 2);

    s_background_color = ((global_config.white == 0) ? GColorBlack : GColorWhite);
    s_forground_color = ((global_config.white == 0) ? GColorWhite : GColorBlack);
    update_text_layers();
 
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
    GPoint second_hand = {
        .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
    };

  // second hand
    if (global_config.showseconds == 1) {
        graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(GColorRed,s_forground_color));
        graphics_draw_line(ctx, second_hand, center);
    }

  // minute/hour hand
    graphics_context_set_fill_color(ctx, s_forground_color);
    graphics_context_set_stroke_color(ctx, s_background_color);

    gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
    gpath_draw_filled(ctx, s_minute_arrow);
    gpath_draw_outline(ctx, s_minute_arrow);

    gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
    gpath_draw_filled(ctx, s_hour_arrow);
    gpath_draw_outline(ctx, s_hour_arrow);

    // dot in the middle
    graphics_context_set_fill_color(ctx, s_background_color);
    graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static int day_number (int y,int m,int d) {
    m = (m + 9) % 12;
    y = y - m/10;
    return (365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + ( d - 1 ));
}

static void update_counter (struct tm *now_secs) {
    // Countdown update
    struct tm *now;
    int days_now, days_counter;
    
//    time_t seconds_now;
    int difference = 1;
    
    if (now_secs == NULL) {
        time_t t = time(NULL);
        now = localtime(&t);
    } else {
        now = now_secs;
    }
    
    days_now = day_number(now->tm_year+1900,now->tm_mon+1,now->tm_mday);
    days_counter = day_number(then.tm_year+1900,then.tm_mon+1,then.tm_mday);
  
    switch (global_config.countformat) {
        case FMT_DAYS :
            difference = days_now - days_counter;
            if (difference < 0) {
                difference = -(difference);
            }
            snprintf (s_count_buffer,sizeof(s_count_buffer),"%d Days",difference);
            break;
        case FMT_ZEN :
            snprintf (s_count_buffer,sizeof(s_count_buffer),"%d hrs,%d mins",now->tm_hour,now->tm_min);
            break;
        case FMT_BLANK :
            s_count_buffer[0] = '\0';
            break;
        default:
            snprintf (s_count_buffer,sizeof(s_count_buffer),"%d Broken",difference);
            
    }
    text_layer_set_text(s_count_label, s_count_buffer);
}
    
static void date_update_proc(Layer *layer, GContext *ctx) {
    time_t t = time(NULL);
    struct tm *now = localtime(&t);

    s_background_color = ((global_config.white == 0) ? GColorBlack : GColorWhite);
    s_forground_color = ((global_config.white == 0) ? GColorWhite : GColorBlack);
    update_text_layers();
    
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d", now);
    text_layer_set_text(s_date_label, s_date_buffer);
  
    update_counter(now);

    if (global_config.battery == 1) {
        BatteryChargeState charge_state = battery_state_service_peek();
        if (charge_state.is_charging) {
            snprintf(s_battery_buffer, sizeof(s_battery_buffer), "C");
            text_layer_set_text_color(s_battery_label, COLOR_FALLBACK(GColorRed,s_forground_color));
        } else {
            if (charge_state.charge_percent<25) {
                text_layer_set_text_color(s_battery_label, COLOR_FALLBACK(GColorRed,s_forground_color));
            } else {
                text_layer_set_text_color(s_battery_label, s_forground_color);
            }
          snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
        }
    }
    
    update_text_layers();
}

static void update_weather(struct tm *tick_time) {
    DictionaryIterator *iter;

    // Get weather update every 30 minutes
    if (tick_time == NULL) {
        // forced weather update
#ifdef DO_DEBUG_LOGS
        APP_LOG (APP_LOG_LEVEL_DEBUG,"Polling Checked: time to poll =time_to_poll: %d - poll_once: %d",s_time_to_poll,s_weather_updated);
#endif
        app_message_outbox_begin(&iter);
        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);
        // Send the message!
        app_message_outbox_send();
    } else {
        if((tick_time->tm_min % WEATHER_POLL_DIV) == 0) {
            if (s_weather_updated == 0) {
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"Polling Weather min: %d div: %d poll_once: %d time_to_poll: %d",tick_time->tm_min,WEATHER_POLL_DIV,s_weather_updated,s_time_to_poll);
#endif
                if (s_time_to_poll == 0) {
                    // Begin dictionary
                    s_time_to_poll = global_config.weatherpoll;
#ifdef DO_DEBUG_LOGS
                    APP_LOG (APP_LOG_LEVEL_DEBUG,"Polling Checked: time to poll =time_to_poll: %d - poll_once: %d",s_time_to_poll,s_weather_updated);
#endif
                    app_message_outbox_begin(&iter);

                    // Add a key-value pair
                    dict_write_uint8(iter, 0, 0);

                    // Send the message!
                    app_message_outbox_send();
                    
                }
                s_time_to_poll = s_time_to_poll - WEATHER_POLL_DIV;
                s_weather_updated = 1;            
            }
        } else {
            s_weather_updated = 0;
        }
    }
}
static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
    
    if (global_config.showweather == 1)
        update_weather(tick_time);

    layer_mark_dirty(window_get_root_layer(s_window));
}

/* App Message Handelers */
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *t = dict_read_first(iter);

    while (t != NULL) {
        switch (t->key) {
//             Weather Messsage
            case KEY_TEMPERATURE :           
                s_current_temp = t->value->int8;
                if (global_config.showfahrenheit == 1)
                    s_current_temp = ((s_current_temp * 9) / 5) + 32;
                snprintf (s_weather_buffer,sizeof(s_weather_buffer),"%d%s, %s",s_current_temp,((global_config.showfahrenheit == 1)?"F":"C"), s_current_conditions);
#ifdef DO_DEBUG_LOGS
                APP_LOG(APP_LOG_LEVEL_DEBUG,"Temerature Message: %d%s, %s",t->value->int8,((global_config.showfahrenheit == 1)?"F":"C"), s_current_conditions);
#endif
                break;
            case KEY_CONDITIONS :
                snprintf (s_current_conditions,sizeof(s_current_conditions),"%s", t->value->cstring);
                snprintf (s_weather_buffer,sizeof(s_weather_buffer),"%d%s, %s",s_current_temp,((global_config.showfahrenheit == 1)?"F":"C"), s_current_conditions);
                
#ifdef DO_DEBUG_LOGS
               APP_LOG(APP_LOG_LEVEL_DEBUG,"Condition Message: %d%s, %s",s_current_temp,((global_config.showfahrenheit == 1)?"F":"C"), s_current_conditions);
#endif
                break;
            case KEY_LOCATION :
                snprintf (s_location_buffer,sizeof(s_location_buffer),"%s", t->value->cstring);
#ifdef DO_DEBUG_LOGS
               APP_LOG(APP_LOG_LEVEL_DEBUG,"Location: %s", s_location_buffer);
#endif
                break;
                
//           Configeration Message
            case KEY_YEAR :
                global_config.year = t->value->int32;
                break;
            case KEY_DAY :
                global_config.day = t->value->int8;
                break;
            case KEY_MONTH :
                global_config.month = t->value->int8;
                break;
            case KEY_SHOWSECONDS :
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Seconds CHanged %d",t->value->int8);
#endif
                global_config.showseconds = t->value->int8;
                tick_timer_service_unsubscribe();
                tick_timer_service_subscribe(((global_config.showseconds == 1) ? SECOND_UNIT : MINUTE_UNIT), handle_second_tick);
                break;
            case KEY_SHOWTRIANGLE :
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Triangle Changed %d",t->value->int8);
#endif
                global_config.showtriangle = t->value->int8;
                break;
            case KEY_FORMAT :
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Format changed %d",(int)t->value->int32);
#endif
                global_config.countformat = t->value->int32;
                break;
            case KEY_BLACK :
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Black Changed %d",t->value->int8);
#endif
                global_config.white = t->value->int8;
                s_background_color = ((global_config.white == 0) ? GColorBlack : GColorWhite);
                s_forground_color = ((global_config.white == 0) ? GColorWhite : GColorBlack);
                break;
            case KEY_BATTERY : 
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Battery Changed %d",t->value->int8);
#endif
                global_config.battery = t->value->int8;
                layer_set_hidden(text_layer_get_layer(s_battery_label),(global_config.battery == 0));
                break;
            case KEY_WEATHER :
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Weather Changed %d",t->value->int8);
#endif
                global_config.showweather = t->value->int8;
                layer_set_hidden(text_layer_get_layer(s_weather_label),(global_config.showweather == 0));
                update_weather(NULL);
                break;
            case KEY_FAHRENHEIT :
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Farheneit Changed %d",t->value->int8);
#endif
                global_config.showfahrenheit = t->value->int8;
                update_weather (NULL);
                break;
            case KEY_WEATHERPOLL:
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: Update Changed %d",t->value->int8);
#endif
                global_config.weatherpoll = t->value->int8;
                s_time_to_poll = 0;
                break;
            case KEY_SHOWDATE:
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: ShowDate Changed %d",t->value->int8);
#endif
                global_config.showdate = t->value->int8;
                break;
            case KEY_SHOWLOCATION:
#ifdef DO_DEBUG_LOGS
                APP_LOG (APP_LOG_LEVEL_DEBUG,"INFO: ShowLocation Changed %d",t->value->int8);
#endif
                global_config.showlocation = t->value->int8;
        }
        t = dict_read_next(iter);
    }

  
#ifdef DO_DEBUG_LOGS
    APP_LOG (APP_LOG_LEVEL_DEBUG,"AppMessage : year - %d, month - %d, - day %d", (int)global_config.year, global_config.month, global_config.day);
    APP_LOG (APP_LOG_LEVEL_DEBUG, "Seconds %d, format %d, triangle %d, battery %d, bluetooth %d, white %d, weather %d, f %d,p %d, date %d, loc %d",
        global_config.showseconds,
        (int)global_config.countformat,
        global_config.showtriangle,
        global_config.battery,
        global_config.bluetooth,
        global_config.white,
        global_config.showweather,
        global_config.showfahrenheit,
        global_config.weatherpoll,
        global_config.showdate,
        global_config.showlocation
        );  
#endif
    
    then.tm_year = global_config.year - 1900;
    then.tm_mon = global_config.month -1;
    then.tm_mday = global_config.day;
    update_counter (NULL);
    update_text_layers();
}


static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    GPoint centre = grect_center_point(&bounds);

    s_background_color = ((global_config.white == 0) ? GColorBlack : GColorWhite);
    s_forground_color = ((global_config.white == 0) ? GColorWhite : GColorBlack);
    
    s_simple_bg_layer = layer_create(bounds);
    layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
    layer_add_child(window_layer, s_simple_bg_layer);
    
    s_bt_layer = layer_create(GRect(((bounds.size.w*13)/18)-10, centre.y-10, 21, 21));
//    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_set_update_proc(s_bt_layer, bt_update_proc);
    layer_add_child(window_layer, s_bt_layer);

    s_date_layer = layer_create(bounds);
    layer_set_update_proc(s_date_layer, date_update_proc);
    layer_add_child(window_layer, s_date_layer);

    s_date_label = text_layer_create(GRect(centre.x-50, ((bounds.size.h * 3 )/18)-5, 101, 21));
    text_layer_set_text_alignment(s_date_label,GTextAlignmentCenter);
    text_layer_set_text(s_date_label, s_day_buffer);
    text_layer_set_background_color(s_date_label, s_background_color);
    text_layer_set_text_color(s_date_label, s_forground_color);
    text_layer_set_font(s_date_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(s_date_layer, text_layer_get_layer(s_date_label));
    layer_set_hidden(text_layer_get_layer(s_date_label),(global_config.showdate == 0));

    s_loc_label = text_layer_create(GRect(centre.x-50, ((bounds.size.h * 5 )/18)-4, 101, 21));
    text_layer_set_text_alignment(s_loc_label,GTextAlignmentCenter);
    text_layer_set_text(s_loc_label, s_location_buffer);
    text_layer_set_background_color(s_loc_label, s_background_color);
    text_layer_set_text_color(s_loc_label, s_forground_color);
    text_layer_set_font(s_loc_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(s_date_layer, text_layer_get_layer(s_loc_label));
    layer_set_hidden(text_layer_get_layer(s_loc_label),(global_config.showlocation == 0));

    s_count_label = text_layer_create(GRect(centre.x-50, ((bounds.size.h * 11 )/ 18)-1, 101, 21));
    text_layer_set_text_alignment(s_count_label,GTextAlignmentCenter);
    text_layer_set_text(s_count_label, s_count_buffer);
    text_layer_set_background_color(s_count_label, s_background_color);
    text_layer_set_text_color(s_count_label, s_forground_color);
    text_layer_set_font(s_count_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(s_date_layer, text_layer_get_layer(s_count_label));
    layer_set_hidden(text_layer_get_layer(s_count_label),(global_config.countformat == FMT_BLANK));
    
    s_weather_label = text_layer_create(GRect(centre.x-50, ((bounds.size.h * 13 )/ 18)+1, 101, 21));
    text_layer_set_text_alignment(s_weather_label,GTextAlignmentCenter);
    text_layer_set_text(s_weather_label, s_weather_buffer);
    text_layer_set_background_color(s_weather_label, s_background_color);
    text_layer_set_text_color(s_weather_label, s_forground_color);
    text_layer_set_font(s_weather_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(s_date_layer, text_layer_get_layer(s_weather_label));
    layer_set_hidden(text_layer_get_layer(s_weather_label),(global_config.showweather == 0));
   
    s_battery_label = text_layer_create(GRect(((bounds.size.w*5)/18)-30, centre.y-10, 61, 21));
    text_layer_set_text_alignment(s_battery_label,GTextAlignmentCenter);
    text_layer_set_text(s_battery_label, s_battery_buffer);
    text_layer_set_background_color(s_battery_label,s_background_color);
    text_layer_set_text_color(s_battery_label, s_forground_color);
    text_layer_set_font(s_battery_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(s_date_layer, text_layer_get_layer(s_battery_label));
    layer_set_hidden(text_layer_get_layer(s_battery_label),(global_config.battery == 0));
   
    s_hands_layer = layer_create(bounds);
    
//    bluetooth_callback(connection_service_peek_pebble_app_connection());

    layer_set_update_proc(s_hands_layer, hands_update_proc);
    layer_add_child(window_layer, s_hands_layer);
    
 
    // Show the correct state of the BT connection from the start
    bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void window_unload(Window *window) {

  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_date_label);
  text_layer_destroy(s_count_label);
  text_layer_destroy(s_battery_label);
  text_layer_destroy(s_weather_label);
  
//  gbitmap_destroy(s_bt_icon_bitmap);
//  bitmap_layer_destroy(s_bt_icon_layer);

  layer_destroy(s_hands_layer);

}

static void init_config() {
    int version = persist_read_int(STORAGE_VERSION_KEY);
    
    switch (version) {
        case STORAGE_VERSION :
            /* CUrrent version of config */
            persist_read_data (KEY_STRUCTURE,&global_config,sizeof(global_config));

#ifdef DO_DEBUG_LOGS
            APP_LOG (APP_LOG_LEVEL_DEBUG,"Read %d : year - %d, month - %d, - day %d",version, (int)global_config.year, global_config.month, global_config.day);
            APP_LOG (APP_LOG_LEVEL_DEBUG, "Seconds %d, format %d, triangle %d, battery %d, bluetooth %d, white %d, weather %d,f %d,p %d, date %d, loc %d",
                global_config.showseconds,
                (int)global_config.countformat,
                global_config.showtriangle,
                global_config.battery,
                global_config.bluetooth,
                global_config.white,
                global_config.showweather,
                global_config.showfahrenheit,
                global_config.weatherpoll,
                global_config.showdate,
                global_config.showlocation
            );  
#endif
            break;
        default : 
            global_config.year = 2014;
            global_config.month = 11;
            global_config.day = 8;
            global_config.showseconds = 1;
            global_config.countformat = FMT_DAYS;
            global_config.showtriangle = 1;
            global_config.battery = 0;
            global_config.bluetooth = 1;
            global_config.white = 0;
            global_config.showweather = 1;
            global_config.showfahrenheit = 0;
            global_config.weatherpoll = 60;
            global_config.showdate = 1;
            global_config.showlocation = 1;
#ifdef DO_DEBUG_LOGS
            APP_LOG (APP_LOG_LEVEL_DEBUG,"Set Old version: %d year - %d, month - %d, - day %d", version,  (int)global_config.year, global_config.month, global_config.day);
            APP_LOG (APP_LOG_LEVEL_DEBUG, "Seconds %d, format %d, triangle %d, battery %d, bluetooth %d, white %d, weather %d,f %d,p %d, date %d, loc %d",
                global_config.showseconds,
                (int)global_config.countformat,
                global_config.showtriangle,
                global_config.battery,
                global_config.bluetooth,
                global_config.white,
                global_config.showweather,
                global_config.showfahrenheit,
                global_config.weatherpoll,
                global_config.showdate,
                global_config.showlocation
            );  
#endif
    }
    
    s_time_to_poll = global_config.weatherpoll;
    
    // Setup conter time from presist
    then.tm_hour = 0;
    then.tm_min = 0;
    then.tm_sec = 0;
    then.tm_year = global_config.year - 1900;
    then.tm_mon = global_config.month -1;
    then.tm_mday = global_config.day;
   
}

static void init() {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);

    s_day_buffer[0] = '\0';
    s_num_buffer[0] = '\0';
    s_count_buffer[0] = '\0';
    s_battery_buffer[0] = '\0';
    
    init_config();
    update_weather(NULL);
    update_counter (NULL);
 
    // init hand paths
    s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
    s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

    Layer *window_layer = window_get_root_layer(s_window);
    GRect bounds = layer_get_bounds(window_layer);
    GPoint center = grect_center_point(&bounds);
    gpath_move_to(s_minute_arrow, center);
    gpath_move_to(s_hour_arrow, center);

    for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
        s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
    }

    /* init the triabgle for round or square */
    TRIANGLE_POINTS.points[0].x = bounds.size.w /2;
    TRIANGLE_POINTS.points[1].x = bounds.size.w / 9;
    TRIANGLE_POINTS.points[1].y = (bounds.size.h * 13) / 18;
    TRIANGLE_POINTS.points[2].x = (bounds.size.w * 8) / 9;
    TRIANGLE_POINTS.points[2].y = (bounds.size.h * 13) / 18;
    s_triangle = gpath_create(&TRIANGLE_POINTS);

    s_bt_path = gpath_create (&BT_POINTS);
    
    if (global_config.showseconds == 1) {
        tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
    } else {
        tick_timer_service_subscribe(MINUTE_UNIT, handle_second_tick);
    }
    
    // Register for Bluetooth connection updates
    connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = bluetooth_callback
    });

    app_message_register_inbox_received(inbox_received_handler);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback); 
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());


  //    app_message_open(64,64);
}

static void deinit() {
#ifdef DO_DEBUG_LOGS
    persist_write_int(STORAGE_VERSION_KEY,STORAGE_VERSION);
    int written = persist_write_data (KEY_STRUCTURE,&global_config,sizeof(global_config));
    APP_LOG (APP_LOG_LEVEL_DEBUG,"Wrote : %d, Size : %d",written,sizeof(global_config));
#else
    persist_write_int(STORAGE_VERSION_KEY,STORAGE_VERSION);
    persist_write_data (KEY_STRUCTURE,&global_config,sizeof(global_config));
#endif
   
    gpath_destroy(s_minute_arrow);
    gpath_destroy(s_hour_arrow);

    for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
        gpath_destroy(s_tick_paths[i]);
    }

    gpath_destroy(s_triangle);
    gpath_destroy(s_bt_path);
    tick_timer_service_unsubscribe();
    window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
