import pandas as pd
import random


def gen_vehicles(req_path, veh_path, veh_num):
    df = pd.read_csv(req_path, names=['time', 'start', 'term'])
    vehicles = []
    cnt = 0
    for idx, row in df.iterrows():
        if cnt >= veh_num:
            break
        vehicles.append(row['start'])
        cnt += 1
    for i in range(veh_num - cnt):
        vehicles.append(random.randint(1, max_node))

    with open(veh_path, 'w') as f:
        for i in range(veh_num):
            f.write(str(vehicles[i]) + "\n")


veh_nums = [100, 250, 500, 750, 1000]

win_len = 10
dist_ranges = ['4', '6', '8', '10', 'ul']
for idx, dist_range in enumerate(dist_ranges):
    minute = 420
    while minute < 540:
        minute_1 = minute + win_len
        req_path = 'range_data/' + dist_range + '/req_' + str(minute) + '_' + str(minute_1) + '.csv'
        veh_path = 'range_data/' + dist_range + '/veh_' + str(minute) + '_' + str(minute_1) + '.csv'
        gen_vehicles(req_path, veh_path, veh_nums[idx])
        print(req_path)
        minute = minute_1