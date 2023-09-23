
import torch.utils.data
import torch.optim as optim
import torch.nn.functional as F
import torch.nn as nn
import torchvision.datasets as datasets
import torchvision.transforms as transforms
from simple_dataset import ImageClassificationDataset
import matplotlib.pyplot as plt
import skimage.io as io
import numpy as np
import sys
sys.path.append('script/ml')
import log  # nopep8
import train  # nopep8
import plot  # nopep8


class ImageClassificationModel(nn.Module):
    def __init__(self, num_classes):
        super().__init__()
        self.conv1 = nn.Conv2d(1, 6, 5)
        self.conv2 = nn.Conv2d(6, 16, 5)
        self.pool = nn.MaxPool2d(2, 2)
        self.fc1 = nn.Linear(16*125*125, 120)  # for 512x512
        self.fc2 = nn.Linear(120, 84)
        self.fc3 = nn.Linear(84, num_classes)
        self.transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Resize((512, 512), antialias=True),
            transforms.Normalize(np.mean([0.485, 0.456, 0.406]), np.mean([0.229, 0.224, 0.225]))])

    def forward(self, x):
        x = self.pool(F.relu(self.conv1(x)))
        x = self.pool(F.relu(self.conv2(x)))
        x = torch.flatten(x, 1)  # flatten all dimensions except batch
        # log.debug(f"Forward shape flatten: {x.shape}")
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = self.fc3(x)
        return x

    def predict(self, image):
        device = next(self.parameters()).device
        input = torch.unsqueeze(image, 0).to(device)
        output = torch.softmax(torch.squeeze(self.forward(input)), 0)
        return torch.argmax(output), torch.max(output)

    def accuracy(self, dataset, log_predictions=False, classes=None):
        model.eval()
        with torch.no_grad():
            accuracy = 0
            for input, target_class in dataset:
                pred_class, confidence = self.predict(input)
                accuracy += 1 if pred_class == target_class else 0
                if log_predictions and classes:
                    if pred_class == target_class:
                        log.debug(f"[match] '{classes[target_class]}' predicted as '{classes[pred_class]}' with {confidence:.1%} confidence")
                    else:
                        log.warning(f"[mismatch] '{classes[target_class]}' predicted as '{classes[pred_class]}' with {confidence:.1%} confidence")

        return accuracy/len(dataset)

    def predict_plot(self, dataset, num_samples):
        fig, axs = plot.create_fig("sample model predictions", 1, num_samples)
        for idx, dataidx in enumerate(torch.randint(0, len(dataset), (num_samples,))):
            image = dataset[dataidx][0]
            axs[idx].imshow(torch.squeeze(image), interpolation='none', cmap="viridis")
            pred_class, confidence = self.predict(image)
            axs[idx].set_title(f"{dataset.classes[pred_class]} {confidence:.1%}")
        plt.tight_layout()
        plt.show()


if __name__ == "__main__":
    if False:
        dataset = ImageClassificationDataset(root="data/ml/image_classification/datasets/HISAS")
    else:
        dataset = datasets.ImageFolder(root="data/ml/image_classification/datasets/HISAS", loader=lambda path: io.imread(path))
        # torchvision.io.read_image(path, mode=torchvision.io.ImageReadMode.UNCHANGED)
    model = ImageClassificationModel(len(dataset.classes))
    dataset.transform = model.transform
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    use_augment = False
    augment_transform = transforms.Compose([transforms.RandomHorizontalFlip(p=0.5), transforms.RandomResizedCrop(
        size=(512, 512), scale=(0.5, 1.0), ratio=(1, 1), antialias=True), transforms.RandomRotation(180)]) if use_augment else None
    options = train.TrainOptions(num_epochs=20, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam,
                                 learn_rate=5e-4, batch_size=8, test_ratio=0.2, device=device, measure_accuracy=True, augment_transform=augment_transform)
    train.train(model, dataset, options)
    model.predict_plot(dataset, 4)
