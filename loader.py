#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

from mahjong import *


class MahjongLoader:
    def __init__(self, file_name):
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
        board.show_stat()
        res = ()
        for i, line in enumerate(lines):
            line = line.strip().split()
            if i < 6:
                continue
            player = int(line[0])
            if i == 6:
                print(board.mopai(player))
            op = line[1]
            board.show_stat()
            print(line)
            if op == "打牌":
                card = Card.from_str(eval(line[2])[0])
                assert board.chupai(player, card)
            elif op.find("摸牌") != -1:
                card = Card.from_str(eval(line[2])[0])
                if card.cname == CardName.HUA:
                    continue
                cards = board.mopai(player)
                assert card == cards[-1]
            elif op == "补花":
                continue
            elif op == "暗杠":
                card = Card.from_str(line[3])
                assert board.angang(player, card)
            elif op == "补杠":
                assert board.bugang(player)
            elif op == "明杠":
                assert board.gang(player)
            elif op == "碰":
                assert board.peng(player)
            elif op == "吃":
                card = Card.from_str(eval(line[2])[1])
                assert board.chi(player, card)
            elif op == "和牌":
                if lines[1].strip().split()[-1] == "自摸":
                    res = board.zimo(player)
                else:
                    res = board.dianpao(player)
            else:
                raise RuntimeError(op)
        board.show_stat()
        print(lines[1].strip().split())
        print(res)



if __name__ == "__main__":
    a = MahjongLoader("../data/MO/2017-05-11-35.txt")
