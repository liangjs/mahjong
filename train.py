from model import MahjongModel
from keras import optimizers, losses
from loader import DataLoader
import numpy as np
import os


BATCH = 32
model_file = "model.h5"


if __name__ == "__main__":
    model = MahjongModel()
    model.compile(optimizers.Adam(lr=0.001, decay=0.0001),
                  loss=[losses.binary_crossentropy, losses.mean_squared_error, losses.mean_squared_error],
                  loss_weights=[1, 100, 10])
    loader = DataLoader("../data")
    idx = 0
    loss_period = 300
    save_period = 10000
    loss_sum = [0, 0, 0, 0]
    for data0 in loader.generate(processes=8, batch_size=BATCH):
        inputs, outputs = [], []
        for i in range(2):
            inputs.append(np.array([[data[1][i]] for data in data0]))
        for i in range(3):
            outputs.append(np.array([data[2][i] for data in data0]))
        loss = model.train_on_batch(inputs, outputs)
        if idx == 0 and os.path.isfile(model_file):
            model.load_weights(model_file)
            print("load model", model_file)
        for i in range(4):
            loss_sum[i] += loss[i]
        idx += 1
        if idx % loss_period == 0:
            print("loss", [x / loss_period for x in loss_sum])
            loss_sum = [0, 0, 0, 0]
        if idx % save_period == 0:
            model.save_weights(model_file)
            print("save model", model_file)
