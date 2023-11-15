import torch


def accuracy(pred, target):
    with torch.no_grad():
        return (torch.argmax(pred, 1) == target).sum().item()/pred.size(0)
