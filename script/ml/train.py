import torch.nn as nn
import torch.optim as optim
import torch.utils.data
import matplotlib.pyplot as plt
import matplotlib.ticker
import numpy as np
import torchvision.transforms as transforms
import torchsummary
from tqdm import tqdm
import copy
import log
import plot

bar_format = "{desc}{percentage:3.0f}%|{bar:20}| {n_fmt}/{total_fmt} [{elapsed}<{remaining},{rate_inv_fmt}{postfix}]"


class TrainOptions:
    def __init__(self, num_epochs, learn_rate, acc_metric, save_model=False, plot_progress=True, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam, batch_size=16, test_ratio=0.2, device=torch.device('cuda' if torch.cuda.is_available() else 'cpu')):
        self.num_epochs = num_epochs
        self.learn_rate = learn_rate
        self.acc_metric = acc_metric
        self.save_model = save_model
        self.plot_progress = plot_progress
        self.criterion = criterion
        self.optimizer = optimizer
        self.batch_size = batch_size
        self.test_ratio = test_ratio
        self.device = device


class TrainStatistics:
    def __init__(self):
        self.acc_train = []
        self.acc_test = []
        self.loss_train = []
        self.loss_test = []


def train(model, dataset, options):
    model.to(options.device)
    # torchsummary.summary(model, (1, 224, 224), device=options.device)
    num_workers = 0  # 0 = os.cpu_count()
    train_dataset, test_dataset = torch.utils.data.random_split(dataset, [(1-options.test_ratio), options.test_ratio])
    train_loader = torch.utils.data.DataLoader(dataset=train_dataset, batch_size=64, shuffle=True, num_workers=num_workers)
    test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=64, shuffle=False, num_workers=num_workers)
    optimizer = options.optimizer(model.parameters(), lr=options.learn_rate)
    # log.info(f"Model accuracy before training: train_dataset {options.acc_metric(model, train_loader):.1%}, test_dataset {options.acc_metric(model,  test_loader_raw):.1%}")

    if options.plot_progress:
        fig, axs = plot.create_fig("model training", 2, 1, aspect_ratio=1.4, sharex=True, position=(100, 100))

    log.info(
        f"Training started: train_size: {len(train_dataset)}, test_size: {len(test_dataset)}, batch_size: {options.batch_size}, device: '{options.device}', learn_rate: {options.learn_rate}")

    stats = TrainStatistics()
    loss_train, loss_test, acc_train, acc_test = np.inf, np.inf, 0, 0
    loop = tqdm(range(options.num_epochs), total=options.num_epochs, colour="green", bar_format=bar_format, unit="epoch")
    loop.set_description(f"Epoch [0/{options.num_epochs}]")
    loop.set_postfix(loss_train=loss_train, loss_test=loss_test, acc_train=acc_train, acc_test=acc_test)
    for epoch in loop:
        loss_train, loss_test, acc_train, acc_test = train_one_epoch(model, train_loader, test_loader, optimizer, options.criterion, options.device, options.acc_metric)
        loop.set_description(f"Epoch [{epoch+1}/{options.num_epochs}]")
        loop.set_postfix(loss_train=loss_train, loss_test=loss_test, acc_train=acc_train, acc_test=acc_test)

        if options.plot_progress:
            stats.acc_test.append(acc_test)
            stats.acc_train.append(acc_train)
            stats.loss_train.append(loss_train)
            stats.loss_test.append(loss_test)
            plot_progress(fig, axs, stats)

    log.info('Training finished')
    log.info(f"Model accuracy after training: train_dataset {acc_train:.1%}, test_dataset {acc_test:.1%}")

    if options.save_model:
        model_name = "gigachad"
        savepath = f"data/ml/{model_name}.pth"
        log.info(f"Saving model to {savepath}")
        torch.save(model.state_dict(), savepath)


def train_one_epoch(model, train_loader, test_loader, optimizer, criterion, device, acc_metric):
    model.train()

    loss_train = 0.0
    acc_train = 0.0
    num_batches = len(train_loader)
    loop = tqdm(enumerate(train_loader), total=num_batches, leave=False, colour="#0E86D4", bar_format=bar_format, unit="batch")
    loop.set_description("Batch")
    for batchidx, [inputs, targets] in loop:
        inputs = inputs.to(device)
        targets = targets.to(device)
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = criterion(outputs, targets)
        acc = acc_metric(outputs, targets)
        loss.backward()
        optimizer.step()
        loss_train += loss.item()*inputs.size(0)
        acc_train += acc*inputs.size(0)
        loop.set_description(f"Batch [{batchidx+1}/{num_batches}]")
        loop.set_postfix(loss_train=loss.item(), acc_train=acc)

    with torch.no_grad():
        loss_test = 0.0
        acc_test = 0.0
        num_batches = len(test_loader)
        loop = tqdm(enumerate(test_loader), total=num_batches, leave=False, colour="#7F00FF", bar_format=bar_format, unit="batch")
        loop.set_description("BatchT")
        for batchidx, [inputs, targets] in loop:
            inputs = inputs.to(device)
            targets = targets.to(device)
            outputs = model(inputs)
            loss = criterion(outputs, targets)
            acc = acc_metric(outputs, targets)
            loss_test += loss.item()*inputs.size(0)
            acc_test += acc*inputs.size(0)
            loop.set_description(f"Batch [{batchidx+1}/{num_batches}]")
            loop.set_postfix(loss_test=loss.item(), acc_test=acc)

    # sample-average loss & average accuracy
    return loss_train/len(train_loader.dataset), loss_test/len(test_loader.dataset), acc_train/len(train_loader.dataset), acc_test/len(test_loader.dataset)


def plot_progress(fig, axs, stats):
    (ax1, ax2) = axs

    ax1.clear()
    ax1.plot(stats.loss_train, linewidth=4, label="train loss")
    ax1.plot(stats.loss_test, linewidth=4, label="test loss")
    ax1.set_ylabel("loss")
    ax1.set_yscale("log")
    ax1.legend()

    ax2.clear()
    ax2.plot(stats.acc_train, linewidth=4, label="train accuracy")
    ax2.plot(stats.acc_test, linewidth=4, label="test accuracy")
    ax2.set_xlabel("epoch")
    ax2.set_ylabel("accuracy")
    ax2.legend()
    ax2.yaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(xmax=1.0))

    plt.tight_layout()
    plt.draw()
    plt.pause(1e-4)
