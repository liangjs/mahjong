#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

from mahjong import *
import os


class LoaderHUAError(Exception):
    pass


def get_stat(file_name):
    with open(file_name, 'r') as f:
        lines = f.readlines()
    stat = [0] * len(CardIndexes)
    for i, line in enumerate(lines):
        line = line.strip().split()
        if i <= 1:
            continue
        if 2 <= i <= 5:
            player = int(line[0])
            assert player == i - 2
            cards = list(map(Card.from_str, eval(line[1])))
            for c in cards:
                cid = card_id(c)
                if cid != -1:
                    stat[cid] += 1
            continue
        op = line[1]
        if op == "打牌":
            pass
        elif op.find("摸牌") != -1:
            card = Card.from_str(eval(line[2])[0])
            cid = card_id(card)
            if cid != -1:
                stat[cid] += 1
        elif op == "补花":
            continue
        elif op == "暗杠":
            pass
        elif op == "补杠":
            pass
        elif op == "明杠":
            card = Card.from_str(line[3])
            cid = card_id(card)
            if cid != -1:
                stat[cid] += 1
        elif op == "碰":
            card = Card.from_str(line[3])
            cid = card_id(card)
            if cid != -1:
                stat[cid] += 1
        elif op == "吃":
            card = Card.from_str(line[3])
            cid = card_id(card)
            if cid != -1:
                stat[cid] += 1
        elif op == "和牌":
            card = Card.from_str(line[3])
            cid = card_id(card)
            if cid != -1:
                stat[cid] += 1
            break
        else:
            raise RuntimeError(op)
    stat = [x / 16.0 for x in stat]
    return stat


if __name__ == "__main__":
    path = "/run/media/liangjs/Develop/study/game/data/"
    files = []
    def addfiles(files, name):
        files += map(lambda f: os.path.join(path, name, f), os.listdir(os.path.join(path, name)))
    for name in ["LIU", "MO", "PLAY"]:
        addfiles(files, name)
    random.shuffle(files)

    ans = [0] * len(CardIndexes)
    num = 0
    for fname in files:
        try:
            stat = get_stat(fname)
        except:
            continue
        for i in range(len(CardIndexes)):
            ans[i] = ans[i] * (num / (num+1)) + stat[i] / (num + 1)
        num += 1
        if num % 10000 == 0:
            print('%.2f' % (num / len(files) * 100) + '%')
            for i in range(len(CardIndexes)):
                print(allcards_nohua[i], ans[i])
