# === Import required libraries ===
import serial
import time
import matplotlib.pyplot as plt
from collections import deque

# === SETTINGS ===
PORT = 'COM5'            # Set this to your Arduino COM port
BAUD_RATE = 9600         # Must match Arduino's Serial.begin() baud rate
OUTPUT_FILE = 'data_plot_4.csv'  # Output CSV file name
MAX_POINTS = 50          # Maximum recent points to display in live plot

# === INITIAL SETUP ===

# Open Serial connection
ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Wait 2 seconds for Arduino auto-reset after opening serial

# Create deques to store recent data points (with a fixed max length)
times = deque(maxlen=MAX_POINTS)
temps = deque(maxlen=MAX_POINTS)
humidities = deque(maxlen=MAX_POINTS)
ppms = deque(maxlen=MAX_POINTS)
voltages = deque(maxlen=MAX_POINTS)
currents = deque(maxlen=MAX_POINTS)

# Open the CSV file and write the header
f = open(OUTPUT_FILE, 'w')
f.write("Time (s),Temp (°C),Humidity (%),CO2 (PPM),Volt (V),Current (mA)\n")

# === Prepare Live Plots ===
plt.ion()  # Turn interactive mode ON
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 12))  # 3 subplots stacked

# Subplot 1: Temperature and Humidity
line_temp, = ax1.plot([], [], label='Temp (°C)', color='red')
line_humidity, = ax1.plot([], [], label='Humidity (%)', color='blue')
ax1.set_ylabel('Temp / Humidity')
ax1.grid(True)
ax1.legend(loc='center left', bbox_to_anchor=(1, 0.5))  # Move legend outside

# Subplot 2: CO2 PPM
line_ppm, = ax2.plot([], [], label='CO2 (PPM)', color='orange')
ax2.set_ylabel('CO2 (PPM)')
ax2.grid(True)
ax2.legend(loc='center left', bbox_to_anchor=(1, 0.5))

# Subplot 3: Voltage and Current
line_voltage, = ax3.plot([], [], label='Voltage (V)', color='green')
line_current, = ax3.plot([], [], label='Current (mA)', color='purple')
ax3.set_xlabel('Time (s)')
ax3.set_ylabel('Voltage / Current')
ax3.grid(True)
ax3.legend(loc='center left', bbox_to_anchor=(1, 0.5))

# Adjust layout to prevent overlap
fig.tight_layout()

print("Connected to Arduino. Logging and plotting... Press Ctrl+C to stop.")

# === MAIN LOOP ===
try:
    while True:
        line = ser.readline().decode('utf-8').strip()  # Read a line from serial
        if line:
            print(line)  # Print raw data for monitoring
            f.write(line + '\n')  # Save raw data to CSV
            parts = line.split(',')  # Split incoming line into fields

            if len(parts) == 6:  # Check if line has all expected fields
                cur_time = int(parts[0])
                temp = float(parts[1])
                humidity = float(parts[2])
                ppm = float(parts[3])
                voltage = float(parts[4])
                current = float(parts[5])

                # Append new data to the deques
                times.append(cur_time)
                temps.append(temp)
                humidities.append(humidity)
                ppms.append(ppm)
                voltages.append(voltage)
                currents.append(current)

                # Update live plot data
                line_temp.set_data(times, temps)
                line_humidity.set_data(times, humidities)
                line_ppm.set_data(times, ppms)
                line_voltage.set_data(times, voltages)
                line_current.set_data(times, currents)

                # Adjust axes to fit new data
                ax1.relim()
                ax1.autoscale_view()

                ax2.relim()
                ax2.autoscale_view()

                ax3.relim()
                ax3.autoscale_view()

                plt.pause(0.01)  # Pause to refresh plot

except KeyboardInterrupt:
    print("\nLogging stopped by user.")  # Exit gracefully on Ctrl+C

except Exception as e:
    print(f"Error: {e}")  # Print any other error

finally:
    # === CLEANUP ===
    ser.close()   # Close serial port
    f.close()     # Close CSV file
    print("Serial connection closed. Data file saved.")

    # Save the final combined live plot
    fig.savefig('final_plot.png', dpi=300)
    print("Final combined plot saved as 'final_plot.png'.")

    # === Save Separate Sensor Graphs ===

    # 1. Temperature and Humidity plot
    plt.figure()
    plt.plot(times, temps, label='Temp (°C)', color='red')
    plt.plot(times, humidities, label='Humidity (%)', color='blue')
    plt.xlabel('Time (s)')
    plt.ylabel('Temp / Humidity')
    plt.title('Temperature and Humidity vs Time')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('temp_humidity_plot.png')

    # 2. CO2 PPM plot
    plt.figure()
    plt.plot(times, ppms, label='CO2 (PPM)', color='orange')
    plt.xlabel('Time (s)')
    plt.ylabel('CO2 (PPM)')
    plt.title('CO2 Concentration vs Time')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('co2_plot.png')

    # 3. Voltage and Current plot
    plt.figure()
    plt.plot(times, voltages, label='Voltage (V)', color='green')
    plt.plot(times, currents, label='Current (mA)', color='purple')
    plt.xlabel('Time (s)')
    plt.ylabel('Voltage / Current')
    plt.title('Voltage and Current vs Time')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('voltage_current_plot.png')

    print("Separate sensor plots saved successfully.")
