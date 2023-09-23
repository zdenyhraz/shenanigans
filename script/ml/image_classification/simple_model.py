
import torch.utils.data
import torch.optim as optim
import torch.nn.functional as F
import torch.nn as nn
import torchvision.datasets as datasets
import torchvision.transforms as transforms
from simple_dataset import ImageClassificationDataset
import skimage.io as io
import sys
sys.path.append('script/ml')  # nopep8
import log  # nopep8
import train  # nopep8


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
        # log.debug(f"Forward shape flatten: {x.shape}")
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = self.fc3(x)
        return x

    def accuracy(self, dataset, log_predictions=False, classes=None):
        model.eval()
        with torch.no_grad():
            accuracy = 0
            for input, target_class in dataset:
                device = next(self.parameters()).device
                input = torch.unsqueeze(input, 0).to(device)
                pred_class = torch.argmax(self.forward(input))
                accuracy += 1 if pred_class == target_class else 0
                if log_predictions and classes:
                    if pred_class == target_class:
                        log.debug(f"[match] '{classes[target_class]}' predicted as '{classes[pred_class]}'")
                    else:
                        log.warning(f"[mismatch] '{classes[target_class]}' predicted as '{classes[pred_class]}'")

        return accuracy/len(dataset)


if __name__ == "__main__":
    if False:
        dataset = ImageClassificationDataset("data/ml/image_classification/datasets/HISAS", transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Resize((512, 512), antialias=True)
        ]))
    else:
        dataset = datasets.ImageFolder(root="data/ml/image_classification/datasets/HISAS", transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Resize((512, 512), antialias=True)
        ]), loader=lambda path: io.imread(path)
        )

    model = ImageClassificationModel(len(dataset.classes))
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    options = train.TrainOptions(num_epochs=50, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam,
                                 learn_rate=0.0005, batch_size=8, test_ratio=0.1, device=device, measure_accuracy=True)
    train.train(model, dataset, options)
