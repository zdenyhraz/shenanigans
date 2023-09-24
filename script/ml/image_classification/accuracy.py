import torch


def accuracy(model, dataloader):
    model.eval()
    device = next(model.parameters()).device
    with torch.no_grad():
        accuracy = 0
        for x, y in dataloader:
            x = x.to(device)
            y = y.to(device)
            pred = torch.argmax(model.forward(x), 1)
            accuracy += (pred == y).sum().item()

    return accuracy/len(dataloader.dataset)
