import http.server
import socketserver
import os
import json
import urllib.parse
import threading
import time
import random
import secrets

PORT = 8000
STATIC_DIR = os.path.join(os.path.dirname(__file__), 'main', 'static')

# Global mock state matching real ESP32
state = {
    # System info
    "uptime_start": time.time(),
    "free_heap": 185240,
    "min_free_heap": 172900,
    "total_heap": 284000,
    "largest_free_block": 118000,
    "cpu_freq_mhz": 240,
    "stg_type": 0,
    "http_json_send_failures": 0,
    "http_static_send_failures": 0,
    "http_static_cache_hits": 24,
    "http_static_deadline_aborts": 0,
    
    # Credentials
    "username": "admin",
    "password": "admin123",
    "auth_nonce": "",
    
    # Wifi Status
    "wifi_mode": "APSTA",
    "wifi_connected": True,
    "wifi_ssid": "MyHomeWiFi",
    "wifi_profiles": [
        {"ssid": "MyHomeWiFi", "auto": True, "connected": True}
    ],
    "wifi_auto_index": 0,
    "wifi_networks_in_air": [
        {"ssid": "MyHomeWiFi", "rssi": -62, "secure": True},
        {"ssid": "Mock_Office_5G", "rssi": -72, "secure": True},
        {"ssid": "Neighbor_WiFi_AP", "rssi": -88, "secure": False}
    ],
    
    # Hardware Map (Active and Pending)
    "hardware_map": {
        "float_input_gpio": 32,
        "pump_relay1_gpio": 26,
        "pump_relay2_gpio": 27,
        "ds18b20_gpio": 33,
        "cooling_relay_gpio": 25
    },
    "pending_hardware_map": None,
    "reboot_required": False,
    
    # Pump Runtime State
    "running": False,
    "initialized": True,
    "config_valid": True,
    "initial_stabilizing": False,
    "fault": False,
    "float_state": "off", # off = Timer 1, on = Timer 2
    "confirmed_float_state": "off",
    "pending_float_state": "off",
    "pending_since_ms": 0,
    "float_changed": False,
    "active_timer": "timer1",
    "active_relay": "relay1",
    "phase": "idle", # on, off, idle
    "countdown_sec": 0,
    "phase_deadline_ms": 0,
    "relay1_energized": False,
    "relay2_energized": False,
    "float_switch_on": False,
    
    # Pump Config Settings
    "timer1_on_sec": 20,
    "timer1_off_sec": 60,
    "timer2_on_sec": 10,
    "timer2_off_sec": 180,
    "relay1_active_low": True,
    "relay2_active_low": True,
    "timer1_start_phase": "on",
    "timer2_start_phase": "on",
    "auto_start": False,
    "debounce_ms": 100,
    
    # Cooling Runtime State
    "cooling_temp_c": 27.5,
    "cooling_relay_state": False,
    "cooling_demand": False,
    "lockout_active": False,
    "lockout_remaining_sec": 0,
    "test_remaining_sec": 0,
    "cooling_mode": "force_off", # auto, force_off, test_on
    "cooling_previous_mode": "force_off",
    "blocked_reason": "force_off",
    "cooling_auto_demand": False,
    
    # Cooling Config Settings
    "cooling_threshold_c_x10": 300,
    "cooling_hysteresis_c_x10": 10,
    "cooling_auto_enable": False,
    "cooling_test_timeout_sec": 10,
    "cooling_compressor_min_off_sec": 10,
    "cooling_relay_polarity": "active_low",
}

