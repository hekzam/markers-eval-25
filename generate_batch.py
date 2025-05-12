import random
import json

config: list[list[str]] = json.load(open("all_config.json"))

nb_copies_per_config = 20

all_batch = []
for i in range(nb_copies_per_config):
    random.shuffle(config)
    all_batch += config

first = True

with open("batch.txt", "w") as f:
    for config, parser in all_batch:
        f.write(f"gen-parse --nb-copies 1 {"--warmup-iterations 1" if first else ""} --seed {random.randint(0,10000)} --parser-type {parser} --header-marker-size 10 --marker-config ({config}) --unencoded-marker-size 8 --encoded-marker-size 20 --dpi 200 --csv-filename out.csv {"" if first else "--csv-mode append"}\n")
        first = False
