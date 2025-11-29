import socket
import tkinter as tk
import time

# --------------------------------------------------------------------
# CONFIGURATION
# --------------------------------------------------------------------
# ESP8266 TCP server details
# Change ESP_IP to match the IP printed by your ESP8266 when it connects.
ESP_IP = "192.168.1.7"
PORT = 5050

# --------------------------------------------------------------------
# CONNECT TO ESP8266 VIA TCP
# --------------------------------------------------------------------
# Creates a TCP socket using IPv4 (AF_INET) and TCP (SOCK_STREAM)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to ESP8266 Wi-Fi TCP server
# ESP must be in the same Wi-Fi network (same router) as the PC
s.connect((ESP_IP, PORT))
print("Connected to ESP8266")

# --------------------------------------------------------------------
# VARIABLES TO HOLD SPEED + DIRECTION STATE
# --------------------------------------------------------------------
last_change_time = 0            # Used for "send only after slider stops"
current_value = 0               # Current speed (0–100%)
current_dir = "fwd"             # Current direction ("fwd" or "rev")


# ====================================================================
#  SEND PACKET TO ESP8266 IMMEDIATELY
# ====================================================================
def send_immediate():
    """
    Sends data instantly to the ESP8266 in the format:
        speed,direction
    Examples:
        "40,fwd"
        "75,rev"
        "0,fwd"
    """
    packet = f"{current_value},{current_dir}"
    try:
        s.send(packet.encode())
        print("Sent:", packet)
    except:
        # If ESP disconnects temporarily, script should not crash
        pass


# ====================================================================
#  CALLBACK WHEN SLIDER MOVES
# ====================================================================
def slider_moved(value):
    """
    Triggered every time the user moves the speed slider.
    We only update the number and wait until movement stops
    before sending it to ESP.
    """
    global last_change_time, current_value
    current_value = int(float(value))
    percent_label.config(text=f"{current_value}%")

    # Reset the timer – used in check_and_send()
    last_change_time = time.time()


# ====================================================================
#  SPEED BUTTONS (+ and -)
# ====================================================================
def increase_speed():
    """Increases speed by +1%."""
    global current_value, last_change_time
    if current_value < 100:
        current_value += 1
        slider.set(current_value)
        percent_label.config(text=f"{current_value}%")
        last_change_time = time.time()


def decrease_speed():
    """Decreases speed by -1%."""
    global current_value, last_change_time
    if current_value > 0:
        current_value -= 1
        slider.set(current_value)
        percent_label.config(text=f"{current_value}%")
        last_change_time = time.time()


# ====================================================================
#  START & STOP BUTTONS
# ====================================================================
def stop_motor():
    """
    Stops the motor immediately.
    Sends: "0,fwd" or "0,rev"
    (Direction doesn’t matter since speed is 0.)
    """
    global current_value, last_change_time
    current_value = 0
    slider.set(0)
    percent_label.config(text="0%")

    send_immediate()

    # Prevent further auto-send for a while
    last_change_time = time.time() + 999


def start_motor():
    """
    Convenience button to start motor at 40%.
    """
    global current_value, last_change_time
    current_value = 40
    slider.set(40)
    percent_label.config(text="40%")

    send_immediate()
    last_change_time = time.time() + 999


# ====================================================================
#  DIRECTION BUTTONS
# ====================================================================
def set_forward():
    """
    Sets direction to forward.
    Immediately sends updated packet to ESP.
    """
    global current_dir
    current_dir = "fwd"

    # Update UI button colors
    fwd_btn.config(bg="green", fg="white")
    rev_btn.config(bg="lightgray", fg="black")

    send_immediate()


def set_reverse():
    """
    Sets direction to reverse.
    Immediately sends updated packet to ESP.
    """
    global current_dir
    current_dir = "rev"

    rev_btn.config(bg="red", fg="white")
    fwd_btn.config(bg="lightgray", fg="black")

    send_immediate()


# ====================================================================
#  AUTO-SEND WHEN SLIDER STOPS MOVING
# ====================================================================
def check_and_send():
    """
    Checks if 1 second has passed since last slider movement.
    If YES → send updated speed & direction packet.
    This prevents spamming the ESP with too many packets.
    """
    global last_change_time

    if time.time() - last_change_time >= 1:
        send_immediate()
        last_change_time = time.time() + 999  # Delay next auto-send

    # Re-run every 100ms
    window.after(100, check_and_send)


# ====================================================================
#  GUI (Tkinter)
# ====================================================================
window = tk.Tk()
window.title("Motor Speed Control")

# --- Slider Row ---
frame = tk.Frame(window)
frame.pack(pady=20)

minus_btn = tk.Button(frame, text="-", font=("Arial", 18), width=4, command=decrease_speed)
minus_btn.grid(row=0, column=0, padx=10)

slider = tk.Scale(frame, from_=0, to=100, orient="horizontal",
                  length=300, command=slider_moved)
slider.grid(row=0, column=1)

plus_btn = tk.Button(frame, text="+", font=("Arial", 18), width=4, command=increase_speed)
plus_btn.grid(row=0, column=2, padx=10)

percent_label = tk.Label(window, text="0%", font=("Arial", 20))
percent_label.pack(pady=10)

# --- Start/Stop Buttons ---
start_stop_frame = tk.Frame(window)
start_stop_frame.pack(pady=10)

stop_btn = tk.Button(start_stop_frame, text="STOP", font=("Arial", 16),
                     width=8, bg="red", fg="white",
                     command=stop_motor)
stop_btn.grid(row=0, column=0, padx=20)

start_btn = tk.Button(start_stop_frame, text="START", font=("Arial", 16),
                      width=8, bg="green", fg="white",
                      command=start_motor)
start_btn.grid(row=0, column=1, padx=20)

# --- Direction Buttons ---
dir_frame = tk.Frame(window)
dir_frame.pack(pady=10)

fwd_btn = tk.Button(dir_frame, text="FWD", font=("Arial", 16),
                    width=8, bg="green", fg="white",
                    command=set_forward)
fwd_btn.grid(row=0, column=0, padx=20)

rev_btn = tk.Button(dir_frame, text="REV", font=("Arial", 16),
                    width=8, bg="lightgray", fg="black",
                    command=set_reverse)
rev_btn.grid(row=0, column=1, padx=20)

# Start auto-send system
check_and_send()

window.mainloop()
