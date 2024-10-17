from torchvision.io.image import read_image
import torchvision.models.detection
from torchvision.utils import draw_bounding_boxes
import matplotlib.pyplot as plt
import sys
sys.path.append('script/ml')
import plot  # nopep8

img = read_image("data/ml/object_detection/datasets/cats/cats2.jpg")

if True:  # RCNN
    weights = torchvision.models.detection.FasterRCNN_ResNet50_FPN_V2_Weights.DEFAULT  # 91 classes
    model = torchvision.models.detection.fasterrcnn_resnet50_fpn_v2(weights=weights, box_score_thresh=0.4)  # , num_classes=7
elif True:  # RetinaNet
    weights = torchvision.models.detection.RetinaNet_ResNet50_FPN_V2_Weights.DEFAULT
    model = torchvision.models.detection.retinanet_resnet50_fpn_v2(eights=weights, box_score_thresh=0.1)
elif True:  # SSD
    weights = torchvision.models.detection.SSD300_VGG16_Weights.DEFAULT
    model = torchvision.models.detection.ssd300_vgg16(eights=weights, box_score_thresh=0.1)

model.eval()
print(model)

# Step 2: Initialize the inference transforms
preprocess = weights.transforms()
print(f"Preprocess: {preprocess}")

# Step 3: Apply inference preprocessing transforms
batch = [preprocess(img)]

# Step 4: Use the model and visualize the prediction
prediction = model(batch)[0]
print(f"Scores:{prediction['scores']}")
score_threshold = 0.1
labels = [weights.meta["categories"][i]
          for i in prediction["labels"][prediction['scores'] > score_threshold]]
box = draw_bounding_boxes(img, boxes=prediction["boxes"][prediction['scores'] > score_threshold],
                          labels=labels,
                          colors="red",
                          width=4,
                          font="data/apps/CascadiaCode.ttf",
                          font_size=int(img.size()[1]/224*10))

plot.create_fig("sample model predictions", sizeMultiplier=2)
plt.imshow(box.permute(1, 2, 0), interpolation='none')
plt.tight_layout()
plt.show()
