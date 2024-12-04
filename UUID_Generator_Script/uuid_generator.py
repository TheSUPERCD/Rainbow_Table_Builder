import uuid
from pathlib import Path

required_number_of_UUIDs = int(input("Please type the number of UUIDs you want to generate: "))

path = Path('./data/')
path.mkdir(parents=True, exist_ok=True)
with open('./data/uuids_small.txt', 'w') as f:
    f.writelines(str(uuid.uuid1()).replace("-", "") + "\n" for i in range(required_number_of_UUIDs-1))
    f.writelines(str(uuid.uuid1()).replace("-", ""))
