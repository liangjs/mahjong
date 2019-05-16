#!/usr/bin/env python3

from MahjongGB import MahjongFanCalculator
from enum import Enum
import random
from functools import reduce
import copy


class CardName(Enum):
    WAN = 0
    BING = 1
    TIAO = 2
    FENG = 3
    JIAN = 4
    HUA = 5

    @staticmethod
    def from_char(c):
        _convert2cname = {'W': CardName.WAN,
                          'B': CardName.BING,
                          'T': CardName.TIAO,
                          'F': CardName.FENG,
                          'J': CardName.JIAN,
                          'H': CardName.HUA}
        return _convert2cname[c]

    def __str__(self):
        _convert2char = ('W', 'B', 'T', 'F', 'J', 'H')
        return _convert2char[self.value]

    __repr__ = __str__


class Card:
    def __init__(self, cname, num):
        self.cname = cname
        self.num = num

    def __eq__(self, other):
        return self.cname == other.cname and self.num == other.num

    def __str__(self):
        return str(self.cname) + str(self.num)

    __repr__ = __str__

    @staticmethod
    def from_str(s):
        return Card(CardName.from_char(s[0]), int(s[1]))


class PackName(Enum):
    NONE = 0
    PENG = 1
    GANG = 2
    CHI = 3

    def __str__(self):
        _convert2char = ('NONE', 'PENG', 'GANG', 'CHI')
        return _convert2char[self.value]

    __repr__ = __str__


class Pack:
    def __init__(self, pname, card, cfrom):
        self.pname = pname
        self.card = card
        self.cfrom = cfrom

    def __str__(self):
        return '{' + str(self.pname) + ', ' + str(self.card) + '}'

    __repr__ = __str__


allcards_nohua = [Card(CardName(i), j) for i in range(3) for j in range(1, 10)] \
                 + [Card(CardName.FENG, i) for i in range(1, 5)] \
                 + [Card(CardName.JIAN, i) for i in range(1, 4)]
allcards = allcards_nohua + [Card(CardName.HUA, i) for i in range(1, 9)]

HU_FAN = 8
CardNameMax = 6
CardNumMax = 10
PackNameMax = 4
PlayerNum = 4
CardIndexes = [(i, j) for i in range(3) for j in range(1, 10)] \
              + [(CardName.FENG.value, i) for i in range(1, 5)] \
              + [(CardName.JIAN.value, i) for i in range(1, 4)]


