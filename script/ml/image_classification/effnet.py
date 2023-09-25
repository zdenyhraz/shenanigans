import torch
import torch.nn as nn
import torch.optim as optim
import torchvision.models as models
import torchvision.datasets as datasets
import torchvision.transforms as transforms
import skimage.io as io
import numpy as np
from predict import predict_plot
from accuracy import accuracy
import sys
sys.path.append('script/ml')
import train  # nopep8

if __name__ == "__main__":
    weights = models.EfficientNet_B0_Weights.DEFAULT
    transform = weights.transforms()
    print(f"Model transform: {transform}")
    model = models.efficientnet_b0(weights=weights)
    dataset = datasets.ImageFolder(root="data/ml/image_classification/datasets/HISAS", loader=lambda path: io.imread(path), transform=transforms.Compose([
        transforms.ToTensor(),
        transforms.Resize((224, 224), antialias=True),
        transforms.Lambda(lambda x: x.repeat(3, 1, 1)),
        transform
    ]))

    # freeze
    for param in model.parameters():
        param.requires_grad = False

    # replace classification layer
    model.classifier = nn.Sequential(nn.Dropout(p=0.2, inplace=True), nn.Linear(1280, len(dataset.classes)))

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    batch_size = int(np.clip(0.05*len(dataset), 1, 32))
    options = train.TrainOptions(num_epochs=50, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam,
                                 learn_rate=1e-3, acc_metric=accuracy, batch_size=batch_size, test_ratio=0.2, device=device)
    train.train(model, dataset, options)
    predict_plot(model, dataset, 4)
