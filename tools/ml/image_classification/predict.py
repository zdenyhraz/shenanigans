import torch
import matplotlib.pyplot as plt
import sys
sys.path.append('script/ml')
import plot  # nopep8
import log  # nopep8


def predict(model, image):
    device = next(model.parameters()).device
    x = torch.unsqueeze(image, 0).to(device)
    scores = torch.softmax(torch.squeeze(model.forward(x)), 0)
    class_id = torch.argmax(scores).item()
    return class_id, scores[class_id].item()


def predict_plot(model, dataset, num_samples):
    fig, axs = plot.create_fig("Sample model predictions on test dataset", 1, num_samples)
    for idx, dataidx in enumerate(torch.randint(0, len(dataset), (num_samples,))):
        image = dataset[dataidx][0]
        dims = image.dim()
        if dims != 3:
            log.warning(f"Cannot plot images with {dims} dimensions")
            plt.close(fig)
            return
        channels = image.size(0)
        if channels == 3:  # rgb
            axs[idx].imshow(torch.squeeze(image).permute(1, 2, 0), interpolation='none')
        elif channels == 1:  # grayscale
            axs[idx].imshow(torch.squeeze(image), interpolation='none', cmap="viridis")
        else:
            log.warning(f"Cannot plot images with {channels} channels")
            plt.close(fig)
            return
        class_id, conf = predict(model, image)
        axs[idx].set_title(f"{dataset.dataset.classes[class_id]} {conf:.1%}")
    plt.tight_layout()
    plt.show()


def predict_all(model, dataset):
    for idx, [image, target] in enumerate(dataset):
        class_id, score = predict(model, image)
        if class_id == target:
            log.info(f"[{idx+1}/{len(dataset)}]: Predicted '{dataset.dataset.classes[target]}' as '{dataset.dataset.classes[class_id]}' {score:.1%}")
        else:
            log.warning(f"[{idx+1}/{len(dataset)}]: Predicted '{dataset.dataset.classes[target]}' as '{dataset.dataset.classes[class_id]}' {score:.1%}")
