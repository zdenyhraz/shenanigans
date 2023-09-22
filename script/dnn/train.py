import torch.nn as nn
import torch.optim as optim
import torch.utils.data


class TrainOptions:
    def __init__(self, num_epochs=10, learn_rate=5e-1, save_model=False, log_progress=True, plot_progress=True, criterion=nn.CrossEntropyLoss(), optimizer=optim.Adam, batch_size=16, test_ratio=0.2):
        self.num_epochs = num_epochs
        self.learn_rate = learn_rate
        self.save_model = save_model
        self.log_progress = log_progress
        self.plot_progress = plot_progress
        self.criterion = criterion
        self.optimizer = optimizer
        self.batch_size = batch_size
        self.test_ratio = test_ratio


def train(model, dataset, options):
    train_dataset, test_dataset = torch.utils.data.random_split(dataset, [len(dataset)*(1-options.test_ratio), len(dataset)*options.test_ratio])
    train_loader = torch.utils.data.DataLoader(dataset=train_dataset, batch_size=options.batch_size, shuffle=True)
    test_loader = torch.utils.data.DataLoader(dataset=test_dataset, batch_size=options.batch_size, shuffle=True)
    optimizer = options.optimizer(model.parameters(), lr=options.learn_rate)

    for epoch in range(options.num_epochs):
        loss_train, loss_test = train_one_epoch(model, train_loader, test_loader, optimizer, options.criterion)
        if options.log_progress:
            print("Epoch {} | TrainLoss {:.2e} | TestLoss {:.2e}", epoch, loss_train, loss_test)

    print('Finished Training')
    if options.save_model:
        torch.save(model.state_dict(), "model.pth")


def train_one_epoch(model, train_loader, test_loader, optimizer, criterion):
    loss_train = 0.0
    for inputs, targets in train_loader:
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = criterion(outputs, targets)
        loss.backward()
        optimizer.step()
        loss_train += loss.item()*inputs.size(0)

    loss_test = 0.0
    for inputs, targets in test_loader:
        outputs = model(inputs)
        loss = criterion(outputs, targets)
        loss_test += loss.item()*inputs.size(0)

    return loss_train/len(train_loader.sampler), loss_test/len(test_loader.sampler)  # sample-average loss
