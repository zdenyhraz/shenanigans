import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import torch.utils.data
import train


class ImageClassificationModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv2d(3, 6, 5)
        self.pool = nn.MaxPool2d(2, 2)
        self.conv2 = nn.Conv2d(6, 16, 5)
        self.fc1 = nn.Linear(16 * 5 * 5, 120)
        self.fc2 = nn.Linear(120, 84)
        self.fc3 = nn.Linear(84, 10)

    def forward(self, x):
        x = self.pool(F.relu(self.conv1(x)))
        x = self.pool(F.relu(self.conv2(x)))
        x = F.flatten(x, 1)  # flatten all dimensions except batch
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = self.fc3(x)
        return x


class ImageClassificationDataset(torch.utils.data.Dataset):
    def __init__(self, path):
        return

    def __len__(self):
        return

    def __getitem__(self, index):
        return


model = ImageClassificationModel()
dataset = ImageClassificationDataset("../../data/DNN/objdetect/HISAS")
train_options = train.TrainOptions(num_epochs=10, learn_rate=5e-1, save_model=False, log_progress=True, plot_progress=True,
                                   criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam, batch_size=16, test_ratio=0.2)
train.train(model, dataset, train_options)
