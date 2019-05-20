#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

from mahjong import *
import os
import random
from multiprocessing import Queue, Process
import queue


class LoaderHUAError(Exception):
    pass


class MahjongLoader:
    def __init__(self, file_name):
        self.file = file_name
        self.records = []
        with open(file_name, 'r') as f:
            lines = f.readlines()
        fengmap = {'东': 0, '南': 1, '西': 2, '北': 3}
        self.quanfeng = self.zhuang = None
        leftcards = [[0] * CardNumMax for i in range(CardNameMax)]
        for c in allcards + allcards_nohua * 3:
            leftcards[c.cname.value][c.num] += 1
        wallcards = []
        huanum = [0] * 4
        first_card = None
        for i, line in enumerate(lines):
            line = line.strip().split()
            if i == 0:
                continue
            if i == 1:
                self.quanfeng = fengmap[line[0]]
                continue
            if 2 <= i <= 5:
                player = int(line[0])
                assert player == i - 2
                cards = list(map(Card.from_str, eval(line[1])))
                if any(c.cname == CardName.HUA for c in cards):
                    raise LoaderHUAError()
                if len(cards) == 14:
                    first_card = cards[-1]
                    del cards[-1]
                num = int(line[2])
                wallcards += [Card(CardName.HUA, x + sum(huanum) + 1) for x in range(num)]
                huanum[player] = num
                wallcards += cards
                continue
            if i == 6:
                wallcards.append(first_card)
                self.zhuang = int(line[0])
            if line[1].find("摸牌") != -1:
                card = Card.from_str(eval(line[2])[0])
                if card.cname == CardName.HUA:
                    card.num = sum(huanum) + 1
                    huanum[int(line[0])] += 1
                wallcards.append(card)
        for c in wallcards:
            leftcards[c.cname.value][c.num] -= 1
        for c in allcards:
            wallcards += [c] * leftcards[c.cname.value][c.num]
        board = MahjongBoard(quanfeng=self.quanfeng, zhuang=self.zhuang, wallcards=wallcards)
        #board.show_stat()
        res = ()
        win = -1
        lasthua = False
        for i, line in enumerate(lines):
            line = line.strip().split()
            if i < 6:
                continue
            player = int(line[0])
            if i == 6:
                board.mopai(player)
            op = line[1]
            #board.show_stat()
            #print(line)
            if lasthua:
                if op != "补花":
                    raise LoaderHUAError()
                lasthua = False
            if op == "打牌":
                card = Card.from_str(eval(line[2])[0])
                self._record(board, player, "chupai", card)
                assert board.chupai(player, card)
            elif op.find("摸牌") != -1:
                card = Card.from_str(eval(line[2])[0])
                if card.cname == CardName.HUA:
                    lasthua = True
                    continue
                if board.outcard:
                    for p in range(PlayerNum):
                        self._record(board, p, "pass", None)
                cards = board.mopai(player)
                assert card == cards[-1]
            elif op == "补花":
                continue
            elif op == "暗杠":
                card = Card.from_str(line[3])
                self._record(board, player, "angang", card)
                assert board.angang(player, card)
            elif op == "补杠":
                card = Card.from_str(line[3])
                self._record(board, player, "bugang", card)
                assert board.bugang(player, card)
            elif op == "明杠":
                for p in range(PlayerNum):
                    if p != player:
                        self._record(board, p, "pass", None)
                    else:
                        self._record(board, player, "gang", board.outcard)
                assert board.gang(player)
            elif op == "碰":
                for p in range(PlayerNum):
                    if p != player:
                        self._record(board, p, "pass", None)
                    else:
                        self._record(board, player, "peng", board.outcard)
                assert board.peng(player)
            elif op == "吃":
                card = Card.from_str(eval(line[2])[1])
                for p in range(PlayerNum):
                    if p != player:
                        self._record(board, p, "pass", None)
                    else:
                        self._record(board, player, "chi", card)
                assert board.chi(player, card)
            elif op == "和牌":
                win = player
                if lines[1].strip().split()[-1] == "自摸":
                    res = board.zimo(player)
                else:
                    res = board.dianpao(player)
                break
            else:
                raise RuntimeError(op)
        #board.show_stat()
        #print(lines[1].strip().split())
        #print(res)
        self.winner = win
        self.fan = res
        self._record_winner(win)

    def _record(self, board, player, action, card):
        hands = [board.handcards[player][idx[0]][idx[1]] for idx in CardIndexes]
        lefts = [board.leftcards[idx[0]][idx[1]] for idx in CardIndexes]
        for p in range(PlayerNum):
            if p != player:
                for i in range(len(lefts)):
                    idx = CardIndexes[i]
                    lefts[i] += board.handcards[p][idx[0]][idx[1]]
        shuns = [0] * len(CardIndexes)
        kes = [0] * len(CardIndexes)
        for pack in board.packs[player]:
            cid = card_id(pack.card)
            if pack.pname == PackName.CHI:
                shuns[cid] += 1
            elif pack.pname == PackName.PENG:
                kes[cid] += 3
            elif pack.pname == PackName.GANG:
                if pack.cfrom != player:
                    kes[cid] += 4
                else:
                    kes[cid] -= 4
        outc = [0] * len(CardIndexes)
        cfrom = 0
        if board.outcard:
            outc[card_id(board.outcard)] = 1
            cfrom = (player - board.cfrom) % PlayerNum
        in_tensor = [[shuns, kes, hands, lefts, outc], [cfrom, self.quanfeng, (player - self.zhuang) % PlayerNum]]
        out_tensor = [[0] * 7, [0] * (len(CardIndexes) + 1)]
        cid = card_id(card)
        # 出、过、吃-1、吃+0、吃+1、碰、杠
        if action == "chupai":
            out_tensor[0][0] = 1
            out_tensor[1][cid] = 1
            if board.outcard:
                print("error, outcard!!!", self.file)
                return
        else:
            out_tensor[1][-1] = 1
            if action == "pass":
                if len(board.actions(player)) == 1:
                    return
                if random.random() > 0.25:
                    return
                out_tensor[0][1] = 1
            elif action == "chi":
                out_tensor[0][3 + card.num - board.outcard.num] = 1
            elif action == "peng":
                out_tensor[0][5] = 1
            elif action == "gang":
                out_tensor[0][6] = 1
            elif action == "angang" or action == "bugang":
                return
            else:
                raise RuntimeError("unknown action")
        #print(player)
        #print(in_tensor)
        #print(out_tensor)
        self.records.append([player, in_tensor, out_tensor])
    
    def _record_winner(self, win):
        """
        score = [0] * PlayerNum
        if win == -1:
            score = [0.25] * PlayerNum
        else:
            score[win] = 1
        """
        score = [0.25] * PlayerNum
        for r in self.records:
            r[2].append(score[r[0]])


