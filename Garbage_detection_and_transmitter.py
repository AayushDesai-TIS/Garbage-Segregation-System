import numpy as np
from PIL import Image
import cv2
import socket
from tflite_runtime.interpreter import Interpreter

UDP_IP = "192.168.1.107"  # Replace with the IP address of your ESP32
UDP_PORT = 8080

# Load the TFLite model
interpreter = Interpreter(model_path='/home/pi/Downloads/model_unquant.tflite')
interpreter.allocate_tensors()

# Get input and output tensors
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Load labels
with open('/home/pi/Downloads/labels.txt', 'r') as file:
    labels = [line.strip() for line in file.readlines()]

# Initialize video capture
cap = cv2.VideoCapture(0)  # Use 0 for the default webcam

if not cap.isOpened():
    print("Error: Could not open video capture.")
    exit()

# Set the desired frame width and height
frame_width = 640  # Desired width
frame_height = 480  # Desired height

cap.set(cv2.CAP_PROP_FRAME_WIDTH, frame_width)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, frame_height)

# Initialize label detection counters
label_detection_counter = {label: 0 for label in labels}
last_detected_label = None
message_sent = False

while True:
    # Capture frame-by-frame
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read frame.")
        break

    # Rotate the frame by 90 degrees clockwise
    frame = cv2.transpose(frame)  # Transpose the frame
    frame = cv2.flip(frame, flipCode=0)  # Flip horizontally

    # Convert frame to PIL Image and preprocess
    image = Image.fromarray(frame).resize((224, 224))  # Adjust size as needed
    input_data = np.array(image, dtype=np.float32)

    # Normalize the input data if required
    input_data = (input_data / 255.0)  # Example normalization

    # Convert to the model's expected format (BGR to RGB, if needed)
    if input_data.shape[-1] == 3:  # Ensure RGB format
        input_data = input_data[..., ::-1]

    # Add batch dimension
    input_data = np.expand_dims(input_data, axis=0)

    # Set input tensor
    interpreter.set_tensor(input_details[0]['index'], input_data)

    # Run inference
    interpreter.invoke()

    # Get output tensor
    output_data = interpreter.get_tensor(output_details[0]['index'])

    # Get the predicted class and its probability
    probabilities = output_data[0]
    predicted_class = np.argmax(probabilities)
    predicted_prob = probabilities[predicted_class]

    # Detect label if probability is above 80%
    detected_label = labels[predicted_class] if predicted_prob > 0.70 else None

    if detected_label:
        if detected_label == last_detected_label:
            # Increment counter if the same label is detected
            label_detection_counter[detected_label] += 1
        else:
            # Reset counter for the previous label
            if last_detected_label:
                label_detection_counter[last_detected_label] = 0
            
            # Update to the new label
            last_detected_label = detected_label
            label_detection_counter[detected_label] = 1  # Start counting for the new label

        # Check if the current label count has reached 10
        if label_detection_counter[detected_label] >= 10:
            if not message_sent:
                print(f'Detected: {detected_label} with probability {predicted_prob:.2f}')
                print(f'Count for {detected_label}: {label_detection_counter[detected_label]}')
                MESSAGE = detected_label
                sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                sock.sendto(MESSAGE.encode(), (UDP_IP, UDP_PORT))
                message_sent = True
        else:
            # Print the count for the detected label if it is less than 10
            print(f'Count for {detected_label}: {label_detection_counter[detected_label]}')
    else:
        # Reset the state if no significant detection
        if last_detected_label:
            label_detection_counter[last_detected_label] = 0
        last_detected_label = None
        message_sent = False

    # Display the result on the frame if a label is detected
    if detected_label:
        cv2.putText(frame, f'Class: {detected_label} ({predicted_prob:.2f})', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2, cv2.LINE_AA)

    # Display the resulting frame
    cv2.imshow('Live Feed', frame)

    # Break the loop if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the capture and close windows
cap.release()
cv2.destroyAllWindows()
