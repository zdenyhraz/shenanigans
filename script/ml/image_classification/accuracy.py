import torch


def accuracy(pred, target):
    with torch.no_grad():
        acc = (torch.argmax(pred, 1) == target).sum().item()/pred.size(0)
        if acc > 1:
            print(f"Watafak {acc}>1:\npred:\n{pred}, target:\n{target}")
        return acc
