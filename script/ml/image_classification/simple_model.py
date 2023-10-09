
import predict
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
        self.fc1 = nn.Linear(16*53*53, 120)
        self.fc2 = nn.Linear(120, 84)
        self.fc3 = nn.Linear(84, num_classes)
        self.transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Resize((224, 224), antialias=True),
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
    if False:
        dataset = datasets.ImageFolder(root="data/ml/image_classification/datasets/HISAS", loader=lambda path: io.imread(path))
        # torchvision.io.read_image(path, mode=torchvision.io.ImageReadMode.UNCHANGED)
    else:
        dataset = datasets.MNIST(root="data/ml/image_classification/datasets", download=True)

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model = ImageClassificationModel(len(dataset.classes)).to(device)
    use_augment = False
    augment_transform = transforms.Compose([transforms.RandomHorizontalFlip(p=0.5), transforms.RandomResizedCrop(
        size=(128, 128), scale=(0.7, 1.0), ratio=(1, 1), antialias=True), transforms.RandomRotation(180)]) if use_augment else None
    dataset.transform = transforms.Compose([model.transform, augment_transform]) if augment_transform else model.transform
    test_ratio = 0.2
    train_dataset, test_dataset = torch.utils.data.random_split(dataset, [(1-test_ratio), test_ratio])
    batch_size = train.get_batch_size(train_dataset)
    num_epochs = train.get_num_epochs(train_dataset, batch_size, 500)
    num_workers = 0  # 0 = os.cpu_count()
    train_loader = torch.utils.data.DataLoader(dataset=train_dataset, batch_size=batch_size, shuffle=True, num_workers=num_workers)
    test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=batch_size, shuffle=False, num_workers=num_workers)
    options = train.TrainOptions(num_epochs=num_epochs, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam,
                                 learn_rate=1e-3, acc_metric=accuracy, device=device, plot_progress=True)

    train.train(model, train_loader, test_loader, options)
    if (False and len(test_dataset) <= 100):
        predict.predict_all(model, test_dataset)
    predict.predict_plot(model, test_dataset, 4)  # sample predictions on test dataset
