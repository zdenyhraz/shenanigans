import random
from datetime import datetime
import os
from script.log import log


def valid(sources, targets):
    for source, target in zip(sources, targets):
        if source == target:
            return False
    return True


sources = ["Tomas", "Mia", "Sasa", "Zdenek", "Babi", "Eva", "Maros", "Diana"]
targets = ["Tomas", "Mia", "Sasa", "Zdenek", "Babi", "Eva", "Maros", "Diana"]
file = True
random.seed(9)

while not valid(sources, targets):
    sublist = targets[2:]
    random.shuffle(sublist)
    targets[2:] = sublist

if not file:
    log.debug("Results:")
    for source, target in zip(sources, targets):
        log.debug(f"{source} -> {target}")
else:
    for source, target in zip(sources, targets):
        with open(f'{os.path.dirname(os.path.abspath(__file__))}/{source}.txt', 'w') as file:
            file.write(f'Darek na Vanoce od tebe dostava {target}.')
            file.write(f'\n\nNahodne vygenerovano {datetime.now().strftime("%d.%m.%Y v %H:%M:%S")} pro {sources}')
