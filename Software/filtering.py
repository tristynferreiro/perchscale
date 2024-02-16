import numpy as np
import matplotlib.pyplot as plt

# Read data from file
with open('readings.txt', 'r') as file:
    data = file.readlines()

# Extract measurements, filtering out readings less than 0 and below upper bound
measurements = []
for line in data:
    sample, value = map(float, line.strip().split(','))
    if 10000 <= value <= 30000:
        measurements.append(value)

x_hat = 24000  # Initial state estimate

def kalman_filter(x_hat,P0, Q, R):
    # Initialize Kalman filter
    P = P0         # Initial error covariance

    predicted_values = []

    # Kalman filter loop
    for z in measurements:
        # Prediction update
        x_hat_minus = x_hat
        P_minus = P + Q

        # Measurement update
        K = P_minus / (P_minus + R)
        x_hat = x_hat_minus + K * (z - x_hat_minus)
        P = (1 - K) * P_minus

        # Save predicted value
        predicted_values.append(x_hat)

    return predicted_values
   

# Q defines how changes impact predicted value
# Larger = closer following, smaller = more averaging
# Define parameter ranges for grid search
P0_range = np.linspace(1, 100000, 10)
Q_range = np.linspace(1e-12, 1e-10, 10)
R_range = np.linspace(1e-2, 100, 10)

best_error = float('inf')
best_params = None

# Grid search
for P0 in P0_range:
    for Q in Q_range:
        for R in R_range:
            predicted_values = kalman_filter(x_hat,P0, Q, R)
            error = np.mean((np.array(measurements) - np.array(predicted_values)) ** 2)
            if error < best_error:
                best_error = error
                best_params = (P0, Q, R)

# Use the best parameters to run the Kalman filter
P0_opt, Q_opt, R_opt = best_params
predicted_values_opt = kalman_filter(x_hat,P0_opt, Q_opt, R_opt)

# with open('filterParams.txt', 'a') as file:
#         file.write(best_params)

# Plot the data and predicted values
plt.plot(range(len(measurements)), measurements, label='Measurements')
plt.plot(range(len(predicted_values_opt)), predicted_values_opt, label='Predicted Values (Optimal)', linestyle='--')
plt.xlabel('Sample Number')
plt.ylabel('Weight')
plt.title('Kalman Filter Prediction with Optimal Parameters')
plt.legend()
plt.grid(True)
plt.show()

print("Optimal Parameters:")
print("P0 =", P0_opt)
print("Q =", Q_opt)
print("R =", R_opt)



# # Extract x and y values
# x_values = []
# y_values = []
# for line in data:
#     sample, value = map(float, line.strip().split(','))
#     x_values.append(sample)
#     y_values.append(value)

# # Plot the data
# plt.plot(x_values, y_values, marker='o', linestyle='-')
# plt.xlabel('Sample Number')
# plt.ylabel('Value')
# plt.title('Sample Data')
# plt.grid(True)
# plt.show()
