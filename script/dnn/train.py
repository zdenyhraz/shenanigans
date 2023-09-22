import torch.nn as nn
import torch.optim as optim
import torch.utils.data
import matplotlib.pyplot as plt
import numpy as np
import log


class TrainOptions:
    def __init__(self, num_epochs=10, learn_rate=5e-1, save_model=False, log_progress=True, plot_progress=True, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam, batch_size=16, test_ratio=0.2, device=torch.device('cuda' if torch.cuda.is_available() else 'cpu')):
        self.num_epochs = num_epochs
        self.learn_rate = learn_rate
        self.save_model = save_model
        self.log_progress = log_progress
        self.plot_progress = plot_progress
        self.criterion = criterion
        self.optimizer = optimizer
        self.batch_size = batch_size
        self.test_ratio = test_ratio
        self.device = device


def train(model, dataset, options):
    test_size = int(len(dataset)*options.test_ratio)
    train_size = len(dataset)-test_size
    train_dataset, test_dataset = torch.utils.data.random_split(dataset, [train_size, test_size])
    train_loader = torch.utils.data.DataLoader(dataset=train_dataset, batch_size=options.batch_size, shuffle=True)
    test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=options.batch_size, shuffle=True)
    model.to(options.device)
    model.train()
    optimizer = options.optimizer(model.parameters(), lr=options.learn_rate)

    log.info(f"Model accuracy before training: train_dataset {model.accuracy(train_dataset):.1%}, test_dataset {model.accuracy(test_dataset):.1%}")
    log.info(f"Training started: train_size: {train_size}, test_size: {test_size}, batch_size: {options.batch_size}, device: '{options.device}'")
    losses_train, losses_test = [], []
    for epoch in range(options.num_epochs):
        loss_train, loss_test = train_one_epoch(model, train_loader, test_loader, optimizer, options.criterion, options.device)

        if options.log_progress:
            log.debug(f"[Epoch {epoch}] train_loss {loss_train:.2e} | test_loss {loss_test:.2e}")
        if options.plot_progress:
            losses_train.append(loss_train)
            losses_test.append(loss_test)
            plot_train(epoch, losses_train, losses_test)

    log.info('Training finished')
    log.info(
        f"Model accuracy after training: train_dataset {model.accuracy(train_dataset):.1%}, test_dataset {model.accuracy(test_dataset, log_predictions=True, class_names=dataset.class_names):.1%}")

    if options.save_model:
        torch.save(model.state_dict(), "model.pth")
    if options.plot_progress:
        plt.show()


def train_one_epoch(model, train_loader, test_loader, optimizer, criterion, device):
    loss_train = 0.0
    for inputs, targets in train_loader:
        inputs = inputs.to(device)
        targets = targets.to(device)
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = criterion(outputs, targets)
        loss.backward()
        optimizer.step()
        loss_train += loss.item()*inputs.size(0)

    with torch.no_grad():
        loss_test = 0.0
        for inputs, targets in test_loader:
            inputs = inputs.to(device)
            targets = targets.to(device)
            outputs = model(inputs)
            loss = criterion(outputs, targets)
            loss_test += loss.item()*inputs.size(0)

    return loss_train/len(train_loader.dataset), loss_test/len(test_loader.dataset)  # sample-average loss


def plot_train(epoch, losses_train, losses_test):
    plt.clf()
    plotx = np.arange(0, epoch+1, 1)
    plt.plot(plotx, losses_train, linewidth=3, color="blue", label="train loss")
    plt.plot(plotx, losses_test, linewidth=3, color="green", label="test loss")
    # plt.xticks(plotx)
    plt.xlabel("epoch")
    plt.ylabel("loss")
    plt.yscale("log")
    plt.legend()
    plt.draw()
    plt.pause(1e-4)