# Real ESP32 GPIO lists for /api/hardware/map options
GPIO_OPTIONS = {
    "float_input_gpio": [
        {"gpio": 32, "label": "GPIO32", "role": "float_input", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": True},
        {"gpio": 33, "label": "GPIO33", "role": "float_input", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 25, "label": "GPIO25", "role": "float_input", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 26, "label": "GPIO26", "role": "float_input", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 27, "label": "GPIO27", "role": "float_input", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 34, "label": "GPIO34 external pull required", "role": "float_input", "input_capable": True, "output_capable": False, "internal_pull_capable": False, "is_default": False},
        {"gpio": 35, "label": "GPIO35 external pull required", "role": "float_input", "input_capable": True, "output_capable": False, "internal_pull_capable": False, "is_default": False},
        {"gpio": 36, "label": "GPIO36 external pull required", "role": "float_input", "input_capable": True, "output_capable": False, "internal_pull_capable": False, "is_default": False},
        {"gpio": 39, "label": "GPIO39 external pull required", "role": "float_input", "input_capable": True, "output_capable": False, "internal_pull_capable": False, "is_default": False}
    ],
    "pump_relay1_gpio": [
        {"gpio": 26, "label": "GPIO26", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": True},
        {"gpio": 27, "label": "GPIO27", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 25, "label": "GPIO25", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 32, "label": "GPIO32", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 33, "label": "GPIO33", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 18, "label": "GPIO18", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 19, "label": "GPIO19", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 21, "label": "GPIO21", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 22, "label": "GPIO22", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 23, "label": "GPIO23", "role": "pump_relay_1", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False}
    ],
    "pump_relay2_gpio": [
        {"gpio": 27, "label": "GPIO27", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": True},
        {"gpio": 26, "label": "GPIO26", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 25, "label": "GPIO25", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 32, "label": "GPIO32", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 33, "label": "GPIO33", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 18, "label": "GPIO18", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 19, "label": "GPIO19", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 21, "label": "GPIO21", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 22, "label": "GPIO22", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 23, "label": "GPIO23", "role": "pump_relay_2", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False}
    ],
    "ds18b20_gpio": [
        {"gpio": 33, "label": "GPIO33", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": True},
        {"gpio": 32, "label": "GPIO32", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 25, "label": "GPIO25", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 26, "label": "GPIO26", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 27, "label": "GPIO27", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 18, "label": "GPIO18", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 19, "label": "GPIO19", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 21, "label": "GPIO21", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 22, "label": "GPIO22", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 23, "label": "GPIO23", "role": "ds18b20_data", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False}
    ],
    "cooling_relay_gpio": [
        {"gpio": 25, "label": "GPIO25", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": True},
        {"gpio": 26, "label": "GPIO26", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 27, "label": "GPIO27", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 32, "label": "GPIO32", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 33, "label": "GPIO33", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 18, "label": "GPIO18", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 19, "label": "GPIO19", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 21, "label": "GPIO21", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 22, "label": "GPIO22", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False},
        {"gpio": 23, "label": "GPIO23", "role": "cooling_relay", "input_capable": True, "output_capable": True, "internal_pull_capable": True, "is_default": False}
    ]
}

def update_pump_simulation(dt_ms):
    global state
    now_ms = int(time.time() * 1000)
    
    # Float debounce logic
    raw_float = "on" if state["float_switch_on"] else "off"
    if state["pending_float_state"] != raw_float:
        state["pending_float_state"] = raw_float
        state["pending_since_ms"] = now_ms
    elif state["confirmed_float_state"] != raw_float:
        if now_ms - state["pending_since_ms"] >= state["debounce_ms"]:
            state["confirmed_float_state"] = raw_float
            state["float_changed"] = True
            
    state["float_state"] = state["confirmed_float_state"]
    
    if not state["running"]:
        # Preview Mode
        if state["confirmed_float_state"] == "on":
            state["active_timer"] = "timer2"
            state["active_relay"] = "relay2"
            state["phase"] = state["timer2_start_phase"]
            state["countdown_sec"] = state["timer2_on_sec"] if state["phase"] == "on" else state["timer2_off_sec"]
        else:
            state["active_timer"] = "timer1"
            state["active_relay"] = "relay1"
            state["phase"] = state["timer1_start_phase"]
            state["countdown_sec"] = state["timer1_on_sec"] if state["phase"] == "on" else state["timer1_off_sec"]
        state["relay1_energized"] = False
        state["relay2_energized"] = False
        return

    # Running Mode
    if state["initial_stabilizing"]:
        state["relay1_energized"] = False
        state["relay2_energized"] = False
        if state["confirmed_float_state"] != "unknown":
            state["initial_stabilizing"] = False
            start_channel(state["confirmed_float_state"], now_ms)
        return

    if state["float_changed"]:
        state["float_changed"] = False
        start_channel(state["confirmed_float_state"], now_ms)
    else:
        # Check if phase deadline is reached
        if state["phase"] != "idle" and state["phase_deadline_ms"] <= now_ms:
            if state["phase"] == "on":
                set_phase("off", now_ms)
            else:
                set_phase("on", now_ms)

    # Update countdown_sec
    if state["phase"] != "idle" and state["phase_deadline_ms"] > now_ms:
        remaining_ms = state["phase_deadline_ms"] - now_ms
        state["countdown_sec"] = int((remaining_ms + 999) / 1000)
    else:
        state["countdown_sec"] = 0

def start_channel(float_state, now_ms):
    global state
    if float_state == "on":
        state["active_timer"] = "timer2"
        state["active_relay"] = "relay2"
    else:
        state["active_timer"] = "timer1"
        state["active_relay"] = "relay1"
        
    start_phase = state["timer2_start_phase"] if state["active_timer"] == "timer2" else state["timer1_start_phase"]
    set_phase(start_phase, now_ms)

def set_phase(phase, now_ms):
    global state
    state["phase"] = phase
    duration = 0
    if state["active_timer"] == "timer1":
        duration = state["timer1_on_sec"] if phase == "on" else state["timer1_off_sec"]
    else:
        duration = state["timer2_on_sec"] if phase == "on" else state["timer2_off_sec"]
        
    state["phase_deadline_ms"] = now_ms + (duration * 1000)
    state["countdown_sec"] = duration
    
    energized = (phase == "on")
    if state["active_relay"] == "relay1":
        state["relay1_energized"] = energized
        state["relay2_energized"] = False
    elif state["active_relay"] == "relay2":
        state["relay1_energized"] = False
        state["relay2_energized"] = energized

def update_cooling_simulation(dt_ms):
    global state
    now_ms = int(time.time() * 1000)
    
    # Lockout countdown
    if state["lockout_remaining_sec"] > 0:
        state["lockout_remaining_sec"] -= (dt_ms / 1000.0)
        if state["lockout_remaining_sec"] <= 0:
            state["lockout_remaining_sec"] = 0
    state["lockout_active"] = state["lockout_remaining_sec"] > 0
            
    # Test countdown
    if state["cooling_mode"] == "test_on":
        if not state["lockout_active"]:
            state["test_remaining_sec"] -= (dt_ms / 1000.0)
            if state["test_remaining_sec"] <= 0:
                state["test_remaining_sec"] = 0
                state["cooling_mode"] = state["cooling_previous_mode"]

    # Decision logic
    demand = False
    may_energize = False
    
    if state["cooling_mode"] == "force_off":
        state["cooling_demand"] = False
        state["cooling_auto_demand"] = False
        state["blocked_reason"] = "force_off"
    elif state["cooling_mode"] == "test_on":
        demand = True
        may_energize = True
        state["cooling_auto_demand"] = False
        state["blocked_reason"] = "none"
    else: # auto mode
        thresh = state["cooling_threshold_c_x10"] / 10.0
        hyst = state["cooling_hysteresis_c_x10"] / 10.0
        off_temp = thresh - hyst
        
        if state["cooling_auto_demand"]:
            demand = state["cooling_temp_c"] >= off_temp
        else:
            demand = state["cooling_temp_c"] >= thresh
            
        state["cooling_auto_demand"] = demand
        may_energize = demand
        state["blocked_reason"] = "none"
        
    state["cooling_demand"] = demand
    
    # Check Lockout block
    if may_energize and state["lockout_active"]:
        may_energize = False
        state["blocked_reason"] = "compressor_lockout"
        
    # Apply relay level
    was_energized = state["cooling_relay_state"]
    state["cooling_relay_state"] = may_energize
    
    # If turned off, start lockout
    if was_energized and not may_energize:
        state["lockout_remaining_sec"] = state["cooling_compressor_min_off_sec"]
        state["lockout_active"] = True

def simulation_thread_func():
    last_time = time.time()
    while True:
        time.sleep(0.1)
        now = time.time()
        dt_ms = int((now - last_time) * 1000)
        last_time = now
        
        # Periodic float auto-toggle every 30 seconds for local preview dynamism
        if int(now) % 30 == 0:
            if not hasattr(simulation_thread_func, "last_toggle_sec") or simulation_thread_func.last_toggle_sec != int(now):
                simulation_thread_func.last_toggle_sec = int(now)
                state["float_switch_on"] = not state["float_switch_on"]
                print(f"[Simulation] Float switch auto-toggled to {'ON (Water High)' if state['float_switch_on'] else 'OFF (Water Normal)'}")
                
        # Simulate slight temperature fluctuation
        if state["cooling_relay_state"]:
            state["cooling_temp_c"] -= 0.05 * (dt_ms / 1000.0)
        else:
            state["cooling_temp_c"] += 0.02 * (dt_ms / 1000.0)
        state["cooling_temp_c"] = max(24.0, min(35.0, state["cooling_temp_c"]))
        
        update_pump_simulation(dt_ms)
        update_cooling_simulation(dt_ms)


class MockWebServer(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        # Serve static assets directly if they contain file extensions
        if any(path.endswith(ext) for ext in ['.js', '.css', '.ico', '.png', '.jpg', '.txt']):
            return os.path.join(STATIC_DIR, path.lstrip('/'))
        
        # If it's an API route, let the handler handle it without routing to index.html
        if path.startswith('/api/'):
            return super().translate_path(path)
            
        # SPA routing: map all page routes (/login, /dashboard, /status, /wifi, /hardware) to index.html
        return os.path.join(STATIC_DIR, 'index.html')

    def do_GET(self):
        parsed_url = urllib.parse.urlparse(self.path)
        path = parsed_url.path

        if path.startswith('/api/'):
            # Auth Check (Check cookie)
            cookie_header = self.headers.get('Cookie', '')
            is_authed = 'session=mock-token-12345' in cookie_header
            
            if not is_authed and path not in ['/api/login']:
                self.send_response(401)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "unauthorized"}).encode('utf-8'))
                return

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            
            # API: Status
            if path == '/api/status':
                uptime = int((time.time() - state["uptime_start"]) * 1000)
                data = {
                    "ok": True,
                    "chip_model": "ESP32-D0WDQ6 (Mock)",
                    "chip_revision": 1,
                    "chip_cores": 2,
                    "cpu_freq_mhz": state["cpu_freq_mhz"],
                    "idf_version": "v6.0.1-Mock",
                    "project_name": "Fish Pump Relay Timer Control",
                    "project_version": "v0.1.0",
                    "reset_reason": "POWERON_RESET",
                    "mac_sta": "24:6F:28:1A:2B:3C",
                    "mac_ap": "24:6F:28:1A:2B:3D",
                    "free_heap": state["free_heap"],
                    "min_free_heap": state["min_free_heap"],
                    "total_heap": state["total_heap"],
                    "largest_free_block": state["largest_free_block"],
                    "uptime_ms": uptime,
                    "wifi_mode": state["wifi_mode"],
                    "ap_enabled": True,
                    "ap_ssid": "FishPump-Setup",
                    "ap_ip": "192.168.4.1",
                    "ap_clients": 1,
                    "ap_client_weakest_rssi": -75,
                    "sta_connected": state["wifi_connected"],
                    "sta_connecting": False,
                    "sta_retry_blocked": False,
                    "sta_ip": "192.168.1.150" if state["wifi_connected"] else "",
                    "sta_ssid": state["wifi_ssid"] if state["wifi_connected"] else "",
                    "sta_rssi": -62 if state["wifi_connected"] else 0,
                    "sta_channel": 11 if state["wifi_connected"] else 0,
                    "sta_auth": "WPA2_PSK" if state["wifi_connected"] else "",
                    "dns_server": True,
                    "http_json_send_failures": state["http_json_send_failures"],
                    "http_static_send_failures": state["http_static_send_failures"],
                    "http_static_cache_hits": state["http_static_cache_hits"],
                    "http_static_deadline_aborts": state["http_static_deadline_aborts"],
                    "stg_type": state["stg_type"]
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Pump Status
            elif path == '/api/pump/status':
                active_map = state["hardware_map"]
                data = {
                    "ok": True,
                    "running": state["running"],
                    "initialized": state["initialized"],
                    "config_valid": state["config_valid"],
                    "initial_stabilizing": state["initial_stabilizing"],
                    "fault": state["fault"],
                    "float_state": state["float_state"],
                    "active_timer": state["active_timer"],
                    "active_relay": state["active_relay"],
                    "phase": state["phase"],
                    "countdown_sec": state["countdown_sec"],
                    "relay_energized": state["relay1_energized"] or state["relay2_energized"],
                    "relay1_energized": state["relay1_energized"],
                    "relay2_energized": state["relay2_energized"],
                    "float_gpio": active_map["float_input_gpio"],
                    "relay_gpio": active_map["pump_relay1_gpio"], # Real ESP32 hardcodes relay_gpio to pump_relay1_gpio in status API
                    "active_relay_gpio": active_map["pump_relay2_gpio"] if state["active_relay"] == "relay2" else active_map["pump_relay1_gpio"],
                    "pump_relay1_gpio": active_map["pump_relay1_gpio"],
                    "pump_relay2_gpio": active_map["pump_relay2_gpio"],
                    "ds18b20_gpio": active_map["ds18b20_gpio"],
                    "cooling_relay_gpio": active_map["cooling_relay_gpio"],
                    "relay1_polarity": "active_low" if state["relay1_active_low"] else "active_high",
                    "relay2_polarity": "active_low" if state["relay2_active_low"] else "active_high",
                    "cooling_relay_polarity": state["cooling_relay_polarity"],
                    "timer1_start_phase": state["timer1_start_phase"],
                    "timer2_start_phase": state["timer2_start_phase"],
                    "hardware_reboot_required": state["reboot_required"],
                    "hardware_map_status": "loaded",
                    "auto_start": state["auto_start"],
                    "settings_status": "loaded"
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Pump Config GET
            elif path == '/api/pump/config':
                active_map = state["hardware_map"]
                data = {
                    "ok": True,
                    "timer1_on_sec": state["timer1_on_sec"],
                    "timer1_off_sec": state["timer1_off_sec"],
                    "timer2_on_sec": state["timer2_on_sec"],
                    "timer2_off_sec": state["timer2_off_sec"],
                    "auto_start": state["auto_start"],
                    "relay_polarity": "active_low" if state["relay1_active_low"] else "active_high",
                    "float_gpio": active_map["float_input_gpio"],
                    "relay_gpio": active_map["pump_relay1_gpio"],
                    "debounce_ms": state["debounce_ms"],
                    "pump_relay1_gpio": active_map["pump_relay1_gpio"],
                    "pump_relay2_gpio": active_map["pump_relay2_gpio"],
                    "ds18b20_gpio": active_map["ds18b20_gpio"],
                    "cooling_relay_gpio": active_map["cooling_relay_gpio"],
                    "relay1_polarity": "active_low" if state["relay1_active_low"] else "active_high",
                    "relay2_polarity": "active_low" if state["relay2_active_low"] else "active_high",
                    "timer1_start_phase": state["timer1_start_phase"],
                    "timer2_start_phase": state["timer2_start_phase"],
                    "cooling_relay_polarity": state["cooling_relay_polarity"],
                    "hardware_reboot_required": state["reboot_required"],
                    "hardware_fields_read_only": True,
                    "hardware_map_status": "loaded",
                    "settings_status": "loaded"
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Cooling Status GET
            elif path == '/api/cooling/status':
                data = {
                    "ok": True,
                    "initialized": state["initialized"],
                    "config_valid": state["config_valid"],
                    "mode": state["cooling_mode"],
                    "auto_enable": state["cooling_auto_enable"],
                    "temperature_c": state["cooling_temp_c"],
                    "temperature_valid": True,
                    "sensor_state": "ok",
                    "fault": False,
                    "fault_code": "none",
                    "relay_energized": state["cooling_relay_state"],
                    "threshold_c": state["cooling_threshold_c_x10"] / 10.0,
                    "hysteresis_c": state["cooling_hysteresis_c_x10"] / 10.0,
                    "lockout_active": state["lockout_active"],
                    "lockout_remaining_sec": int(state["lockout_remaining_sec"]),
                    "test_remaining_sec": int(state["test_remaining_sec"]),
                    "cooling_demand": state["cooling_demand"],
                    "blocked_reason": state["blocked_reason"],
                    "ds18b20_gpio": state["hardware_map"]["ds18b20_gpio"],
                    "cooling_relay_gpio": state["hardware_map"]["cooling_relay_gpio"]
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Cooling Config GET
            elif path == '/api/cooling/config':
                data = {
                    "ok": True,
                    "settings_status": "loaded",
                    "threshold_c": state["cooling_threshold_c_x10"] / 10.0,
                    "threshold_c_x10": state["cooling_threshold_c_x10"],
                    "hysteresis_c": state["cooling_hysteresis_c_x10"] / 10.0,
                    "hysteresis_c_x10": state["cooling_hysteresis_c_x10"],
                    "auto_enable": state["cooling_auto_enable"],
                    "mode": state["cooling_mode"],
                    "test_timeout_sec": state["cooling_test_timeout_sec"],
                    "compressor_min_off_sec": state["cooling_compressor_min_off_sec"],
                    "cooling_relay_polarity": state["cooling_relay_polarity"],
                    "limits": {
                        "threshold_c_x10_min": -550,
                        "threshold_c_x10_max": 1250,
                        "hysteresis_c_x10_min": 1,
                        "hysteresis_c_x10_max": 500,
                        "test_timeout_sec_min": 1,
                        "test_timeout_sec_max": 3600,
                        "compressor_min_off_sec_min": 0,
                        "compressor_min_off_sec_max": 86400
                    },
                    "modes": ["auto", "force_off", "test_on"],
                    "relay_polarities": ["active_low", "active_high"],
                    "status": {
                        "initialized": state["initialized"],
                        "config_valid": state["config_valid"],
                        "mode": state["cooling_mode"],
                        "auto_enable": state["cooling_auto_enable"],
                        "temperature_c": state["cooling_temp_c"],
                        "temperature_valid": True,
                        "sensor_state": "ok",
                        "fault": False,
                        "fault_code": "none",
                        "relay_energized": state["cooling_relay_state"],
                        "threshold_c": state["cooling_threshold_c_x10"] / 10.0,
                        "hysteresis_c": state["cooling_hysteresis_c_x10"] / 10.0,
                        "lockout_active": state["lockout_active"],
                        "lockout_remaining_sec": int(state["lockout_remaining_sec"]),
                        "test_remaining_sec": int(state["test_remaining_sec"]),
                        "cooling_demand": state["cooling_demand"],
                        "blocked_reason": state["blocked_reason"],
                        "ds18b20_gpio": state["hardware_map"]["ds18b20_gpio"],
                        "cooling_relay_gpio": state["hardware_map"]["cooling_relay_gpio"]
                    }
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Wi-Fi Scan
            elif path == '/api/wifi/scan':
                data = {
                    "ok": True,
                    "networks": state["wifi_networks_in_air"]
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Wi-Fi Profiles GET
            elif path == '/api/wifi/profiles':
                data = {
                    "ok": True,
                    "auto_index": state["wifi_auto_index"],
                    "profiles": state["wifi_profiles"]
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Hardware Map GET
            elif path == '/api/hardware/map':
                active_map = state["hardware_map"]
                pending_map = state["pending_hardware_map"]
                data = {
                    "ok": True,
                    "active_status": "loaded",
                    "pending_status": "loaded" if pending_map else "not_found",
                    "pending_valid": pending_map is not None,
                    "reboot_required": state["reboot_required"],
                    "active": {
                        "float_input_gpio": active_map["float_input_gpio"],
                        "pump_relay1_gpio": active_map["pump_relay1_gpio"],
                        "pump_relay2_gpio": active_map["pump_relay2_gpio"],
                        "ds18b20_gpio": active_map["ds18b20_gpio"],
                        "cooling_relay_gpio": active_map["cooling_relay_gpio"]
                    },
                    "pending": pending_map,
                    "options": GPIO_OPTIONS
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                
            # API: Auth Nonce GET
            elif path == '/api/auth/nonce':
                state["auth_nonce"] = secrets.token_hex(16)
                self.wfile.write(json.dumps({"ok": True, "nonce": state["auth_nonce"]}).encode('utf-8'))
                
            else:
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
            return

        super().do_GET()

    def do_POST(self):
        parsed_url = urllib.parse.urlparse(self.path)
        path = parsed_url.path

        if path.startswith('/api/'):
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length) if content_length > 0 else b''
            
            payload = {}
            if post_data:
                try:
                    payload = json.loads(post_data.decode('utf-8'))
                except:
                    pass

            # Check Auth for all APIs except /api/login
            cookie_header = self.headers.get('Cookie', '')
            is_authed = 'session=mock-token-12345' in cookie_header
            
            if not is_authed and path not in ['/api/login']:
                self.send_response(401)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "error": "unauthorized"}).encode('utf-8'))
                return

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            
            # API: Login
            if path == '/api/login':
                req_user = payload.get("username", "")
                req_pass = payload.get("password", "")
                if req_user == state["username"] and req_pass == state["password"]:
                    self.send_header('Set-Cookie', 'session=mock-token-12345; Path=/; Max-Age=3600')
                    self.end_headers()
                    self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                else:
                    self.end_headers()
                    self.wfile.write(json.dumps({"ok": False, "error": "invalid_credentials"}).encode('utf-8'))
                return
                
            self.end_headers()

            # API: Logout
            if path == '/api/logout':
                # Note: clearing cookies is typically handled by setting Max-Age=0
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: Pump Config Save
            elif path == '/api/pump/config':
                # Check for read-only fields first
                readonly_fields = [
                    "float_gpio", "relay_gpio", "debounce_ms", 
                    "pump_relay1_gpio", "pump_relay2_gpio", 
                    "ds18b20_gpio", "cooling_relay_gpio", 
                    "cooling_relay_polarity", "hardware_reboot_required"
                ]
                for f in readonly_fields:
                    if f in payload:
                        self.wfile.write(json.dumps({
                            "ok": False,
                            "error": "read_only_field",
                            "message": f
                        }).encode('utf-8'))
                        return

                state["timer1_on_sec"] = int(payload.get("timer1_on_sec", state["timer1_on_sec"]))
                state["timer1_off_sec"] = int(payload.get("timer1_off_sec", state["timer1_off_sec"]))
                state["timer2_on_sec"] = int(payload.get("timer2_on_sec", state["timer2_on_sec"]))
                state["timer2_off_sec"] = int(payload.get("timer2_off_sec", state["timer2_off_sec"]))
                state["timer1_start_phase"] = payload.get("timer1_start_phase", state["timer1_start_phase"])
                state["timer2_start_phase"] = payload.get("timer2_start_phase", state["timer2_start_phase"])
                
                pol1 = payload.get("relay1_polarity", payload.get("relay_polarity", "active_low"))
                pol2 = payload.get("relay2_polarity", "active_low")
                state["relay1_active_low"] = (pol1 == "active_low")
                state["relay2_active_low"] = (pol2 == "active_low")
                
                state["auto_start"] = bool(payload.get("auto_start", state["auto_start"]))
                
                # trigger reset of timings
                now_ms = int(time.time() * 1000)
                if state["running"]:
                    start_channel(state["confirmed_float_state"], now_ms)
                else:
                    state["countdown_sec"] = state["timer1_on_sec"] if state["active_timer"] == "timer1" else state["timer2_on_sec"]
                    
                # Return config JSON
                active_map = state["hardware_map"]
                data = {
                    "ok": True,
                    "timer1_on_sec": state["timer1_on_sec"],
                    "timer1_off_sec": state["timer1_off_sec"],
                    "timer2_on_sec": state["timer2_on_sec"],
                    "timer2_off_sec": state["timer2_off_sec"],
                    "auto_start": state["auto_start"],
                    "relay_polarity": "active_low" if state["relay1_active_low"] else "active_high",
                    "float_gpio": active_map["float_input_gpio"],
                    "relay_gpio": active_map["pump_relay1_gpio"],
                    "debounce_ms": state["debounce_ms"],
                    "pump_relay1_gpio": active_map["pump_relay1_gpio"],
                    "pump_relay2_gpio": active_map["pump_relay2_gpio"],
                    "ds18b20_gpio": active_map["ds18b20_gpio"],
                    "cooling_relay_gpio": active_map["cooling_relay_gpio"],
                    "relay1_polarity": "active_low" if state["relay1_active_low"] else "active_high",
                    "relay2_polarity": "active_low" if state["relay2_active_low"] else "active_high",
                    "timer1_start_phase": state["timer1_start_phase"],
                    "timer2_start_phase": state["timer2_start_phase"],
                    "cooling_relay_polarity": state["cooling_relay_polarity"],
                    "hardware_reboot_required": state["reboot_required"],
                    "hardware_fields_read_only": True,
                    "hardware_map_status": "loaded",
                    "settings_status": "loaded"
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                return

            # API: Pump Start
            elif path == '/api/pump/start':
                already_running = state["running"]
                state["running"] = True
                state["initial_stabilizing"] = True
                self.wfile.write(json.dumps({
                    "ok": True,
                    "already_running": already_running,
                    "running": True,
                    "initialized": True,
                    "config_valid": True,
                    "initial_stabilizing": True,
                    "fault": False,
                    "float_state": state["float_state"],
                    "active_timer": "none",
                    "active_relay": "none",
                    "phase": "idle",
                    "countdown_sec": 0,
                    "relay_energized": False,
                    "relay1_energized": False,
                    "relay2_energized": False,
                    "float_gpio": state["hardware_map"]["float_input_gpio"],
                    "relay_gpio": state["hardware_map"]["pump_relay1_gpio"],
                    "active_relay_gpio": state["hardware_map"]["pump_relay1_gpio"],
                    "pump_relay1_gpio": state["hardware_map"]["pump_relay1_gpio"],
                    "pump_relay2_gpio": state["hardware_map"]["pump_relay2_gpio"],
                    "ds18b20_gpio": state["hardware_map"]["ds18b20_gpio"],
                    "cooling_relay_gpio": state["hardware_map"]["cooling_relay_gpio"],
                    "relay1_polarity": "active_low" if state["relay1_active_low"] else "active_high",
                    "relay2_polarity": "active_low" if state["relay2_active_low"] else "active_high",
                    "cooling_relay_polarity": state["cooling_relay_polarity"],
                    "timer1_start_phase": state["timer1_start_phase"],
                    "timer2_start_phase": state["timer2_start_phase"],
                    "hardware_reboot_required": state["reboot_required"],
                    "hardware_map_status": "loaded",
                    "auto_start": state["auto_start"],
                    "settings_status": "loaded"
                }).encode('utf-8'))
                return

            # API: Pump Stop
            elif path == '/api/pump/stop':
                already_stopped = not state["running"]
                state["running"] = False
                state["initial_stabilizing"] = False
                state["relay1_energized"] = False
                state["relay2_energized"] = False
                self.wfile.write(json.dumps({
                    "ok": True,
                    "already_stopped": already_stopped,
                    "running": False,
                    "initialized": True,
                    "config_valid": True,
                    "initial_stabilizing": False,
                    "fault": False,
                    "float_state": state["float_state"],
                    "active_timer": state["active_timer"],
                    "active_relay": state["active_relay"],
                    "phase": state["phase"],
                    "countdown_sec": state["countdown_sec"],
                    "relay_energized": False,
                    "relay1_energized": False,
                    "relay2_energized": False,
                    "float_gpio": state["hardware_map"]["float_input_gpio"],
                    "relay_gpio": state["hardware_map"]["pump_relay1_gpio"],
                    "active_relay_gpio": state["hardware_map"]["pump_relay2_gpio"] if state["active_relay"] == "relay2" else state["hardware_map"]["pump_relay1_gpio"],
                    "pump_relay1_gpio": state["hardware_map"]["pump_relay1_gpio"],
                    "pump_relay2_gpio": state["hardware_map"]["pump_relay2_gpio"],
                    "ds18b20_gpio": state["hardware_map"]["ds18b20_gpio"],
                    "cooling_relay_gpio": state["hardware_map"]["cooling_relay_gpio"],
                    "relay1_polarity": "active_low" if state["relay1_active_low"] else "active_high",
                    "relay2_polarity": "active_low" if state["relay2_active_low"] else "active_high",
                    "cooling_relay_polarity": state["cooling_relay_polarity"],
                    "timer1_start_phase": state["timer1_start_phase"],
                    "timer2_start_phase": state["timer2_start_phase"],
                    "hardware_reboot_required": state["reboot_required"],
                    "hardware_map_status": "loaded",
                    "auto_start": state["auto_start"],
                    "settings_status": "loaded"
                }).encode('utf-8'))
                return

            # API: Cooling Config Save
            elif path == '/api/cooling/config':
                state["cooling_threshold_c_x10"] = int(payload.get("threshold_c_x10", state["cooling_threshold_c_x10"]))
                state["cooling_hysteresis_c_x10"] = int(payload.get("hysteresis_c_x10", state["cooling_hysteresis_c_x10"]))
                state["cooling_test_timeout_sec"] = int(payload.get("test_timeout_sec", state["cooling_test_timeout_sec"]))
                state["cooling_compressor_min_off_sec"] = int(payload.get("compressor_min_off_sec", state["cooling_compressor_min_off_sec"]))
                state["cooling_auto_enable"] = bool(payload.get("auto_enable", state["cooling_auto_enable"]))
                state["cooling_relay_polarity"] = payload.get("relay_polarity", state["cooling_relay_polarity"])
                
                # Update mode according to auto_enable
                state["cooling_mode"] = "auto" if state["cooling_auto_enable"] else "force_off"
                
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: Cooling Mode Save
            elif path == '/api/cooling/mode':
                new_mode = payload.get("mode", state["cooling_mode"])
                if new_mode == "test_on":
                    if state["cooling_mode"] != "test_on":
                        state["cooling_previous_mode"] = state["cooling_mode"]
                    state["cooling_mode"] = "test_on"
                    state["test_remaining_sec"] = state["cooling_test_timeout_sec"]
                else:
                    state["cooling_mode"] = new_mode
                    state["test_remaining_sec"] = 0
                
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: Hardware Map POST
            elif path == '/api/hardware/map':
                state["pending_hardware_map"] = {
                    "float_input_gpio": int(payload.get("float_input_gpio", state["hardware_map"]["float_input_gpio"])),
                    "pump_relay1_gpio": int(payload.get("pump_relay1_gpio", state["hardware_map"]["pump_relay1_gpio"])),
                    "pump_relay2_gpio": int(payload.get("pump_relay2_gpio", state["hardware_map"]["pump_relay2_gpio"])),
                    "ds18b20_gpio": int(payload.get("ds18b20_gpio", state["hardware_map"]["ds18b20_gpio"])),
                    "cooling_relay_gpio": int(payload.get("cooling_relay_gpio", state["hardware_map"]["cooling_relay_gpio"]))
                }
                state["reboot_required"] = True
                
                active_map = state["hardware_map"]
                pending_map = state["pending_hardware_map"]
                data = {
                    "ok": True,
                    "active_status": "loaded",
                    "pending_status": "loaded",
                    "pending_valid": True,
                    "reboot_required": True,
                    "active": active_map,
                    "pending": pending_map,
                    "options": GPIO_OPTIONS
                }
                self.wfile.write(json.dumps(data).encode('utf-8'))
                return

            # API: Confirm POST
            elif path == '/api/confirm':
                if state["pending_hardware_map"]:
                    state["hardware_map"] = state["pending_hardware_map"]
                    state["pending_hardware_map"] = None
                state["reboot_required"] = False
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: WiFi Connect
            elif path == '/api/wifi/connect':
                ssid = payload.get("ssid", "")
                state["wifi_ssid"] = ssid
                state["wifi_connected"] = True
                found = False
                for p in state["wifi_profiles"]:
                    if p["ssid"] == ssid:
                        p["connected"] = True
                        found = True
                    else:
                        p["connected"] = False
                if not found:
                    state["wifi_profiles"].append({"ssid": ssid, "auto": True, "connected": True})
                    state["wifi_auto_index"] = len(state["wifi_profiles"]) - 1
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: WiFi Disconnect
            elif path == '/api/wifi/disconnect':
                state["wifi_connected"] = False
                for p in state["wifi_profiles"]:
                    p["connected"] = False
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: WiFi Profiles Save
            elif path == '/api/wifi/profiles/save':
                ssid = payload.get("ssid", "")
                found = False
                for p in state["wifi_profiles"]:
                    if p["ssid"] == ssid:
                        found = True
                if not found:
                    state["wifi_profiles"].append({"ssid": ssid, "auto": False, "connected": False})
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: WiFi Profiles Forget
            elif path == '/api/wifi/profiles/forget':
                ssid = payload.get("ssid", "")
                state["wifi_profiles"] = [p for p in state["wifi_profiles"] if p["ssid"] != ssid]
                if state["wifi_ssid"] == ssid:
                    state["wifi_connected"] = False
                    state["wifi_ssid"] = ""
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: WiFi Profiles SetAuto
            elif path == '/api/wifi/profiles/setauto':
                idx = payload.get("index", -1)
                ssid = payload.get("ssid", "")
                
                if idx != -1:
                    state["wifi_auto_index"] = idx
                elif ssid:
                    for i, p in enumerate(state["wifi_profiles"]):
                        if p["ssid"] == ssid:
                            state["wifi_auto_index"] = i
                            break
                else:
                    state["wifi_auto_index"] = -1
                    
                for i, p in enumerate(state["wifi_profiles"]):
                    p["auto"] = (i == state["wifi_auto_index"])
                    
                self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
                return

            # API: Auth Credentials POST
            elif path == '/api/auth/credentials':
                nonce = payload.get("nonce", "")
                curr_pass = payload.get("current_password", "")
                new_user = payload.get("new_username", "")
                new_pass = payload.get("new_password", "")
                
                if nonce != state["auth_nonce"] or not nonce:
                    self.wfile.write(json.dumps({"ok": False, "error": "invalid_nonce"}).encode('utf-8'))
                    return
                state["auth_nonce"] = ""
                
                if curr_pass != state["password"]:
                    self.wfile.write(json.dumps({"ok": False, "error": "invalid_credentials"}).encode('utf-8'))
                    return
                    
                if new_user:
                    state["username"] = new_user
                state["password"] = new_pass
                
                self.wfile.write(json.dumps({"ok": True, "reboot_pending": True}).encode('utf-8'))
                def do_reboot():
                    time.sleep(1.0)
                    state["uptime_start"] = time.time()
                    print("[Simulation] Device rebooted (Credentials updated).")
                threading.Thread(target=do_reboot, daemon=True).start()
                return

            self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
            return
            
        self.send_error(404, "File not found")

    def do_DELETE(self):
        parsed_url = urllib.parse.urlparse(self.path)
        path = parsed_url.path

        # API: Confirm DELETE
        if path == '/api/confirm':
            state["pending_hardware_map"] = None
            state["reboot_required"] = False
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"ok": True}).encode('utf-8'))
            return
            
        self.send_error(404, "Not found")

if __name__ == '__main__':
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    
    sim_thread = threading.Thread(target=simulation_thread_func, daemon=True)
    sim_thread.start()
    
    print("-----------------------------------------------------------------")
    print(f"Starting Aligned Mock Server at http://localhost:{PORT}")
    print("Open this URL in your web browser to preview the UI.")
    print("Log in with admin / admin123.")
    print("-----------------------------------------------------------------")
    
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("0.0.0.0", PORT), MockWebServer) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nStopping Mock Server...")
