from model import MahjongModel
from keras import optimizers, losses
from loader import DataLoader
import numpy as np
import os, sys
from mahjong import *
from functools import reduce
import operator


np.set_printoptions(threshold=np.inf)


BATCH = 32
model_file = "model.h5"


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: %s <data> <model_file>" % sys.argv[0])
        exit(1)
    model_file = sys.argv[2]
    model = MahjongModel()
    model.compile(optimizers.Adam(lr=0.001, decay=0.0001),
                  loss=[losses.categorical_crossentropy, losses.categorical_crossentropy, losses.mean_absolute_error],
                  loss_weights=[10, 1, 10])
    model.predict([np.zeros((1, 1, 5, 34)), np.zeros((1, 3))]) # compute sizes
    if os.path.isfile(model_file):
        model.load_weights(model_file)
        print("load model", model_file)
    else:
        print("new model")
    for epoch in range(10):
        print("epoch", epoch)
        loader = DataLoader(sys.argv[1])
        idx = 0
        loss_period = 300
        save_period = 10000
        predict_period = 5000
        loss_sum = [0, 0, 0, 0]
        for data0 in loader.generate(processes=5, batch_size=BATCH):
            inputs, outputs = [], []
            inputs.append(np.array([[data[1][0]] for data in data0]))
            inputs.append(np.array([data[1][1] for data in data0]))
            for i in range(3):
                outputs.append(np.array([data[2][i] for data in data0]))
            if idx % predict_period == 0:
                pd = model.predict(inputs)
                for i in range(BATCH):
                    #print("input:", inputs[0][i], inputs[1][i])
                    print("-----")
                    print("hand:", ' '.join(map(str, reduce(operator.concat,
                                                            [[allcards[j]] * inputs[0][i][0,2,j] for j in range(34)]))))
                    outc = max(range(34), key=lambda x: inputs[0][i][0,4,x])
                    print("out:", "null" if inputs[0][i][0,4,outc] < 0.1 else allcards[outc])
                    ansop = max(range(7), key=lambda x: outputs[0][i][x])
                    ans = max(range(35), key=lambda x: outputs[1][i][x])
                    mp = ["chu", "pass", "chi-1", "chi+0", "chi+1", "peng", "gang"]
                    print("answer:", mp[ansop], "null" if ansop != 0 else allcards[ans])
                    #print("answer:", outputs[0][i], outputs[1][i])
                    myop = max(range(7), key=lambda x: pd[0][i][x])
                    out = max(range(35), key=lambda x: pd[1][i][x])
                    print("output:", mp[myop], "null" if myop != 0 else allcards[out])
                    print("prob:", pd[0][i][ansop], pd[1][i][ans])
                    #print(pd[0][i], pd[1][i])
            loss = model.train_on_batch(inputs, outputs)
            for i in range(4):
                loss_sum[i] += loss[i]
            idx += 1
            if idx % loss_period == 0:
                print("loss", [x / loss_period for x in loss_sum])
                loss_sum = [0, 0, 0, 0]
            if idx % save_period == 0:
                model.save_weights(model_file)
                print("save model", model_file)
