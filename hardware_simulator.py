import http.server
import socketserver
import os
import json
import urllib.parse
import threading
import time
import secrets
import tkinter as tk
from tkinter import ttk

PORT = 8000
STATIC_DIR = os.path.join(os.path.dirname(__file__), 'main', 'static')

# Shared Simulator State
state = {
    # 1. ENVIRONMENT INPUTS (Controlled by Simulator GUI)
    "float_switch_on": False,         # False = Float OFF, True = Float ON
    "cooling_temp_c": 27.5,           # Simulated DS18B20 temperature
    "ds18b20_disconnected": False,    # Simulate DS18B20 hardware disconnect
    "wifi_networks_in_air": {
        "MyHomeWiFi": True,
        "Office_Network": True,
        "Guest_AP": False,
        "Neighbor_WiFi_AP": True
    },
    
    # 2. ESP32 HARDWARE RESOURCES (Controlled by Simulator GUI)
    "free_heap": 185000,              # RAM size in bytes
    "min_free_heap": 172900,
    "total_heap": 284000,
    "largest_free_block": 118000,
    "cpu_freq_mhz": 240,              # CPU frequency
    "stg_type": 0,                    # Staging type mock
    "router_offline": False,          # Simulate router/connection drops
    "wifi_connected": True,
    "wifi_ssid": "MyHomeWiFi",
    "wifi_mode": "APSTA",
    "wifi_profiles": [
        {"ssid": "MyHomeWiFi", "auto": True, "connected": True}
    ],
    "wifi_auto_index": 0,
    
    # Credentials
    "username": "admin",
    "password": "admin123",
    "auth_nonce": "",
    
    # 3. ESP32 CONFIGURATIONS (Configured via Web Dashboard APIs)
    "running": False,
    "timer1_on_sec": 20,
    "timer1_off_sec": 60,
    "timer2_on_sec": 10,
    "timer2_off_sec": 180,
    "timer1_start_phase": "on",
    "timer2_start_phase": "on",
    "relay1_active_low": True,
    "relay2_active_low": True,
    "auto_start": False,
    "debounce_ms": 100,
    
    "cooling_threshold_c_x10": 300,   # 30.0 C
    "cooling_hysteresis_c_x10": 10,   # 1.0 C
    "cooling_auto_enable": False,
    "cooling_test_timeout_sec": 10,
    "cooling_compressor_min_off_sec": 10,
    "cooling_mode": "force_off",      # auto, force_off, test_on
    "cooling_previous_mode": "force_off",
    "cooling_relay_polarity": "active_low",
    
    "hardware_map": {
        "float_input_gpio": 32,
        "pump_relay1_gpio": 26,
        "pump_relay2_gpio": 27,
        "ds18b20_gpio": 33,
        "cooling_relay_gpio": 25
    },
    "pending_hardware_map": None,
    "reboot_required": False,

    # 4. ESP32 RUNTIME DYNAMIC OUTPUT STATES
    "active_timer": "timer1",
    "active_relay": "relay1",
    "phase": "idle",
    "countdown_sec": 0,
    "phase_deadline_ms": 0,
    "relay1_energized": False,
    "relay2_energized": False,
    
    # Debounce internals
    "confirmed_float_state": "off",
    "pending_float_state": "off",
    "pending_since_ms": 0,
    "float_changed": False,
    "initial_stabilizing": False,
    
    # Cooling internals
    "cooling_relay_state": False,
    "cooling_demand": False,
    "cooling_auto_demand": False,
    "lockout_active": False,
    "lockout_remaining_sec": 0,
    "test_remaining_sec": 0,
    "blocked_reason": "force_off",
    
    # Sensor fault simulation
    "failed_reads": 0,
    "sensor_state": "ok",
    "fault_code": "none",
    "initialized": True,
    "config_valid": True,
    "fault": False
}

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

start_time = time.time()

# Helper function to start pump channel
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

# Helper function to set pump phase
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


