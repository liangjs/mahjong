#!/usr/bin/env python3

from mahjong import *
import sys
import os
import time


nturn = [0] * PlayerNum
logs = [[] for i in range(PlayerNum)]
board = MahjongBoard(int(time.time()))
stat_cnt = 0


def actions(p):
    print(board.actions(p))
    print("-- " + logs[p][-1])


def interact(p, msg):
    global nturn, logs
    nturn[p] += 1
    logs[p].append(msg)
    with open("log/r%dp%d.in" % (nturn[p], p), "w") as f:
        f.write("%d\n" % nturn[p])
        for i in logs[p]:
            f.write(i + '\n')
    os.system("%s < log/r%dp%d.in > log/r%dp%d.out" % (sys.argv[p + 1], nturn[p], p, nturn[p], p))
    with open("log/r%dp%d.out" % (nturn[p], p), "r") as f:
        ans = f.readline()[:-1]
        logs[p].append(ans)
        return ans


def runerror(p):
    score = [10] * PlayerNum
    score[p] = -30
    print("%d %d %d %d" % tuple(score))
    sys.exit(0)


def hu(p, cfrom, fan):
    ans = MahjongBoard.fan_value(fan)
    if ans < HU_FAN:
        runerror(p)
    score = [0] * PlayerNum
    for i in range(PlayerNum):
        if i != p:
            t = HU_FAN
            if p == cfrom or i == cfrom:
                t += ans
            score[i] -= t
            score[p] += t
    print("%d %d %d %d" % tuple(score))
    sys.exit(0)


def turn01():
    for i in range(PlayerNum):
        if interact(i, "0 %d %d" % (i, board.quanfeng)) != "PASS":
            runerror(i)
    for i in range(PlayerNum):
        hand = reduce(lambda acc, idx: acc + [str(CardName(idx[0])) + str(idx[1])] * board.handcards[i][idx[0]][idx[1]],
                      CardIndexes, [])
        hand = ' '.join(hand)
        huanum = ' '.join(map(lambda x: str(len(x)), board.huas))
        hua = ' '.join(map(str, [c for i in range(PlayerNum) for c in board.huas[i]]))
        msg = "1 " + huanum + ' ' + hand + ' ' + hua
        if interact(i, msg) != "PASS":
            runerror(i)


def show_stat(msg):
    global stat_cnt
    print("turn %d" % stat_cnt)
    stat_cnt += 1
    board.show_stat()
    print(msg)
    print()


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: %s <player0> <player1> <player2> <player3>" % sys.argv[0])
        exit(1)
    if not os.path.exists("log"):
        os.mkdir("log")
    show_stat("INIT " + str(board.seed))

    turn01()
    while True:
        p, drawable = board.nextturn
        oc = board.outcard
        if not oc:
            if not drawable:
                for i in range(PlayerNum):
                    if logs[i][-1] == "HU":  # 抢杠和
                        fan = board.dianpao(i)
                        c = board.lastgang[0]
                        board.handcards[i][c.cname.value][c.num] += 1
                        show_stat("%d HU " % i + ' '.join(str(x[0]) + ' ' + x[1] for x in fan))
                        hu(i, board.nextturn[0], fan)
                cs = board.mopai(p)
                if cs is None:
                    runerror(p)
                if len(cs) == 0:
                    print("0 0 0 0")
                    sys.exit(0)
                for h in cs[:-1]:
                    for i in range(PlayerNum):
                        if interact(i, "3 %d BUHUA %s" % (p, h)) != "PASS":
                            runerror(i)
                show_stat("%d GET %s" % (p, board.lastmo))
                interact(p, "2 %s" % board.lastmo)
                for i in range(PlayerNum):
                    if i != p:
                        interact(i, "3 %d DRAW" % p)
            else:  # drawable
                msg = logs[p][-1]
                #actions(p)
                if msg[:4] == "PLAY":
                    c = Card.from_str(msg[5:7])
                    if not board.chupai(p, c):
                        runerror(p)
                    show_stat("%d PLAY %s" % (p, c))
                    for i in range(PlayerNum):
                        interact(i, "3 %d PLAY %s" % (p, c))
                elif msg[:4] == "GANG":
                    c = Card.from_str(msg[5:7])
                    if not board.angang(p, c):
                        runerror(p)
                    show_stat("%d GANG %s" % (p, c))
                    for i in range(PlayerNum):
                        interact(i, "3 %d GANG" % p)
                elif msg == "BUGANG":
                    c = board.lastmo
                    # TODO: wrong!!!!!!!
                    if not board.bugang(p, c):
                        runerror(p)
                    show_stat("%d BUGANG %s" % (p, c))
                    for i in range(PlayerNum):
                        interact(i, "3 %d BUGANG %s" % (p, c))
                elif msg == "HU":
                    fan = board.zimo(p)
                    show_stat("%d HU " % p + ' '.join(str(x[0]) + ' ' + x[1] for x in fan))
                    hu(p, p, fan)
                else:
                    runerror(p)
        else:  # outcard not None
            order = {"PASS": 0, "CHI": 1, "PENG": 2, "GANG": 3, "HU": 4}
            p = max(range(PlayerNum), key=lambda i: order[logs[i][-1].split(' ')[0]])
            msg = logs[p][-1]
            #actions(p)
            if msg == "PASS":
                board.outcard = None
            elif msg[:3] == "CHI":
                mid = Card.from_str(msg[4:6])
                c = Card.from_str(msg[7:9])
                if not board.chi(p, mid) or not board.chupai(p, c):
                    runerror(p)
                show_stat("%d CHI %s PLAY %s" % (p, mid, c))
                for i in range(PlayerNum):
                    interact(i, "3 %d CHI %s %s" % (p, mid, c))
            elif msg[:4] == "PENG":
                c = Card.from_str(msg[5:7])
                if not board.peng(p) or not board.chupai(p, c):
                    runerror(p)
                show_stat("%d PENG %s PLAY %s" % (p, oc, c))
                for i in range(PlayerNum):
                    interact(i, "3 %d PENG %s" % (p, c))
            elif msg == "GANG":
                if not board.gang(p):
                    runerror(p)
                show_stat("%d GANG %s" % (p, oc))
                for i in range(PlayerNum):
                    interact(i, "3 %d GANG" % p)
            elif msg == "HU":
                fan = board.dianpao(p)
                board.handcards[p][oc.cname.value][oc.num] += 1
                show_stat("%d HU " % p + ' '.join(str(x[0]) + ' ' + x[1] for x in fan))
                hu(p, board.cfrom, fan)
