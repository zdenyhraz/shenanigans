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
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model = models.vit_b_16(weights="DEFAULT")
    dataset = datasets.ImageFolder(root="data/ml/image_classification/datasets/HISAS", loader=lambda path: io.imread(path), transform=transforms.Compose([
        transforms.ToTensor(),
        transforms.Resize((224, 224), antialias=True),
        transforms.Lambda(lambda x: x.repeat(3, 1, 1)),
        transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
    ]))

    # freeze
    for param in model.parameters():
        param.requires_grad = False

    # replace classification layer
    model.heads = nn.Linear(768, len(dataset.classes))
    model.to(device)
    print(model)

    batch_size = int(np.clip(0.05*len(dataset), 1, 32))
    options = train.TrainOptions(num_epochs=30, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam,
                                 learn_rate=1e-3, accuracy_fn=accuracy, batch_size=batch_size, test_ratio=0.2, device=device)
    train.train(model, dataset, options)
    predict_plot(model, dataset, 4)