# ----------------- HTTP Mock Web Server -----------------
class SimulatorHTTPHandler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        if any(path.endswith(ext) for ext in ['.js', '.css', '.ico', '.png', '.jpg', '.txt']):
            return os.path.join(STATIC_DIR, path.lstrip('/'))
        if path.startswith('/api/'):
            return super().translate_path(path)
        return os.path.join(STATIC_DIR, 'index.html')

    def do_GET(self):
        parsed_url = urllib.parse.urlparse(self.path)
        path = parsed_url.path

        if path.startswith('/api/'):
            # Auth check
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
            
            # API: System Status
            if path == '/api/status':
                uptime = int((time.time() - start_time) * 1000)
                data = {
                    "ok": True,
                    "chip_model": "ESP32-D0WDQ6 (Simulated)",
                    "chip_revision": 1,
                    "chip_cores": 2,
                    "cpu_freq_mhz": state["cpu_freq_mhz"],
                    "idf_version": "v6.0.1-Simulator",
                    "project_name": "Fish Pump Relay Timer Control",
                    "project_version": "v0.1.6",
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
                    "ap_client_weakest_rssi": -72,
                    "sta_connected": state["wifi_connected"] and not state["router_offline"],
                    "sta_connecting": False,
                    "sta_retry_blocked": False,
                    "sta_ip": "192.168.1.150" if (state["wifi_connected"] and not state["router_offline"]) else "",
                    "sta_ssid": state["wifi_ssid"] if (state["wifi_connected"] and not state["router_offline"]) else "",
                    "sta_rssi": -65 if (state["wifi_connected"] and not state["router_offline"]) else 0,
                    "sta_channel": 6 if (state["wifi_connected"] and not state["router_offline"]) else 0,
                    "sta_auth": "WPA2_PSK" if (state["wifi_connected"] and not state["router_offline"]) else "",
                    "dns_server": True,
                    "http_json_send_failures": 0,
                    "http_static_send_failures": 0,
                    "http_static_cache_hits": 45,
                    "http_static_deadline_aborts": 0,
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
                    "relay_gpio": active_map["pump_relay1_gpio"],
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
                    "temperature_c": state["cooling_temp_c"] if not state["ds18b20_disconnected"] else 0.0,
                    "temperature_valid": not state["ds18b20_disconnected"],
                    "sensor_state": state["sensor_state"],
                    "fault": state["fault"],
                    "fault_code": state["fault_code"],
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
                        "temperature_c": state["cooling_temp_c"] if not state["ds18b20_disconnected"] else 0.0,
                        "temperature_valid": not state["ds18b20_disconnected"],
                        "sensor_state": state["sensor_state"],
                        "fault": state["fault"],
                        "fault_code": state["fault_code"],
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
                
            # API: Wi-Fi Scan (Only shows checked networks from air)
            elif path == '/api/wifi/scan':
                nets = []
                for ssid, active in state["wifi_networks_in_air"].items():
                    if active:
                        nets.append({"ssid": ssid, "rssi": -50 - int(time.time() % 20), "secure": True})
                data = {
                    "ok": True,
                    "networks": nets
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
                    global start_time
                    start_time = time.time()
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

# Run web server in a background thread
def start_web_server():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("0.0.0.0", PORT), SimulatorHTTPHandler) as httpd:
        httpd.serve_forever()


# ----------------- GUI Environment Control Panel (Tkinter) -----------------
class SimulatorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 Fish Pump - Aligned Hardware Simulator")
        self.root.geometry("900x720")
        self.root.resizable(False, False)
        
        self.root.configure(bg="#121216")
        self.style = ttk.Style()
        self.style.theme_use("clam")
        
        self.style.configure(".", background="#121216", foreground="#ffffff")
        self.style.configure("TLabel", background="#121216", foreground="#ffffff", font=("Courier", 10))
        self.style.configure("Header.TLabel", font=("Courier", 12, "bold"), foreground="#00bcd4")
        self.style.configure("Title.TLabel", font=("Courier", 16, "bold"), foreground="#00bcd4")
        
        self.style.configure("Panel.TFrame", background="#1a1a24")
        self.style.configure("Panel.TLabel", background="#1a1a24", foreground="#e2e8f0", font=("Courier", 10))
        self.style.configure("PanelHeader.TLabel", background="#1a1a24", foreground="#00bcd4", font=("Courier", 12, "bold"))
        self.style.configure("PanelCountdown.TLabel", background="#1a1a24", foreground="#38bdf8", font=("Courier", 11, "bold"))
        self.style.configure("ConfigLabel.TLabel", background="#1a1a24", foreground="#a7f3d0", font=("Courier", 9))
        self.style.configure("OutputLabel.TLabel", background="#1a1a24", foreground="#fbcfe8", font=("Courier", 10))

        self.build_ui()
        self.update_gui_loop()
        self.hardware_tick_loop()

    def build_ui(self):
        title_label = ttk.Label(self.root, text="ESP32 PHYSICAL ENVIRONMENT SIMULATOR (100ms)", style="Title.TLabel")
        title_label.pack(pady=12)
        
        web_info = ttk.Label(self.root, text=f"HTTP Server: http://localhost:{PORT}  |  Dashboard: http://localhost:{PORT}/dashboard", foreground="#10b981", font=("Courier", 10, "italic"))
        web_info.pack(pady=1)

        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=15, pady=8)
        
        # LEFT COLUMN (Inputs)
        left_col = ttk.Frame(main_frame)
        left_col.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)
        
        # 1. Environment Inputs
        env_panel = ttk.Frame(left_col, style="Panel.TFrame")
        env_panel.pack(fill=tk.BOTH, expand=True, pady=5)
        
        ttk.Label(env_panel, text=" [1. PHYSICAL ENVIRONMENT INPUTS] ", style="PanelHeader.TLabel").pack(anchor=tk.W, padx=10, pady=8)
        
        # Float Switch
        float_frame = ttk.Frame(env_panel, style="Panel.TFrame")
        float_frame.pack(fill=tk.X, padx=15, pady=4)
        ttk.Label(float_frame, text="Float Switch Level (GPIO 32):", style="Panel.TLabel").pack(anchor=tk.W)
        self.float_var = tk.BooleanVar(value=state["float_switch_on"])
        self.float_cb = tk.Checkbutton(
            float_frame, 
            text="Water High (Switch ON -> Timer 2)", 
            variable=self.float_var,
            bg="#1a1a24", fg="#34d399", selectcolor="#121216",
            activebackground="#1a1a24", activeforeground="#34d399",
            font=("Courier", 9), command=self.on_float_changed
        )
        self.float_cb.pack(anchor=tk.W, pady=2)
        
        # Temp Sensor
        temp_frame = ttk.Frame(env_panel, style="Panel.TFrame")
        temp_frame.pack(fill=tk.X, padx=15, pady=4)
        self.temp_label_var = tk.StringVar(value=f"DS18B20 Temp (GPIO 33): {state['cooling_temp_c']} °C")
        ttk.Label(temp_frame, textvariable=self.temp_label_var, style="Panel.TLabel").pack(anchor=tk.W)
        self.temp_scale = tk.Scale(
            temp_frame, from_=15.0, to=40.0, resolution=0.1, orient=tk.HORIZONTAL,
            bg="#1a1a24", fg="#ffffff", troughcolor="#121216", highlightbackground="#1a1a24",
            command=self.on_temp_changed
        )
        self.temp_scale.set(state["cooling_temp_c"])
        self.temp_scale.pack(fill=tk.X, pady=2)
        
        # Disconnect Sensor Checkbox
        self.disconnect_var = tk.BooleanVar(value=state["ds18b20_disconnected"])
        self.disconnect_cb = tk.Checkbutton(
            env_panel, 
            text="Simulate DS18B20 Disconnected (Sensor Fault)", 
            variable=self.disconnect_var,
            bg="#1a1a24", fg="#f59e0b", selectcolor="#121216",
            activebackground="#1a1a24", activeforeground="#f59e0b",
            font=("Courier", 9), command=self.on_disconnect_changed
        )
        self.disconnect_cb.pack(anchor=tk.W, padx=15, pady=2)
        
        # Wi-Fi in Air
        wifi_frame = ttk.Frame(env_panel, style="Panel.TFrame")
        wifi_frame.pack(fill=tk.X, padx=15, pady=4)
        ttk.Label(wifi_frame, text="Wi-Fi APs visible in the air:", style="Panel.TLabel").pack(anchor=tk.W, pady=2)
        
        self.wifi_vars = {}
        for ssid in state["wifi_networks_in_air"]:
            var = tk.BooleanVar(value=state["wifi_networks_in_air"][ssid])
            self.wifi_vars[ssid] = var
            cb = tk.Checkbutton(
                wifi_frame, text=ssid, variable=var,
                bg="#1a1a24", fg="#f59e0b", selectcolor="#121216",
                activebackground="#1a1a24", activeforeground="#f59e0b",
                font=("Courier", 9), command=self.on_wifi_air_changed
            )
            cb.pack(anchor=tk.W)
            
        # 2. ESP32 System Resources
        res_panel = ttk.Frame(left_col, style="Panel.TFrame")
        res_panel.pack(fill=tk.BOTH, expand=True, pady=5)
        
        ttk.Label(res_panel, text=" [2. ESP32 SYSTEM RESOURCES] ", style="PanelHeader.TLabel").pack(anchor=tk.W, padx=10, pady=8)
        
        # Heap Size
        heap_frame = ttk.Frame(res_panel, style="Panel.TFrame")
        heap_frame.pack(fill=tk.X, padx=15, pady=4)
        self.heap_label_var = tk.StringVar(value=f"Simulated Free Heap: {state['free_heap'] // 1024} KB")
        ttk.Label(heap_frame, textvariable=self.heap_label_var, style="Panel.TLabel").pack(anchor=tk.W)
        self.heap_scale = tk.Scale(
            heap_frame, from_=35000, to=240000, resolution=1000, orient=tk.HORIZONTAL,
            bg="#1a1a24", fg="#ffffff", troughcolor="#121216", highlightbackground="#1a1a24",
            command=self.on_heap_changed
        )
        self.heap_scale.set(state["free_heap"])
        self.heap_scale.pack(fill=tk.X, pady=2)
        
        # CPU Freq
        cpu_frame = ttk.Frame(res_panel, style="Panel.TFrame")
        cpu_frame.pack(fill=tk.X, padx=15, pady=2)
        ttk.Label(cpu_frame, text="CPU Clock Frequency:", style="Panel.TLabel").grid(row=0, column=0, sticky=tk.W, pady=2)
        
        self.cpu_var = tk.StringVar(value=str(state["cpu_freq_mhz"]))
        self.cpu_combo = ttk.Combobox(cpu_frame, textvariable=self.cpu_var, values=["80", "160", "240"], width=6, state="readonly")
        self.cpu_combo.grid(row=0, column=1, padx=5, pady=2, sticky=tk.W)
        self.cpu_combo.bind("<<ComboboxSelected>>", self.on_cpu_changed)
        
        # Router Offline Checkbox
        self.offline_var = tk.BooleanVar(value=state["router_offline"])
        self.offline_cb = tk.Checkbutton(
            res_panel, text="Simulate Router Connection Loss (Router Offline)",
            variable=self.offline_var, bg="#1a1a24", fg="#f43f5e", selectcolor="#121216",
            activebackground="#1a1a24", activeforeground="#f43f5e",
            font=("Courier", 9), command=self.on_offline_changed
        )
        self.offline_cb.pack(anchor=tk.W, padx=15, pady=6)

        # RIGHT COLUMN (Outputs & Logic)
        right_col = ttk.Frame(main_frame)
        right_col.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5)
        
        # 3. Output Indicators
        out_panel = ttk.Frame(right_col, style="Panel.TFrame")
        out_panel.pack(fill=tk.BOTH, expand=True, pady=5)
        
        ttk.Label(out_panel, text=" [3. ESP32 OUTPUT INDICATORS] ", style="PanelHeader.TLabel").pack(anchor=tk.W, padx=10, pady=8)
        
        # Indicators
        self.indicator_frame = ttk.Frame(out_panel, style="Panel.TFrame")
        self.indicator_frame.pack(fill=tk.X, padx=15, pady=4)
        
        # Relay 1 (Pump 1)
        self.relay1_canvas = tk.Canvas(self.indicator_frame, width=20, height=20, bg="#1a1a24", highlightbackground="#1a1a24")
        self.relay1_canvas.grid(row=0, column=0, pady=3, sticky=tk.W)
        self.relay1_oval = self.relay1_canvas.create_oval(2, 2, 18, 18, fill="#4b5563")
        self.relay1_label = ttk.Label(self.indicator_frame, text="Pump Relay 1 (GPIO 26) [Timer 1]", style="OutputLabel.TLabel")
        self.relay1_label.grid(row=0, column=1, padx=10, pady=3, sticky=tk.W)
        
        # Relay 2 (Pump 2)
        self.relay2_canvas = tk.Canvas(self.indicator_frame, width=20, height=20, bg="#1a1a24", highlightbackground="#1a1a24")
        self.relay2_canvas.grid(row=1, column=0, pady=3, sticky=tk.W)
        self.relay2_oval = self.relay2_canvas.create_oval(2, 2, 18, 18, fill="#4b5563")
        self.relay2_label = ttk.Label(self.indicator_frame, text="Pump Relay 2 (GPIO 27) [Timer 2]", style="OutputLabel.TLabel")
        self.relay2_label.grid(row=1, column=1, padx=10, pady=3, sticky=tk.W)
        
        # Cooling relay
        self.cooling_canvas = tk.Canvas(self.indicator_frame, width=20, height=20, bg="#1a1a24", highlightbackground="#1a1a24")
        self.cooling_canvas.grid(row=2, column=0, pady=3, sticky=tk.W)
        self.cooling_oval = self.cooling_canvas.create_oval(2, 2, 18, 18, fill="#4b5563")
        self.cooling_label = ttk.Label(self.indicator_frame, text="Cooling Relay (GPIO 25) [Fan]", style="OutputLabel.TLabel")
        self.cooling_label.grid(row=2, column=1, padx=10, pady=3, sticky=tk.W)

        # Dynamic State Variables
        self.run_status_var = tk.StringVar(value="Status: STOPPED")
        self.active_timer_var = tk.StringVar(value="Active Timer: -")
        self.phase_var = tk.StringVar(value="Current Phase: -")
        self.countdown_var = tk.StringVar(value="Countdown: --s")
        self.cool_status_var = tk.StringVar(value="Cooling: -")
        
        ttk.Label(out_panel, textvariable=self.run_status_var, font=("Courier", 10, "bold"), style="Panel.TLabel").pack(anchor=tk.W, padx=15, pady=1)
        ttk.Label(out_panel, textvariable=self.active_timer_var, style="Panel.TLabel").pack(anchor=tk.W, padx=15, pady=1)
        ttk.Label(out_panel, textvariable=self.phase_var, style="Panel.TLabel").pack(anchor=tk.W, padx=15, pady=1)
        ttk.Label(out_panel, textvariable=self.countdown_var, style="PanelCountdown.TLabel").pack(anchor=tk.W, padx=15, pady=2)
        ttk.Label(out_panel, textvariable=self.cool_status_var, style="Panel.TLabel").pack(anchor=tk.W, padx=15, pady=1)
        
        # 4. Configurations Stored (Read-Only Status display)
        config_panel = ttk.Frame(right_col, style="Panel.TFrame")
        config_panel.pack(fill=tk.BOTH, expand=True, pady=5)
        
        ttk.Label(config_panel, text=" [4. APPLIED PARAMETERS FROM WEB] ", style="PanelHeader.TLabel").pack(anchor=tk.W, padx=10, pady=8)
        
        self.conf_t1_var = tk.StringVar()
        self.conf_t2_var = tk.StringVar()
        self.conf_cool_var = tk.StringVar()
        self.conf_gpios_var = tk.StringVar()
        
        ttk.Label(config_panel, textvariable=self.conf_t1_var, style="ConfigLabel.TLabel").pack(anchor=tk.W, padx=15, pady=2)
        ttk.Label(config_panel, textvariable=self.conf_t2_var, style="ConfigLabel.TLabel").pack(anchor=tk.W, padx=15, pady=2)
        ttk.Label(config_panel, textvariable=self.conf_cool_var, style="ConfigLabel.TLabel").pack(anchor=tk.W, padx=15, pady=2)
        ttk.Label(config_panel, textvariable=self.conf_gpios_var, style="ConfigLabel.TLabel").pack(anchor=tk.W, padx=15, pady=2)

    # UI Handlers
    def on_float_changed(self):
        state["float_switch_on"] = self.float_var.get()
        # Actual state shift is processed in the 100ms hardware tick timer now
        
    def on_temp_changed(self, val):
        state["cooling_temp_c"] = float(val)
        self.temp_label_var.set(f"DS18B20 Temp (GPIO 33): {state['cooling_temp_c']} °C")
        
    def on_disconnect_changed(self):
        state["ds18b20_disconnected"] = self.disconnect_var.get()
        
    def on_wifi_air_changed(self):
        for ssid, var in self.wifi_vars.items():
            state["wifi_networks_in_air"][ssid] = var.get()
            
    def on_heap_changed(self, val):
        state["free_heap"] = int(val)
        self.heap_label_var.set(f"Simulated Free Heap: {state['free_heap'] // 1024} KB")
        
    def on_cpu_changed(self, event):
        state["cpu_freq_mhz"] = int(self.cpu_var.get())
        
    def on_offline_changed(self):
        state["router_offline"] = self.offline_var.get()

    # Loops
    def update_gui_loop(self):
        # Sync values coming from web APIs back to GUI widgets
        if self.float_var.get() != state["float_switch_on"]:
            self.float_var.set(state["float_switch_on"])
            
        if self.offline_var.get() != state["router_offline"]:
            self.offline_var.set(state["router_offline"])
            
        if self.disconnect_var.get() != state["ds18b20_disconnected"]:
            self.disconnect_var.set(state["ds18b20_disconnected"])

        # Update labels
        self.run_status_var.set(f"Status: {'RUNNING' if state['running'] else 'STOPPED'}" + (" (STABILIZING)" if state["initial_stabilizing"] else ""))
        self.active_timer_var.set(f"Active Timer: {state['active_timer'].upper()}")
        self.phase_var.set(f"Current Phase: {state['phase'].upper()}")
        self.countdown_var.set(f"Countdown: {state['countdown_sec']}s")
        
        # Cooling status string
        cool_str = f"Cooling: Mode={state['cooling_mode'].upper()}, Temp={state['cooling_temp_c'] if not state['ds18b20_disconnected'] else 'FAULT'}°C"
        if state["lockout_active"]:
            cool_str += f" (LOCKOUT: {int(state['lockout_remaining_sec'])}s)"
        elif state["cooling_mode"] == "test_on":
            cool_str += f" (TEST: {int(state['test_remaining_sec'])}s)"
        self.cool_status_var.set(cool_str)
        
        # Update Config values display
        self.conf_t1_var.set(f"Timer 1: ON={state['timer1_on_sec']}s, OFF={state['timer1_off_sec']}s (Start={state['timer1_start_phase'].upper()}) [Polarity={'Active Low' if state['relay1_active_low'] else 'Active High'}]")
        self.conf_t2_var.set(f"Timer 2: ON={state['timer2_on_sec']}s, OFF={state['timer2_off_sec']}s (Start={state['timer2_start_phase'].upper()}) [Polarity={'Active Low' if state['relay2_active_low'] else 'Active High'}]")
        self.conf_cool_var.set(f"Cooling Target: {state['cooling_threshold_c_x10']/10.0}°C ± {state['cooling_hysteresis_c_x10']/10.0}°C (Mode={state['cooling_mode'].upper()})")
        gpios = state["hardware_map"]
        self.conf_gpios_var.set(f"GPIOs: Float={gpios['float_input_gpio']}, P1={gpios['pump_relay1_gpio']}, P2={gpios['pump_relay2_gpio']}, DS18={gpios['ds18b20_gpio']}, Fan={gpios['cooling_relay_gpio']}")
        
        # Update Relay lights
        r1_color = "#34d399" if state["relay1_energized"] else "#374151"
        self.relay1_canvas.itemconfig(self.relay1_oval, fill=r1_color)
        
        r2_color = "#34d399" if state["relay2_energized"] else "#374151"
        self.relay2_canvas.itemconfig(self.relay2_oval, fill=r2_color)
        
        cool_color = "#38bdf8" if state["cooling_relay_state"] else "#374151"
        self.cooling_canvas.itemconfig(self.cooling_oval, fill=cool_color)
        
        # Loop every 200ms
        self.root.after(200, self.update_gui_loop)

    def hardware_tick_loop(self):
        # 100ms interval logic tick
        dt_ms = 100
        
        # Oscillate temp slightly (closed-loop simulation)
        if state["cooling_relay_state"]:
            state["cooling_temp_c"] -= 0.05 * (dt_ms / 1000.0)
        else:
            state["cooling_temp_c"] += 0.02 * (dt_ms / 1000.0)
        state["cooling_temp_c"] = max(24.0, min(35.0, state["cooling_temp_c"]))
        
        # Run C-equivalent simulation loops
        update_pump_simulation(dt_ms)
        update_cooling_simulation(dt_ms)
        
        # Handle simulated router reconnection / disconnect
        if state["router_offline"]:
            state["wifi_connected"] = False
        
        # Loop every 100ms
        self.root.after(100, self.hardware_tick_loop)


