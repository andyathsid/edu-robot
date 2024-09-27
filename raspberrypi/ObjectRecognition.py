import cv2
import pyttsx3
import time
import serial

engine = pyttsx3.init()
# voices = engine.getProperty('voices')
# engine.setProperty('voice', voices[1].id)

ser = serial.Serial('/dev/ttyCH341USB0', 9600)

config_file = './/asset//ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt'
frozen_model = './/asset//frozen_inference_graph.pb'

model = cv2.dnn_DetectionModel(frozen_model, config_file)

classlabel = []
file_name = './/asset//coco.names'
with open(file_name, 'rt') as f:
    classlabel = f.read().rstrip('\n').split('\n')

print(classlabel)

model.setInputSize(320, 320)
model.setInputScale(1.0 / 127.5)
model.setInputMean((127.5, 127.5, 127.5))
model.setInputSwapRB(True)

webcam = 0
cap = cv2.VideoCapture(webcam)

if not cap.isOpened():
    cap = cv2.VideoCapture(webcam)

if not cap.isOpened():
    raise IOError("Cannot open webcam")

font_scale = 1.5
font = cv2.FONT_HERSHEY_PLAIN

prev_frame_time = 0
new_frame_time = 0
fps = 0
frame_count = 0

def class_to_character(class_index):
    if class_index in [43, 46]:
        return 'a'
    elif class_index in [26, 30]:
        return 'b'
    elif class_index in [61, 62]:
        return 'c'
    elif class_index in [73, 75]:
        return 'd'
    else:
        return 'z'  

while True:
    ret, frame = cap.read()
    frame_count += 1 

    # fps_end_time = time.time()
    # time_diff = fps_end_time - fps_start_time
    # fps = frame_count / time_diff
    new_frame_time = time.time()
    fps = 1/(new_frame_time-prev_frame_time)
    prev_frame_time = new_frame_time

    class_index, confidence, bbox = model.detect(frame, confThreshold=0.5)

    if len(class_index) != 0:
        max_confidence = 0
        best_class_label = None
        best_confidence = 0


        for classIND, conf, boxes in zip(class_index.flatten(), confidence.flatten(), bbox):
            if conf > max_confidence:
                max_confidence = conf
                best_class_label = classlabel[classIND - 1]
                best_class_index = classIND - 1  
                best_confidence = conf

            if (classIND <= len(classlabel)):
                cv2.rectangle(frame, boxes, (255, 0, 0), 2)
                cv2.putText(frame, f'{classlabel[classIND - 1]}: {conf:.2f}', (boxes[0] + 10, boxes[1] + 40), font, font_scale, color=(0, 255, 0))

        if frame_count % 30 == 0:
            if best_class_label is not None:
                character_to_send = class_to_character(best_class_index)
                print(f'Sending to Arduino: {character_to_send}')
                ser.write(character_to_send.encode())
                
                engine.say(best_class_label)
                engine.runAndWait()

    cv2.putText(frame, f'FPS: {fps:.2f}', (10, 50), font, font_scale, (0, 255, 255), 2)
    # if best_class_label is not None:
    #     cv2.putText(frame, f'Best Class: {best_class_label} ({best_confidence:.2f})', (10, 100), font, font_scale, (255, 255, 0), 2)

    cv2.imshow('Object Detection', frame)

    if cv2.waitKey(2) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
