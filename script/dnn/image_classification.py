import train
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import torch.utils.data
import os.path
from pathlib import Path
import cv2
import skimage.io as io
import torchvision.transforms as transforms


class ImageClassificationModel(nn.Module):
    def __init__(self, num_classes):
        super().__init__()
        self.conv1 = nn.Conv2d(1, 6, 5)
        self.conv2 = nn.Conv2d(6, 16, 5)
        self.pool = nn.MaxPool2d(2, 2)
        self.fc1 = nn.Linear(16*125*125, 120)  # for 512x512
        self.fc2 = nn.Linear(120, 84)
        self.fc3 = nn.Linear(84, num_classes)

    def forward(self, x):
        x = self.pool(F.relu(self.conv1(x)))
        x = self.pool(F.relu(self.conv2(x)))
        x = torch.flatten(x, 1)  # flatten all dimensions except batch
        # print(f"Forward shape flatten: {x.shape}")
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = self.fc3(x)
        return x


class ImageClassificationDataset(torch.utils.data.Dataset):
    def __init__(self, dataset_directory, image_size):
        print(f"Loading ImageClassificationDataset from {dataset_directory}")
        self.samples = []
        self.class_names = []
        self.transform = transforms.ToTensor()
        self.image_size = image_size
        if not os.path.isdir(dataset_directory):
            raise ValueError(f"'{dataset_directory}' is not a directory")

        for class_directory in os.listdir(dataset_directory):
            class_directory = os.path.join(dataset_directory, class_directory)
            if not os.path.isdir(class_directory):
                continue
            class_name = Path(class_directory).stem
            self.class_names.append(class_name)
            # print(f"Registering class '{class_name}' samples")

            for sample in os.listdir(class_directory):
                sample = os.path.join(class_directory, sample)
                if not os.path.isfile(sample):
                    continue
                self.samples.append((sample, class_name))
                # print(f"Registered class '{self.samples[-1][1]}' sample '{self.samples[-1][0]}'")

        self.num_classes = len(self.class_names)
        for class_name in self.class_names:
            num_samples = sum(1 for sample in self.samples if sample[1] == class_name)
            print(f"Class '{class_name}' samples: {num_samples}")

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, index):
        image = io.imread(self.samples[index][0])
        image = cv2.resize(image, (self.image_size, self.image_size))
        if self.transform:
            image = self.transform(image)
        label = torch.tensor(int(self.class_names.index(self.samples[index][1])))
        return image, label


if __name__ == "__main__":
    dataset = ImageClassificationDataset("data/DNN/objdetect/HISAS", image_size=512)
    model = ImageClassificationModel(dataset.num_classes)
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    options = train.TrainOptions(num_epochs=50, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam, learn_rate=0.0001, batch_size=16, test_ratio=0.2, device=device)
    train.train(model, dataset, options)
