import torch
import torch.nn as nn
import torch.optim as optim
import torchvision.models as models
import torchvision.datasets as datasets
import torchvision.transforms as transforms
import skimage.io as io
import numpy as np
import predict
from accuracy import accuracy
import sys
sys.path.append('script/ml')
import train  # nopep8

if __name__ == "__main__":
    weights = models.EfficientNet_B0_Weights.DEFAULT
    print(f"Model transform: {weights.transforms()}")
    model = models.efficientnet_b0(weights=weights)
    if False:
        dataset = datasets.ImageFolder(root="data/ml/image_classification/datasets/HISAS", loader=lambda path: io.imread(path), transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Lambda(lambda x: x.repeat(3, 1, 1)),
            weights.transforms()  # resize, normalize, etc
        ]))
    else:
        dataset = datasets.MNIST(root="data/ml/image_classification/datasets", download=True, transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Lambda(lambda x: x.repeat(3, 1, 1)),
            weights.transforms()  # resize, normalize, etc
        ]))

    # freeze
    freeze = True
    if freeze:
        for param in model.parameters():
            param.requires_grad = False

    # replace classification layer
    model.classifier = nn.Sequential(nn.Dropout(p=0.2, inplace=True), nn.Linear(1280, len(dataset.classes)))

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    test_ratio = 0.2
    train_dataset, test_dataset = torch.utils.data.random_split(dataset, [(1-test_ratio), test_ratio])
    batch_size = train.get_batch_size(train_dataset)
    num_epochs = train.get_num_epochs(train_dataset, batch_size, 500)
    num_workers = 0  # 0 = os.cpu_count()
    train_loader = torch.utils.data.DataLoader(dataset=train_dataset, batch_size=batch_size, shuffle=True, num_workers=num_workers)
    test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=batch_size, shuffle=False, num_workers=num_workers)
    options = train.TrainOptions(num_epochs=num_epochs, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam,
                                 learn_rate=1e-3, acc_metric=accuracy, device=device, plot_progress=False)

    train.train(model, train_loader, test_loader, options)
    predict.predict_plot(model, test_dataset, 4)  # sample predictions on test dataset
