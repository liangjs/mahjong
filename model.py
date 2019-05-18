from keras.models import *
from keras.layers import *
from keras import regularizers
import numpy as np


class MahjongModel(Model):
    def __init__(self):
        super(MahjongModel, self).__init__()
        self.conv1 = Conv2D(40, kernel_size=(3, 3), padding='same', data_format='channels_first', activation='relu', kernel_regularizer=regularizers.l2(0.01))
        self.conv2 = Conv2D(10, kernel_size=(3, 3), padding='same', data_format='channels_first', activation='relu', kernel_regularizer=regularizers.l2(0.01))
        self.conv3 = Conv2D(10, kernel_size=(3, 3), padding='same', data_format='channels_first', activation='relu', kernel_regularizer=regularizers.l2(0.01))
        self.conv4 = Conv2D(10, kernel_size=(3, 3), padding='same', data_format='channels_first', activation='relu', kernel_regularizer=regularizers.l2(0.01))
        self.flat = Flatten()
        self.concat = Concatenate()
        self.dense1 = Dense(30, activation='relu', kernel_regularizer=regularizers.l2(0.01))
        self.op_out = Dense(5, activation='softmax')
        self.card_out = Dense(34, activation='relu')
        self.win = Dense(1, activation='sigmoid')

    def call(self, inputs, **kwargs):
        img = inputs[0]
        cfrom = inputs[1]
        x = self.conv1(img)
        x = self.conv2(x)
        x = self.conv3(x)
        x = self.conv4(x)
        x = self.flat(x)
        x = self.concat([x, cfrom])
        x = self.dense1(x)
        op = self.op_out(x)
        cards = self.card_out(x)
        win = self.win(x)
        return [op, cards, win]
