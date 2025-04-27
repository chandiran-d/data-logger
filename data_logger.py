import serial
import time
import matplotlib.pyplot as plt
from collections import deque

# === SETTINGS ===
PORT = 'COM5'           # CHANGE this to your Arduino port
BAUD_RATE = 9600
OUTPUT_FILE = 'data_plot_4.csv'
MAX_POINTS = 50         # How many recent points to show in live graph

# === INITIAL SETUP ===
ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Wait for Arduino to reset

# Prepare data containers
times = deque(maxlen=MAX_POINTS)
temps = deque(maxlen=MAX_POINTS)
humidities = deque(maxlen=MAX_POINTS)
ppms = deque(maxlen=MAX_POINTS)
voltages = deque(maxlen=MAX_POINTS)
currents = deque(maxlen=MAX_POINTS)

# Prepare CSV file
f = open(OUTPUT_FILE, 'w')
f.write("Time (s),Temp (°C),Humidity (%),CO2 (PPM),Volt (V),Current (mA)\n")  # CSV header

# Prepare plot
plt.ion()
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 12))

# Subplot 1: Temperature and Humidity
line_temp, = ax1.plot([], [], label='Temp (°C)', color='red')
line_humidity, = ax1.plot([], [], label='Humidity (%)', color='blue')
ax1.set_ylabel('Temp / Humidity')
ax1.grid(True)
ax1.legend(loc='center left', bbox_to_anchor=(1, 0.5))  # Legend outside

# Subplot 2: CO2
line_ppm, = ax2.plot([], [], label='CO2 (PPM)', color='orange')
ax2.set_ylabel('CO2 (PPM)')
ax2.grid(True)
ax2.legend(loc='center left', bbox_to_anchor=(1, 0.5))  # Legend outside

# Subplot 3: Voltage and Current
line_voltage, = ax3.plot([], [], label='Voltage (V)', color='green')
line_current, = ax3.plot([], [], label='Current (mA)', color='purple')
ax3.set_xlabel('Time (s)')
ax3.set_ylabel('Voltage / Current')
ax3.grid(True)
ax3.legend(loc='center left', bbox_to_anchor=(1, 0.5))  # Legend outside

fig.tight_layout()

print("Connected to Arduino. Logging and plotting... Press Ctrl+C to stop.")

try:
    while True:
        line = ser.readline().decode('utf-8').strip()
        if line:
            print(line)
            f.write(line + '\n')
            parts = line.split(',')

            if len(parts) == 6:
                cur_time = int(parts[0])
                temp = float(parts[1])
                humidity = float(parts[2])
                ppm = float(parts[3])
                voltage = float(parts[4])
                current = float(parts[5])

                # Update data
                times.append(cur_time)
                temps.append(temp)
                humidities.append(humidity)
                ppms.append(ppm)
                voltages.append(voltage)
                currents.append(current)

                # Update plots
                line_temp.set_data(times, temps)
                line_humidity.set_data(times, humidities)
                line_ppm.set_data(times, ppms)
                line_voltage.set_data(times, voltages)
                line_current.set_data(times, currents)

                ax1.relim()
                ax1.autoscale_view()

                ax2.relim()
                ax2.autoscale_view()

                ax3.relim()
                ax3.autoscale_view()

                plt.pause(0.01)

except KeyboardInterrupt:
    print("\nLogging stopped by user.")

except Exception as e:
    print(f"Error: {e}")

finally:
    # === Close everything ===
    ser.close()
    f.close()
    print("Serial connection closed. Data file saved.")

    # Save final combined plot
    fig.savefig('final_plot.png', dpi=300)
    print("Final combined plot saved as 'final_plot.png'.")

    # === Save Separate Sensor Plots ===
    # Temperature and Humidity Plot
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

    # CO2 PPM Plot
    plt.figure()
    plt.plot(times, ppms, label='CO2 (PPM)', color='orange')
    plt.xlabel('Time (s)')
    plt.ylabel('CO2 (PPM)')
    plt.title('CO2 Concentration vs Time')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig('co2_plot.png')

    # Voltage and Current Plot
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
