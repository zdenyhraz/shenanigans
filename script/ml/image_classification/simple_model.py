
from predict import predict_plot
import torch.utils.data
import torch.optim as optim
import torch.nn.functional as F
import torch.nn as nn
import torchvision.datasets as datasets
import torchvision.transforms as transforms
from simple_dataset import ImageClassificationDataset
from accuracy import accuracy
import matplotlib.pyplot as plt
import skimage.io as io
import numpy as np
import sys
sys.path.append('script/ml')
import train  # nopep8


class ImageClassificationModel(nn.Module):
    def __init__(self, num_classes):
        super().__init__()
        self.conv1 = nn.Conv2d(1, 6, 5)
        self.conv2 = nn.Conv2d(6, 16, 5)
        self.pool = nn.MaxPool2d(2, 2)
        self.fc1 = nn.Linear(16*29*29, 120)
        self.fc2 = nn.Linear(120, 84)
        self.fc3 = nn.Linear(84, num_classes)
        self.transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Resize((128, 128), antialias=True),
            transforms.Normalize(np.mean([0.485, 0.456, 0.406]), np.mean([0.229, 0.224, 0.225]))
        ])

    def forward(self, x):
        x = self.pool(F.relu(self.conv1(x)))
        x = self.pool(F.relu(self.conv2(x)))
        x = torch.flatten(x, 1)  # flatten all dimensions except batch
        # log.debug(f"Forward shape flatten: {x.shape}")
        x = F.dropout(F.relu(self.fc1(x)), p=0.0, training=self.training)
        x = F.dropout(F.relu(self.fc2(x)), p=0.0, training=self.training)
        x = self.fc3(x)
        return x


if __name__ == "__main__":
    if True:
        dataset = datasets.ImageFolder(root="data/ml/image_classification/datasets/HISAS", loader=lambda path: io.imread(path))
        # torchvision.io.read_image(path, mode=torchvision.io.ImageReadMode.UNCHANGED)
    else:
        dataset = datasets.MNIST(root="data/ml/image_classification/datasets", download=True)

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model = ImageClassificationModel(len(dataset.classes)).to(device)
    dataset.transform = model.transform
    use_augment = False
    augment_transform = transforms.Compose([transforms.RandomHorizontalFlip(p=0.5), transforms.RandomResizedCrop(
        size=(128, 128), scale=(0.7, 1.0), ratio=(1, 1), antialias=True), transforms.RandomRotation(180)]) if use_augment else None
    batch_size = int(np.clip(0.05*len(dataset), 1, 32))
    options = train.TrainOptions(num_epochs=30, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam,
                                 learn_rate=1e-3, batch_size=batch_size, test_ratio=0.2, device=device, log_progress=True, augment_transform=augment_transform)
    train.train(model, dataset, options)
    predict_plot(model, dataset, 4)
