#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

from mahjong import *
import os
import random


def card_id(c):
    if c is None:
        return -1
    for i, c2 in enumerate(allcards_nohua):
        if c2 == c:
            return i
    return -1


DATA_LOADER_FILE_BATCH = 10


class LoaderHUAError(Exception):
    pass


class MahjongLoader:
    def __init__(self, file_name):
        self.records = []
        with open(file_name, 'r') as f:
            lines = f.readlines()
        fengmap = {'东': 0, '南': 1, '西': 2, '北': 3}
        quanfeng = zhuang = None
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
                quanfeng = fengmap[line[0]]
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
                zhuang = int(line[0])
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
        board = MahjongBoard(quanfeng=quanfeng, zhuang=zhuang, wallcards=wallcards)
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
        stat = board.player_stat(player)
        hands = [stat.handcards[player][idx[0]][idx[1]] for idx in CardIndexes]
        lefts = [stat.leftcards[idx[0]][idx[1]] for idx in CardIndexes]
        shuns = [0] * len(CardIndexes)
        kes = [0] * len(CardIndexes)
        for pack in stat.packs[player]:
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
        if stat.outcard:
            outc[card_id(stat.outcard)] = 1
            cfrom = (player - stat.cfrom) % PlayerNum
        in_tensor = [hands, shuns, kes, lefts, outc, cfrom]
        out_tensor = [[0] * 5, [0] * len(CardIndexes)]
        cid = card_id(card)
        # 出、过、吃、碰、杠
        if action == "chupai":
            out_tensor[0][0] = 1
            out_tensor[1][cid] = 1
        elif action == "pass":
            out_tensor[0][1] = 1
        elif action == "chi":
            out_tensor[0][2] = 1
            out_tensor[1][cid] = 1
        elif action == "peng":
            out_tensor[0][3] = 1
            out_tensor[1][cid] = 1
        elif action == "angang" or action == "bugang" or action == "gang":
            out_tensor[0][4] = 1
            out_tensor[1][cid] = 1
        else:
            raise RuntimeError("unknown action")
        #print(player)
        #print(in_tensor)
        #print(out_tensor)
        self.records.append([player, in_tensor, out_tensor])
    
    def _record_winner(self, win):
        score = [0] * PlayerNum
        if win == -1:
            score = [0.25] * PlayerNum
        else:
            score[win] = 1
        for r in self.records:
            r[2].append(score[r[0]])


class DataLoader:
    def __init__(self, path):
        self.files = []
        def addfiles(name):
            self.files += map(lambda f: os.path.join(path, name, f), os.listdir(os.path.join(path, name)))
        for name in ["LIU", "MO", "PLAY"]:
            addfiles(name)
        random.shuffle(self.files)
        self.data = []

    def _new_data(self):
        if len(self.files) == 0:
            return False
        num = min(len(self.files), DATA_LOADER_FILE_BATCH)
        for i in range(num):
            try:
                a = MahjongLoader(self.files[i])
            except LoaderHUAError:
                continue
            self.data += a.records
        del self.files[:num]
        random.shuffle(self.data)
        return True

    def get_data(self, num):
        while len(self.data) < num:
            if not self._new_data():
                break
        num = min(num, len(self.data))
        ans = self.data[:num]
        del self.data[:num]
        return ans
