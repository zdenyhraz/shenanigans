import skimage.io as io
import os.path
import torch.utils.data
from pathlib import Path
import sys
sys.path.append('script/ml')  # nopep8
import log  # nopep8


class ImageClassificationDataset(torch.utils.data.Dataset):
    def __init__(self, root, transform):
        self.transform = transform
        self.samples = []
        self.classes = []
        log.debug(f"Loading ImageClassificationDataset from {root}")
        if not os.path.isdir(root):
            raise ValueError(f"'{root}' is not a directory")

        for class_directory in os.listdir(root):
            class_directory = os.path.join(root, class_directory)
            if not os.path.isdir(class_directory):
                continue
            class_name = Path(class_directory).stem
            self.classes.append(class_name)

            for sample in os.listdir(class_directory):
                sample = os.path.join(class_directory, sample)
                if not os.path.isfile(sample):
                    continue
                self.samples.append((sample, self.classes.index(class_name)))

        for class_index, class_name in enumerate(self.classes):
            num_samples = sum(1 for sample in self.samples if sample[1] == class_index)
            log.debug(f"Class '{class_name}' samples: {num_samples}")

    def __len__(self):
        return len(self.samples)

    def __getitem__(self, index):
        image = self.transform(io.imread(self.samples[index][0]))
        label = torch.tensor(self.samples[index][1])
        return image, label