class DataLoader:
    def __init__(self, path):
        DATA_LOADER_BUF_MAX = 1000
        files = []
        def addfiles(files, name):
            files += map(lambda f: os.path.join(path, name, f), os.listdir(os.path.join(path, name)))
        for name in ["LIU", "MO", "PLAY"]:
            addfiles(files, name)
        random.shuffle(files)
        self.files = Queue()
        for i in files:
            self.files.put(i)
        self.data = Queue(DATA_LOADER_BUF_MAX)

    def generate(self, processes, batch_size):
        def worker(worker_id):
            data = []
            while True:
                try:
                    file = self.files.get(False)
                    try:
                        a = MahjongLoader(file)
                    except LoaderHUAError:
                        continue
                    except AssertionError:
                        print("!! assert error", file)
                    while len(a.records) != 0:
                        num = min(batch_size - len(data), len(a.records))
                        data += a.records[:num]
                        del a.records[:num]
                        if len(data) == batch_size:
                            self.data.put(data)
                            data = []
                except queue.Empty:
                    if self.files.empty():
                        print("exit", worker_id)
                        break
            self.data.put(data)

        procs = [Process(target=worker, args=(i,)) for i in range(processes)]
        for proc in procs:
            proc.start()
        while True:
            try:
                data = self.data.get(timeout=1)
                yield data
            except queue.Empty:
                if self.files.empty():
                    break
        for proc in procs:
            proc.join()


if __name__ == "__main__":
    MahjongLoader("/run/media/liangjs/Develop/study/game/data/PLAY/2017-01-25-2260.txt")
