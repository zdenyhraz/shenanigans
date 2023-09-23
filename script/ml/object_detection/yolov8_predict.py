from ultralytics import YOLO
import cv2
import matplotlib.pyplot as plt

image = cv2.imread("M:\Work\shenanigans\data\ObjectDetection\cats2.jpg")
image = cv2.resize(image, (640, 640))

# Load a model
# model = YOLO("yolov8n.yaml")  # build a new model from scratch
model = YOLO("yolov8n-seg.pt")  # load a pretrained model (recommended for training)

# Use the model
result = model(image, save=True)[0]  # predict on an image

print("-------------------------------------------------------------------------------------------------")
print(result)

boxes = result.boxes  # Boxes object for bbox outputs
masks = result.masks  # Masks object for segmenation masks outputs
probs = result.probs  # Class probabilities for classification outputs

for box in boxes:
    print("Box: ", box.xywh)

# print("Masks segments: ", masks.segments)
for mask in masks:
    print("Mask: ", mask.data)
    print("Mask size: ", mask.data.size())
    plt.imshow(mask.data.numpy())
    plt.show()
