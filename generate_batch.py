import random
import json

config: list[list[str]] = json.load(open("all_config.json"))

set_config = set()
for c, _ in config:
    set_config.add(c)

print("Number of configurations:", len(set_config))
print("Number of configurations with parser:", len(config))

nb_copies_per_config = 20
header_marker_size = 10
unencoded_marker_size = 8
encoded_marker_size = 20
dpi = 200
line = []
gen_parse_bench = True

if gen_parse_bench:
    all_batch = []
    for i in range(nb_copies_per_config):
        random.shuffle(config)
        all_batch += config

    first = True

    for config, parser in all_batch:
        line.append(f"gen-parse --nb-copies 1 {"--warmup-iterations 1" if first else ""} --seed {random.randint(0,10000)} --parser-type {parser} --header-marker-size {header_marker_size} --marker-config {config} --unencoded-marker-size {unencoded_marker_size} --encoded-marker-size {encoded_marker_size} --dpi {dpi} --csv-filename gen-parse.csv {"" if first else "--csv-mode append"}\n")
        first = False

config_analysis_bench = True
if config_analysis_bench:
    first = True
    for config in set_config:
        line.append(f"config-analysis --header-marker-size {header_marker_size} --marker-config {config} --unencoded-marker-size {unencoded_marker_size} --encoded-marker-size {encoded_marker_size} --dpi {dpi} --csv-filename config-analysis.csv {"" if first else "--csv-mode append"}\n")
        first = False

with open("batch.txt", "w") as f:
    f.write("".join(line))
    f.write("\n")
    