def update_pump_simulation(dt_ms):
    global state
    now_ms = int(time.time() * 1000)
    
    # Debounce float switch
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

    # Sensor reading fault simulation
    if state["ds18b20_disconnected"]:
        state["failed_reads"] += 1
        if state["failed_reads"] >= 3:
            state["fault"] = True
            state["sensor_state"] = "fault"
            state["fault_code"] = "read_failed"
    else:
        state["failed_reads"] = 0
        state["fault"] = False
        state["sensor_state"] = "ok"
        state["fault_code"] = "none"

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
    elif state["sensor_state"] == "fault":
        state["cooling_demand"] = False
        state["cooling_auto_demand"] = False
        state["blocked_reason"] = "sensor_fault"
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


# Main Execution Entry
if __name__ == '__main__':
    # Start web server thread
    web_thread = threading.Thread(target=start_web_server, daemon=True)
    web_thread.start()
    
    # Run Tkinter GUI on main thread
    root = tk.Tk()
    app = SimulatorApp(root)
    
    print("-----------------------------------------------------------------")
    print("ESP32 Fish Pump Environment & Resource Simulator (Aligned)")
    print(f"1. Desktop GUI Controller running (Tkinter Window)")
    print(f"2. Mock HTTP Web Server running at: http://localhost:{PORT}")
    print("-----------------------------------------------------------------")
    
    root.mainloop()
