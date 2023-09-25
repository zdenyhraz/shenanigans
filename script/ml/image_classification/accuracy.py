import torch
import torchmetrics


def accuracy(model, dataloader):
    model.eval()
    device = next(model.parameters()).device
    accuracy = 0
    with torch.no_grad():
        for x, y in dataloader:
            x = x.to(device)
            y = y.to(device)
            pred = torch.argmax(model.forward(x), 1)
            accuracy += (pred == y).sum().item()

    return accuracy/len(dataloader.dataset)


def accuracy_metric(model, dataloader):
    model.eval()
    device = next(model.parameters()).device
    acc_fn = torchmetrics.Accuracy(task="multiclass", num_classes=len(dataloader.dataset.dataset.classes)).to(device)
    accuracy = 0
    with torch.no_grad():
        for x, y in dataloader:
            x = x.to(device)
            y = y.to(device)
            accuracy += acc_fn(torch.argmax(model.forward(x), 1), y).item()*x.size(0)

    return accuracy/len(dataloader.dataset)
