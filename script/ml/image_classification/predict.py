import torch
import matplotlib.pyplot as plt
import sys
sys.path.append('script/ml')
import plot  # nopep8


def predict(model, image):
    device = next(model.parameters()).device
    input = torch.unsqueeze(image, 0).to(device)
    output = torch.softmax(torch.squeeze(model.forward(input)), 0)
    return torch.argmax(output), torch.max(output)


def predict_plot(model, dataset, num_samples):
    fig, axs = plot.create_fig("sample model predictions", 1, num_samples)
    for idx, dataidx in enumerate(torch.randint(0, len(dataset), (num_samples,))):
        image = dataset[dataidx][0]
        axs[idx].imshow(torch.squeeze(image).permute(1, 2, 0), interpolation='none', cmap="viridis")
        pred_class, confidence = predict(model, image)
        axs[idx].set_title(f"{dataset.classes[pred_class]} {confidence:.1%}")
    plt.tight_layout()
    plt.show()