class MahjongBoard:
    def __init__(self, seed=0, quanfeng=None, zhuang=0, wallcards=None):
        self.seed = seed
        self.rand = random.Random(seed)
        self.zhuang = zhuang
        if wallcards is None:
            self.wallcards = allcards + allcards_nohua * 3
            self.rand.shuffle(self.wallcards)
        else:
            self.wallcards = wallcards
        if quanfeng is None:
            self.quanfeng = self.rand.randint(0, 3)
        else:
            self.quanfeng = quanfeng
        self.leftcards = [[0] * CardNumMax for i in range(CardNameMax)]
        for c in self.wallcards:
            self.leftcards[c.cname.value][c.num] += 1
        self.handcards = [[[0] * CardNumMax for i in range(CardNameMax)] for i in range(PlayerNum)]
        self.huas = [[] for i in range(PlayerNum)]
        self.packs = [[] for i in range(PlayerNum)]
        self.throweds = [[] for i in range(PlayerNum)]

        for p in range(PlayerNum):
            cnt = 0
            while cnt != 13:
                c = self.wallcards[0]
                del self.wallcards[0]
                self.leftcards[c.cname.value][c.num] -= 1
                if c.cname == CardName.HUA:
                    self.huas[p].append(c)
                else:
                    cnt += 1
                    self.handcards[p][c.cname.value][c.num] += 1

        self.outcard = None
        self.cfrom = None
        self.lastgang = (None, False)  # <card, shown>
        self.lastmo = None
        self.nextturn = (zhuang, False)  # <player, drawable>

    def show_player(self, p):
        hand = reduce(lambda acc, idx: acc + [str(CardName(idx[0])) + str(idx[1])] * self.handcards[p][idx[0]][idx[1]],
                      CardIndexes, [])
        hand = ' '.join(hand)
        hua = ' '.join(map(str, self.huas[p]))
        pack = ' '.join(map(str, self.packs[p]))
        print(pack + " | " + hand + " | " + hua)

    def show_remain(self):
        card = map(lambda idx: str(CardName(idx[0])) + str(idx[1]) + '=' + str(self.leftcards[idx[0]][idx[1]]),
                   CardIndexes)
        print(' '.join(card))

    def show_stat(self):
        self.show_remain()
        for p in range(PlayerNum):
            self.show_player(p)

    def mopai(self, p):
        if p != self.nextturn[0] or self.nextturn[1]:
            return None
        ans = []
        while len(self.wallcards) != 0:
            c = self.wallcards[0]
            del self.wallcards[0]
            self.leftcards[c.cname.value][c.num] -= 1
            ans.append(c)
            if c.cname == CardName.HUA:
                self.huas[p].append(c)
            else:
                self.handcards[p][c.cname.value][c.num] += 1
                self.nextturn = (p, True)
                self.lastmo = c
                return ans
        return ans

    def chupai(self, p, c):
        if self.handcards[p][c.cname.value][c.num] == 0:
            return False
        if p != self.nextturn[0] or not self.nextturn[1]:
            return False
        self.handcards[p][c.cname.value][c.num] -= 1
        self.throweds[p].append(c)
        self.outcard = c
        self.cfrom = p
        self.lastgang = (None, False)
        self.nextturn = ((p + 1) % PlayerNum, False)
        return True

    def angang(self, p, c):
        if self.handcards[p][c.cname.value][c.num] < 4:
            return False
        if p != self.nextturn[0] or not self.nextturn[1]:
            return False
        self.handcards[p][c.cname.value][c.num] = 0
        self.packs[p].append(Pack(PackName.GANG, c, p))
        self.outcard = None
        self.lastgang = (c, False)
        self.nextturn = (p, False)
        return True

    def bugang(self, p):
        c = self.lastmo
        if c is None:
            return False
        if p != self.nextturn[0] or not self.nextturn[1]:
            return False
        for i in range(len(self.packs[p])):
            pk = self.packs[p][i]
            if pk.card == c and pk.pname == PackName.PENG:
                self.handcards[p][c.cname.value][c.num] -= 1
                pk.pname = PackName.GANG
                self.outcard = None
                self.lastgang = (c, True)
                self.nextturn = (p, False)
                return True
        return False

    def gang(self, p):
        c = self.outcard
        if c is None:
            return False
        if self.handcards[p][c.cname.value][c.num] < 3:
            return False
        self.handcards[p][c.cname.value][c.num] -= 3
        self.packs[p].append(Pack(PackName.GANG, c, self.cfrom))
        self.outcard = None
        self.lastgang = (c, True)
        self.nextturn = (p, False)
        return True

    def peng(self, p):
        c = self.outcard
        if c is None:
            return False
        if self.handcards[p][c.cname.value][c.num] < 2:
            return False
        self.handcards[p][c.cname.value][c.num] -= 2
        self.packs[p].append(Pack(PackName.PENG, c, self.cfrom))
        self.outcard = None
        self.lastgang = (None, False)
        self.nextturn = (p, True)
        return True

    def chi(self, p, mid):
        c = self.outcard
        if c is None or c.cname != mid.cname:
            return False
        if mid.num == 1 or mid.num == 9:
            return False
        cnt = 0
        for i in range(mid.num - 1, mid.num + 2):
            if i != c.num:
                cnt += 1
                self.handcards[p][c.cname.value][i] -= 1
        if cnt != 2:
            for i in range(mid.num - 1, mid.num + 2):
                if i != c.num:
                    self.handcards[p][c.cname][i] += 1
            return False
        self.packs[p].append(Pack(PackName.CHI, mid, c.num - mid.num + 2))
        self.outcard = None
        self.lastgang = (None, False)
        self.nextturn = (p, True)
        return True

    def _hu(self, p, c, iszimo):
        pack = [(str(i.pname), str(i.card), i.cfrom if i.pname == PackName.CHI else (p - i.cfrom) % PlayerNum)
                for i in self.packs[p]]
        hand = reduce(lambda acc, idx: acc + [str(CardName(idx[0])) + str(idx[1])] * self.handcards[p][idx[0]][idx[1]],
                      CardIndexes, [])
        try:
            fan = MahjongFanCalculator(tuple(pack),
                                       tuple(hand),
                                       str(c),
                                       len(self.huas[p]),
                                       iszimo,
                                       self.leftcards[c.cname.value][c.num] \
                                        + sum(self.handcards[i][c.cname.value][c.num] for i in range(PlayerNum)) \
                                        - self.handcards[p][c.cname.value][c.num] == 0,
                                       self.lastgang[0] is not None,
                                       len(self.wallcards) == 0,
                                       (p - self.zhuang) % PlayerNum,
                                       self.quanfeng)
            #ans = reduce(lambda x, y: x + y[0], fan, 0)
            #return ans
            return fan
        except Exception as e:
            #print(e)
            return ()

    def zimo(self, p):
        if p != self.nextturn[0] or not self.nextturn[1]:
            return ()
        c = self.lastmo
        self.handcards[p][c.cname.value][c.num] -= 1
        ans = self._hu(p, c, iszimo=True)
        self.handcards[p][c.cname.value][c.num] += 1
        return ans

    def dianpao(self, p):
        c = self.outcard if not self.lastgang[0] or not self.lastgang[1] else self.lastgang[0]
        if c is None:
            return ()
        return self._hu(p, c, iszimo=False)

    @staticmethod
    def fan_value(fan):
        return reduce(lambda x, y: x + y[0], fan, 0)

    def actions(self, p):
        if self.outcard:
            ans = ["PASS"]
            if self.cfrom == p:
                return ans
            if MahjongBoard.fan_value(self.dianpao(p)) >= HU_FAN:
                return ["DIANPAO"]
            c = self.outcard
            if self.handcards[p][c.cname.value][c.num] == 3:
                ans.append("GANG")
            if self.handcards[p][c.cname.value][c.num] >= 2:
                ans.append("PENG")
            for mid in range(c.num - 1, c.num + 2):
                if mid >= 2 and mid <= 8 and sum(int(x != c.num and self.handcards[p][c.cname.value][x] > 0)
                                                 for x in range(mid - 1, mid + 2)) == 2:
                    ans.append("CHI " + str(c.cname) + str(mid))
            return ans
        elif self.nextturn[1]:
            if self.nextturn[0] != p:
                return ["PASS"]
            if MahjongBoard.fan_value(self.zimo(p)) >= HU_FAN:
                return ["ZIMO"]
            ans = ["PLAY"]
            c = self.lastmo
            if self.handcards[p][c.cname.value][c.num] == 4:
                ans += ["ANGANG"]
            elif any(i.pname == PackName.PENG and i.card == c for i in self.packs[p]):
                ans.append("BUGANG")
            return ans
        else:
            if self.nextturn[0] == p:
                return ["MO"]
            elif MahjongBoard.fan_value(self.dianpao(p)) >= HU_FAN:
                return ["DIANPAO"]
            else:
                return ["PASS"]

    def player_stat(self, p):
        ans = copy.deepcopy(self)
        del ans.wallcards
        for i in range(PlayerNum):
            if i != p:
                for idx in CardIndexes:
                    ans.leftcards[idx[0]][idx[1]] += self.handcards[i][idx[0]][idx[1]]
                    ans.handcards[i][idx[0]][idx[1]] = 0
        if not ans.lastgang[1]:
            ans.lastgang = (None, False)
        return ans


if __name__ == "__main__":
    print("Don't run this file.")
    exit(1)
