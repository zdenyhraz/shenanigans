import torch.nn as nn
import torch.optim as optim
import torch.utils.data
import matplotlib.pyplot as plt
import matplotlib.ticker
import numpy as np
import os
import log
import plot


class TrainOptions:
    def __init__(self, num_epochs=10, learn_rate=5e-4, save_model=False, log_progress=True, plot_progress=True, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam, batch_size=16, test_ratio=0.2, device=torch.device('cuda' if torch.cuda.is_available() else 'cpu'), measure_accuracy=False):
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
        self.measure_accuracy = measure_accuracy


def train(model, dataset, options):
    test_size = int(len(dataset)*options.test_ratio)
    train_size = len(dataset)-test_size
    train_dataset, test_dataset = torch.utils.data.random_split(dataset, [train_size, test_size])
    num_workers = 0  # os.cpu_count()
    train_loader = torch.utils.data.DataLoader(dataset=train_dataset, batch_size=options.batch_size, shuffle=True, num_workers=num_workers)
    test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=options.batch_size, shuffle=False, num_workers=num_workers)
    model.to(options.device)
    optimizer = options.optimizer(model.parameters(), lr=options.learn_rate)

    if options.plot_progress:
        fig, axs = plot.create_fig("model training", 2, 1, aspect_ratio=1.4, sharex=True, position=(100, 100))

    accuracies_train, accuracies_test = [], []
    losses_train, losses_test = [], []
    log.info(f"Model accuracy before training: train_dataset {model.accuracy(train_dataset):.1%}, test_dataset {model.accuracy(test_dataset):.1%}")
    log.info(f"Training started: train_size: {train_size}, test_size: {test_size}, batch_size: {options.batch_size}, device: '{options.device}'")

    for epoch in range(options.num_epochs):
        loss_train, loss_test = train_one_epoch(model, train_loader, test_loader, optimizer, options.criterion, options.device)

        if options.measure_accuracy:
            accuracy_train = model.accuracy(train_dataset)
            accuracy_test = model.accuracy(test_dataset)
            accuracies_train.append(accuracy_train)
            accuracies_test.append(accuracy_test)
        if options.log_progress:
            if options.measure_accuracy:
                log.debug(f"[Epoch {epoch}] train_loss {loss_train:.2e} | test_loss {loss_test:.2e} | train_accuracy {accuracy_train:.1%}, | test_accuracy {accuracy_test:.1%}")
            else:
                log.debug(f"[Epoch {epoch}] train_loss {loss_train:.2e} | test_loss {loss_test:.2e}")
        if options.plot_progress:
            losses_train.append(loss_train)
            losses_test.append(loss_test)
            plot_loss(fig, axs, epoch, losses_train, losses_test, accuracies_train, accuracies_test)

    log.info('Training finished')
    log.info(
        f"Model accuracy after training: train_dataset {model.accuracy(train_dataset):.1%}, test_dataset {model.accuracy(test_dataset, log_predictions=True, classes=dataset.classes):.1%}")

    if options.save_model:
        model_name = "gigachad"
        savepath = f"data/runs/{model_name}.pth"
        log.info(f"Saving model to {savepath}")
        torch.save(model.state_dict(), savepath)
    if options.plot_progress:
        plt.show()


def train_one_epoch(model, train_loader, test_loader, optimizer, criterion, device):
    model.train()
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


def plot_loss(fig, axs, epoch, losses_train, losses_test, accuracies_train, accuracies_test):
    (ax1, ax2) = axs
    plotx = np.arange(0, epoch+1, 1)

    ax1.clear()
    ax1.plot(plotx, losses_train, linewidth=4, label="train loss")
    ax1.plot(plotx, losses_test, linewidth=4, label="test loss")
    ax1.set_ylabel("loss")
    ax1.set_yscale("log")
    ax1.legend()

    ax2.clear()
    ax2.plot(plotx, accuracies_train, linewidth=4, label="train accuracy")
    ax2.plot(plotx, accuracies_test, linewidth=4, label="test accuracy")
    ax2.set_xlabel("epoch")
    ax2.set_ylabel("accuracy")
    ax2.legend()
    ax2.yaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(xmax=1.0))

    plt.tight_layout()
    plt.draw()
    plt.pause(1e-4)
