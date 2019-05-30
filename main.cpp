#include "LibMahjongGB/MahjongGB.h"

//#define KEEP_RUN

//#define LOCAL

int nturn, turn = 1;

void output(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
#ifdef KEEP_RUN
    vprintf(fmt, ap);
    va_end(ap);
    printf(">>>BOTZONE_REQUEST_KEEP_RUNNING<<<\n");
    fflush(stdout);
#else
    if (turn++ == nturn) {
        vprintf(fmt, ap);
        exit(0);
    }
    else {
        va_end(ap);
        scanf("%*s%*[^\n]"); // ignore response
    }
#endif
}

void ungets(const char *s)
{
    for (const char *i = s; *i; ++i)
        ungetc(*i, stdin);
}

const double eps = 1e-3;
const double inf = 1e100;
const double timeout = 0.93;

#define ALPHA 2.5

class Timer
{
    clock_t cl;
public:
    void start()
    {
        cl = clock();
    }
    double get()
    {
        return double(clock() - cl) / CLOCKS_PER_SEC;
    }
    bool out()
    {
        return get() > timeout;
    }
} timer;

#define TIMER_JUMP(timer, label) {if (timer.out()) goto label;}

#define ADDHUA 0

#define CNAME_WAN   0
#define CNAME_BING  1
#define CNAME_TIAO  2
#define CNAME_FENG  3
#define CNAME_JIAN  4
#define CNAME_HUA   5
#define CNAME_XUNUM 3   /* 3种序数牌 */
#define CNAME_TNUM  6   /* 6种类型 */
#define CNAME_NONE  6
#define NUM_MAX     10  /* 数字1-9 */
const char cname2char[CNAME_TNUM + 1] = {'W', 'B', 'T', 'F', 'J', 'H', '?'};
inline uint8_t char2cname(char x)
{
    switch (x) {
    case 'W':
        return CNAME_WAN;
    case 'B':
        return CNAME_BING;
    case 'T':
        return CNAME_TIAO;
    case 'F':
        return CNAME_FENG;
    case 'J':
        return CNAME_JIAN;
    case 'H':
        return CNAME_HUA;
    default:
        return CNAME_NONE;
    }
}

struct Card {
    uint8_t cname;  // 牌名
    uint8_t num;    // 数字
    Card(): cname(CNAME_NONE), num(0) {}
    Card(uint8_t c, uint8_t n): cname(c), num(n) {}
    bool operator== (const Card &a) const
    {
        return cname == a.cname && num == a.num;
    }
    string toString() const
    {
        string ans(1, cname2char[cname]);
        return ans += num + '0';
    }
};

#define PNAME_NONE  0
#define PNAME_PENG  1
#define PNAME_GANG  2
#define PNAME_CHI   3
#define PNAME_BUKAO 4
const int reqnum[3] = {2, 3, 4};
const char *pname2char[4] = {"NONE", "PENG", "GANG", "CHI"};
#define FROM_SELF   0xff    // 非鸣牌
#define FROM_ANY    0xfe
struct Pack {
    uint8_t pname;
    Card card;  // 见算番器说明，暗杠cname=NONE表示不知道
    uint8_t from;   /* 碰和杠：来自谁
                     * 吃：123表示第几张来自上家
                     * 来自自己的杠是暗杠
                     */
    Pack(uint8_t p, Card c, uint8_t f = FROM_SELF): pname(p), card(c), from(f) {}
};

vector<Pack> allpacks, allduis;
void initAllPacks()
{
    // 顺子
    for (int i = 0; i < CNAME_XUNUM; ++i)
        for (int j = 1; j <= 7; ++j)
            allpacks.push_back(Pack(PNAME_CHI, Card(i, j + 1), FROM_ANY));
    // 刻杠
    for (int i = 0; i < CNAME_XUNUM; ++i)
        for (int j = 1; j <= 9; ++j) {
            allpacks.push_back(Pack(PNAME_PENG, Card(i, j), FROM_ANY));
            allpacks.push_back(Pack(PNAME_GANG, Card(i, j), FROM_ANY));
        }
    for (int i = 1; i <= 4; ++i) {
        allpacks.push_back(Pack(PNAME_PENG, Card(CNAME_FENG, i), FROM_ANY));
        allpacks.push_back(Pack(PNAME_GANG, Card(CNAME_FENG, i), FROM_ANY));
    }
    for (int i = 1; i <= 3; ++i) {
        allpacks.push_back(Pack(PNAME_PENG, Card(CNAME_JIAN, i), FROM_ANY));
        allpacks.push_back(Pack(PNAME_GANG, Card(CNAME_JIAN, i), FROM_ANY));
    }
    //random_shuffle(allpacks.begin(), allpacks.end());
    for (int i = 0; i < CNAME_XUNUM; ++i)
        for (int j = 1; j <= 9; ++j)
            allduis.push_back(Pack(PNAME_NONE, Card(i, j), FROM_ANY));
    for (int i = 1; i <= 4; ++i)
        allduis.push_back(Pack(PNAME_NONE, Card(CNAME_FENG, i), FROM_ANY));
    for (int i = 1; i <= 3; ++i)
        allduis.push_back(Pack(PNAME_NONE, Card(CNAME_JIAN, i), FROM_ANY));
}

int menfeng, quanfeng;
int huashu[4]; // 补花次数
int handcards[CNAME_TNUM][NUM_MAX]; // 手牌，不计入已亮出的牌
int wallcards[CNAME_TNUM][NUM_MAX]; // 牌墙
int wallcnt;
vector<Pack> packs[4];
vector<Card> throweds[4];

double card_prob[CNAME_TNUM][NUM_MAX] = {
    {
        0,
        /*W1*/ 0.19059186320754481,
        /*W2*/ 0.191428301886785,
        /*W3*/ 0.1933301886792462,
        /*W4*/ 0.1922028301886828,
        /*W5*/ 0.1923260613207562,
        /*W6*/ 0.19241061320754177,
        /*W7*/ 0.19330436320754057,
        /*W8*/ 0.19179775943396737,
        /*W9*/ 0.19055448113207335
    },
    {
        0,
        /*B1*/ 0.1908747641509353,
        /*B2*/ 0.19179198113207924,
        /*B3*/ 0.19329893867925105,
        /*B4*/ 0.19250766509434825,
        /*B5*/ 0.19245955188678532,
        /*B6*/ 0.19228502358490845,
        /*B7*/ 0.19338915094339904,
        /*B8*/ 0.19195188679245453,
        /*B9*/ 0.19083466981132083
    },
    {
        0,
        /*T1*/ 0.1904905660377342,
        /*T2*/ 0.19168455188679095,
        /*T3*/ 0.19310483490565417,
        /*T4*/ 0.1924074292452801,
        /*T5*/ 0.19244693396225776,
        /*T6*/ 0.1924824292452778,
        /*T7*/ 0.19324209905660386,
        /*T8*/ 0.19176816037736108,
        /*T9*/ 0.1908910377358543
    },
    {
        0,
        /*F1*/ 0.18849209905661088,
        /*F2*/ 0.18723702830188307,
        /*F3*/ 0.18719834905661073,
        /*F4*/ 0.18707134433962455
    },
    {
        0,
        /*J1*/ 0.18886297169811583,
        /*J2*/ 0.18865648584906428,
        /*J3*/ 0.18885377358490713
    }
};

/* 胡牌牌型（一般） */
struct HState {
    vector<Pack> packs; // pname=0 表示对子/将牌
    vector<pair<Card, int>> needcards;
    double pr; // 概率
    double pr2; // 大的先搜
    //double ex; // 番数期望
    bool operator< (const HState &a) const
    {
        return pr2 < a.pr2;
    }
};

void stateToString(const HState &a, vector<pair<string, pair<string, int>>> &pack, vector<string> &hand)
{
    pack.clear();
    hand.clear();
    for (auto i : a.needcards)
        for (int j = 0; j < i.second; ++j)
            hand.push_back(i.first.toString());
    for (Pack p : a.packs)
        if (p.pname == PNAME_NONE || p.from == FROM_SELF) { // 暗牌
            /* keep this! */;
            /*
            if (p.pname == PNAME_CHI) {
                hand.push_back(Card(p.card.cname, p.card.num - 1).toString());
                hand.push_back(p.card.toString());
                hand.push_back(Card(p.card.cname, p.card.num + 1).toString());
            }
            else {
                for (int i = 0; i < reqnum[p.pname]; ++i)
                    hand.push_back(p.card.toString());
            }
            */
        }
        else { // 明牌
            switch (p.pname) {
            case PNAME_CHI:
                pack.push_back(make_pair("CHI", make_pair(p.card.toString(), p.from == FROM_ANY ? 1 : p.from)));
                break;
            case PNAME_PENG:
                pack.push_back(make_pair("PENG", make_pair(p.card.toString(), p.from == FROM_ANY ? 1 : (menfeng - p.from) & 3)));
                break;
            case PNAME_GANG:
                pack.push_back(make_pair("GANG", make_pair(p.card.toString(), p.from == FROM_ANY ? 1 : (menfeng - p.from) & 3)));
                break;
            }
            if (p.from == FROM_ANY) {
                if (p.pname == PNAME_CHI) {
                    hand.erase(find(hand.begin(), hand.end(), Card(p.card.cname, p.card.num - 1).toString()));
                    hand.erase(find(hand.begin(), hand.end(), p.card.toString()));
                    hand.erase(find(hand.begin(), hand.end(), Card(p.card.cname, p.card.num + 1).toString()));
                }
                else {
                    for (int i = 0; i < reqnum[p.pname]; ++i)
                        hand.erase(find(hand.begin(), hand.end(), p.card.toString()));
                }
            }
        }
    sort(hand.begin(), hand.end(), [](const string & a, const string & b) {
        int t1 = char2cname(a[0]);
        int t2 = char2cname(b[0]);
        if (t1 != t2)
            return t1 < t2;
        return a[1] < b[1];
    });
}

void showHState(const HState &s)
{
    vector<pair<string, pair<string, int>>> pack;
    vector<string> hand;
    stateToString(s, pack, hand);
    for (auto i : pack)
        printf("{%s, %s} ", i.first.c_str(), i.second.first.c_str());
    printf("| ");
    for (auto i : hand)
        printf("%s ", i.c_str());
    printf("| %g\n", s.pr);
}

void showTargets(const vector<HState> &a)
{
    for (const HState &s : a)
        showHState(s);
}

void showHand()
{
    for (Pack p : packs[menfeng])
        printf("{%s, %s} ", pname2char[p.pname], p.card.toString().c_str());
    printf("| ");
    for (int i = 0; i < CNAME_TNUM; ++i)
        for (int j = 1; j <= 9; ++j)
            for (int k = 0; k < handcards[i][j]; ++k)
                printf("%c%d ", cname2char[i], j);
    printf("| hand\n");
}

#if ADDHUA
#define goodfan(fan, hua) ((fan) >= 8)
#else
#define goodfan(fan, hua) ((fan) - (hua) >= 8)
#endif

bool canHu(const int handcards[CNAME_TNUM][NUM_MAX],
           const vector<Pack> &packs,
           Card winCard,
           bool isZIMO,
           bool isJUEZHANG,
           bool isGANG,
           bool isLAST)
{
    vector<pair<string, pair<string, int>>> pack;
    for (Pack p : packs)
        switch (p.pname) {
        case PNAME_CHI:
            pack.push_back(make_pair("CHI", make_pair(p.card.toString(), p.from)));
            break;
        case PNAME_PENG:
            pack.push_back(make_pair("PENG", make_pair(p.card.toString(), (menfeng - p.from) & 3)));
            break;
        case PNAME_GANG:
            pack.push_back(make_pair("GANG", make_pair(p.card.toString(), (menfeng - p.from) & 3)));
            break;
        }
    vector<string> hand;
    for (int i = 0; i < CNAME_TNUM; ++i)
        for (int j = 1; j <= 9; ++j)
            for (int k = 0; k < handcards[i][j]; ++k)
                hand.push_back(cname2char[i] + to_string(j));
    string wintile = winCard.toString();
    vector<pair<int, string>> ans;
    try {
        ans = MahjongFanCalculator(pack, hand, wintile, huashu[menfeng], isZIMO, isJUEZHANG, isGANG, isLAST, menfeng, quanfeng);
        int fan = 0;
        for (auto i : ans)
            fan += i.first;
        return goodfan(fan, huashu[menfeng]);
    }
    catch (...) {
        return false;
    }
}

int getFan(const HState &a)
{
    vector<pair<string, pair<string, int>>> pack;
    vector<string> hand;
    stateToString(a, pack, hand);
    try {
        vector<pair<int, string>> ans;
        string wintile = hand.front(); // strange
        hand.erase(hand.begin());
        ans = MahjongFanCalculator(pack, hand, wintile, huashu[menfeng], false, false, false, false, menfeng, quanfeng);
        int fan = 0;
        for (auto i : ans)
            fan += i.first;
        return fan;
    }
    catch (...) {
        return 0;
    }
}

/* n张牌获得m张的概率 */
double getprob(const Card &card, int n, int m)
{
    if (m > n)
        return 0;
    double pr = card_prob[card.cname][card.num];
    const int fac[5] = {1, 1, 2, 6, 24};
    double ans = 0;
    for (int i = 0; i < m; ++i)
        ans += (double)fac[n] / fac[i] / fac[n - i] * pow(pr, i) * pow(1 - pr, n - i);
    return 1 - ans;
}

int searchCardList(const vector<pair<Card, int>> &needcards, Card c)
{
    for (auto it = needcards.begin(); it != needcards.end(); ++it)
        if (it->first == c)
            return it->second;
    return 0;
};

int addCardList(vector<pair<Card, int>> &needcards, Card c, int k)
{
    for (auto it = needcards.begin(); it != needcards.end(); ++it)
        if (it->first == c)
            return it->second += k;
    if (k != 0)
        needcards.emplace_back(c, k);
    return k;
};

double stateProb(const HState &a)
{
    int cnt = 0;
    for (auto i : a.packs)
        if (i.from == FROM_ANY) {
            /*
            if (i.pname == PNAME_CHI) {
                if (handcards[i.card.cname][i.card.num - 1]
                    && handcards[i.card.cname][i.card.num]
                    && handcards[i.card.cname][i.card.num + 1])
                    return 0;
            }
            else {
                if (reqnum[i.pname] <= handcards[i.card.cname][i.card.num])
                    return 0;
            }
            */
            ++cnt;
        }

    double ans = 1;
    for (auto i : a.needcards) {
        Card c = i.first;
        if (i.second > handcards[c.cname][c.num])
            ans *= getprob(c, wallcards[c.cname][c.num], i.second - handcards[c.cname][c.num]);
    }

    ans *= pow(0.6, cnt);
    //return min(ans, 1.0);
    return ans;
}

void qidui(vector<HState> &targets)
{
    priority_queue<HState> q;
    HState s;
    int cnt = 0;
    const int QIDUI_NUM = 10;
    auto update = [&](uint8_t pname, Card card) {
        HState t = s;
        if (addCardList(t.needcards, card, 2) > handcards[card.cname][card.num] + wallcards[card.cname][card.num])
            return;
        t.packs.push_back(Pack(pname, card, FROM_SELF));
        if ((t.pr = stateProb(t)) > eps) {
            t.pr2 = t.pr * pow(2, t.packs.size());
            if (t.packs.size() == 7) {
                int fan = getFan(t);
                if (goodfan(fan, huashu[menfeng])) {
                    //t.ex = t.pr * log2(fan);
                    targets.push_back(t);
                    ++cnt;
                }
            }
            else
                q.push(t);
        }
        t.packs.pop_back();
    };

    s.pr = s.pr2 = 1;
    q.push(s);
    while (cnt < QIDUI_NUM && !q.empty()) {
        s = q.top();
        q.pop();
        auto ibegin = allduis.begin();
        if (!s.packs.empty()) {
            /* 防止重复枚举 */
            Pack sback = s.packs.back();
            while (ibegin != allduis.end()) {
                if (ibegin->pname == sback.pname && ibegin->card == sback.card)
                    break;
                ++ibegin;
            }
        }
        for (auto i = ibegin; i != allduis.end(); ++i)
            update(i->pname, i->card);
    }
}

void shisanyao(vector<HState> &targets)
{
    HState s;
    for (int i = 0; i < CNAME_XUNUM; ++i) {
        s.needcards.emplace_back(Card(i, 1), 1);
        s.needcards.emplace_back(Card(i, 9), 1);
    }
    for (int i = 1; i <= 4; ++i)
        s.needcards.emplace_back(Card(CNAME_FENG, i), 1);
    for (int i = 1; i <= 3; ++i)
        s.needcards.emplace_back(Card(CNAME_JIAN, i), 1);
    if ((s.pr = stateProb(s)) > eps)
        targets.push_back(s);
}

void bukao(vector<HState> &targets)
{
    HState s;
    Card cs0[16];
    int n = 0;
    for (int i = 1; i <= 4; ++i)
        if (handcards[CNAME_FENG][i] == 0)
            cs0[n++] = Card(CNAME_FENG, i);
        else
            s.needcards.emplace_back(Card(CNAME_FENG, i), 1);
    for (int i = 1; i <= 3; ++i)
        if (handcards[CNAME_JIAN][i] == 0)
            cs0[n++] = Card(CNAME_JIAN, i);
        else
            s.needcards.emplace_back(Card(CNAME_JIAN, i), 1);
    int n0 = n;
    for (int p1 = 0; p1 < 3; ++p1)
        for (int p2 = 0; p2 < 3; ++p2) {
            if (p1 == p2)
                continue;
            int p3 = 3 - p1 - p2;
            Card cs[16];
            memcpy(cs, cs0, sizeof(Card) * n0);
            n = n0;
            s.needcards.resize(7 - n0);
            for (int j : {1, 4, 7})
                if (handcards[p1][j] == 0)
                    cs[n++] = Card(p1, j);
                else
                    s.needcards.emplace_back(Card(p1, j), 1);
            for (int j : {2, 5, 8})
                if (handcards[p2][j] == 0)
                    cs[n++] = Card(p2, j);
                else
                    s.needcards.emplace_back(Card(p2, j), 1);
            for (int j : {3, 6, 9})
                if (handcards[p3][j] == 0)
                    cs[n++] = Card(p3, j);
                else
                    s.needcards.emplace_back(Card(p3, j), 1);
            sort(cs, cs + n, [](const Card & a, const Card & b) {return wallcards[a.cname][a.num] > wallcards[b.cname][b.num];});
            for (int i = 0; i < n - 2; ++i)
                s.needcards.emplace_back(cs[i], 1);
            if ((s.pr = stateProb(s)) > eps)
                targets.push_back(s);
        }
}

double evaluate(const vector<Pack> &packs, vector<HState> &targets)
{
    const int TARGET_SIZE_ALL = 500;
    const int TARGET_SIZE_SUM = 3;
    targets.clear();

    priority_queue<HState> q;
    HState s;
    auto update = [&](uint8_t pname, Card card) {
        HState t = s;
        // 更新needcards
        int badcnt = 0;
        if (pname == PNAME_CHI) {
            for (int i = card.num - 1; i <= card.num + 1; ++i) {
                Card c(card.cname, i);
                int num = addCardList(t.needcards, c, 1);
                if (num > handcards[card.cname][i] + wallcards[card.cname][i])
                    return;
                if (num > handcards[card.cname][i])
                    ++badcnt;
            }
            if (badcnt == 3)
                return;
        }
        else {
            int num = addCardList(t.needcards, card, reqnum[pname]);
            if (num > handcards[card.cname][card.num] + wallcards[card.cname][card.num])
                return;
            badcnt = max(0, num - handcards[card.cname][card.num]);
            if (badcnt == reqnum[pname])
                return;
        }

        // 是否鸣牌
        for (int from : {FROM_ANY, FROM_SELF}) {
            if (from == FROM_ANY && pname == PNAME_NONE)
                continue;
            t.packs.push_back(Pack(pname, card, from));
            if ((t.pr = stateProb(t)) > eps) {
                t.pr2 = t.pr * pow(ALPHA, t.packs.size());
                if (pname == PNAME_NONE) {
                    int fan = getFan(t);
                    if (goodfan(fan, huashu[menfeng])) {
                        //t.ex = t.pr * log2(fan);
                        targets.push_back(t);
                    }
                }
                else
                    q.push(t);
            }
            t.packs.pop_back();
        }
    };

    auto zuhelong = [&]() {
        for (int p1 = 0; p1 < 3; ++p1)
            for (int p2 = 0; p2 < 3; ++p2) {
                if (p1 == p2)
                    continue;
                int p3 = 3 - p1 - p2;
                HState t = s;
                for (int i = 0; i < 3; ++i)
                    t.packs.push_back(Pack(PNAME_BUKAO, Card(), FROM_SELF));
                for (int j : {1, 4, 7})
                    addCardList(t.needcards, Card(p1, j), 1);
                for (int j : {2, 5, 8})
                    addCardList(t.needcards, Card(p2, j), 1);
                for (int j : {3, 6, 9})
                    addCardList(t.needcards, Card(p3, j), 1);
                if ((t.pr = stateProb(t)) > eps) {
                    t.pr2 = t.pr * pow(ALPHA, t.packs.size());
                    q.push(t);
                }
            }
    };

    s.packs = packs;
    s.pr = s.pr2 = 1;
    q.push(s);
    while (targets.size() < TARGET_SIZE_ALL && !q.empty()) {
        TIMER_JUMP(timer, skip_evaluate);
        s = q.top();
        q.pop();
        if (s.packs.size() < 4) {
            auto ibegin = allpacks.begin();
            if (s.packs.size() > packs.size()) {
                /* 防止重复枚举 */
                Pack sback = s.packs.back();
                while (ibegin != allpacks.end()) {
                    if (ibegin->pname == sback.pname && ibegin->card == sback.card)
                        break;
                    ++ibegin;
                }
            }
            for (auto i = ibegin; i != allpacks.end(); ++i)
                update(i->pname, i->card);
        }
        else { // 缺将牌
            for (auto i : allduis)
                update(i.pname, i.card);
        }
        if (s.packs.size() == 1)
            zuhelong();
    }

    if (packs.empty()) {
        qidui(targets);
        shisanyao(targets);
        bukao(targets); // 七星不靠、全不靠
    }

skip_evaluate:
    sort(targets.begin(), targets.end(), [](const HState & a, const HState & b) {return a.pr > b.pr;});
    targets.resize(min((int)targets.size(), TARGET_SIZE_SUM));
    double ans = 0;
    for (const HState &i : targets)
        ans += (1 - ans) * i.pr;
    return ans;
}

/* 返回打出的牌 */
Card selectPlay(Card newcard, bool lastgang)
{
    if (turn < nturn) { // 假回合，复读
        char act[10], c_, cc, cn;
        scanf(" %s", act);
        switch (act[0]) {
        case 'P': // PLAY
        case 'G': // GANG
            scanf("%c%c%c", &c_, &cc, &cn);
            ungetc(cn, stdin), ungetc(cc, stdin), ungetc(c_, stdin);
            ungets(act);
            output("%s %c%c\n", act, cc, cn);
            return Card(char2cname(cc), cn - '0');
        case 'B': // BUGANG
            ungets(act);
            output("BUGANG\n");
            return newcard;
        default: // 不可能
            ungets(act);
            output("FAKE\n");
            return Card();
        }
    }

    // HU
    --handcards[newcard.cname][newcard.num];
    if (canHu(handcards, packs[menfeng], newcard, true, wallcards[newcard.cname][newcard.num] == 0, lastgang, wallcnt == 0)) {
        output("HU\n");
        return Card();
    }
    ++handcards[newcard.cname][newcard.num];

    double maxpr = 0;
    vector<HState> targets;
    Card ans;
    char anstype;
    auto update = [&](char tp, Card card) {
        vector<HState> tv;
        double v = evaluate(packs[menfeng], tv);
        if (v > maxpr || maxpr == 0) {
            maxpr = v;
            targets = tv;
            ans = card;
            anstype = tp;
        }
    };

    // BUGANG
    for (Pack &p : packs[menfeng])
        if (p.pname == PNAME_PENG && p.card == newcard) {
            p.pname = PNAME_GANG;
            --handcards[newcard.cname][newcard.num];
            update('B', newcard);
            ++handcards[newcard.cname][newcard.num];
            p.pname = PNAME_PENG;
            TIMER_JUMP(timer, all_end);
        }

    for (int i = CNAME_TNUM - 1; i >= 0; --i)
        for (int j = 1; j <= 9; ++j) {
            // PLAY
            if (handcards[i][j]) {
                --handcards[i][j];
                update('P', Card(i, j));
                ++handcards[i][j];
            }
            // GANG
            if (handcards[i][j] == 4) {
                handcards[i][j] = 0;
                packs[menfeng].push_back(Pack(PNAME_GANG, Card(i, j), menfeng));
                update('G', Card(i, j));
                packs[menfeng].pop_back();
                handcards[i][j] = 4;
            }
            TIMER_JUMP(timer, all_end);
        }

all_end:

#ifdef LOCAL
    showHand();
    showTargets(targets);
#endif

    if (anstype == 'P')
        output("PLAY %c%d\n", cname2char[ans.cname], ans.num);
    else if (anstype == 'G')
        output("GANG %c%d\n", cname2char[ans.cname], ans.num);
    else if (anstype == 'B')
        output("BUGANG\n");
    return ans;
}

void forcePlay(Card newcard, int from, bool lastgang)
{
    if (turn < nturn) {
        output("FAKE\n");    // 假回合，不用复读
        return;
    }

    // HU
    if (canHu(handcards, packs[menfeng], newcard, false, wallcards[newcard.cname][newcard.num] == 0, lastgang, wallcnt == 0)) {
        output("HU\n");
        return;
    }

    vector<HState> targets;
    double maxpr = evaluate(packs[menfeng], targets);
    Card ans, mid;
    char anstype = 'N';

    auto update = [&](char tp, Card cardout) {
        vector<HState> tv;
        double v = evaluate(packs[menfeng], tv);
        if (v > maxpr) {
            maxpr = v;
            targets = tv;
            ans = cardout;
            anstype = tp;
            return true;
        }
        return false;
    };

    auto update_emu = [&](char tp) {
        bool ok = false;
        for (int i = CNAME_TNUM - 1; i >= 0; --i)
            for (int j = 1; j <= 9; ++j)
                if (handcards[i][j]) {
                    if (i == newcard.cname && j == newcard.num)
                        continue;
                    --handcards[i][j];
                    if (update(tp, Card(i, j)))
                        ok = true;
                    ++handcards[i][j];
                    TIMER_JUMP(timer, emu_end);
                }
emu_end:
        return ok;
    };

    // CHI
    if (((from + 1) & 3) == menfeng && newcard.cname < CNAME_XUNUM)
        for (int pos = 1; pos <= 3; ++pos) {
            if (newcard.num < pos || newcard.num > 6 + pos)
                continue;
            Card tmp(newcard.cname, newcard.num - pos + 2); // 中间牌
            bool ok = true;
            for (int i = tmp.num - 1; i <= tmp.num + 1; ++i)
                if (i != newcard.num && handcards[tmp.cname][i] == 0)
                    ok = false;
            if (!ok)
                continue;
            packs[menfeng].push_back(Pack(PNAME_CHI, tmp, pos));
            for (int i = tmp.num - 1; i <= tmp.num + 1; ++i)
                if (i != newcard.num)
                    --handcards[tmp.cname][i];
            if (update_emu('C'))
                mid = tmp;
            for (int i = tmp.num - 1; i <= tmp.num + 1; ++i)
                if (i != newcard.num)
                    ++handcards[tmp.cname][i];
            packs[menfeng].pop_back();
            TIMER_JUMP(timer, all_end);
        }

    // GANG
    if (handcards[newcard.cname][newcard.num] == 3) {
        packs[menfeng].push_back(Pack(PNAME_GANG, newcard, from));
        handcards[newcard.cname][newcard.num] = 0;
        update('G', Card());
        handcards[newcard.cname][newcard.num] = 3;
        packs[menfeng].pop_back();
    }

    // PENG
    if (handcards[newcard.cname][newcard.num] == 2) {
        packs[menfeng].push_back(Pack(PNAME_PENG, newcard, from));
        handcards[newcard.cname][newcard.num] = 0;
        update_emu('P');
        handcards[newcard.cname][newcard.num] = 2;
        packs[menfeng].pop_back();
    }

all_end:

#ifdef LOCAL
    showHand();
    showTargets(targets);
#endif

    if (anstype == 'P')
        output("PENG %c%d\n", cname2char[ans.cname], ans.num);
    else if (anstype == 'C')
        output("CHI %c%d %c%d\n", cname2char[mid.cname], mid.num, cname2char[ans.cname], ans.num);
    else if (anstype == 'G')
        output("GANG\n");
    else if (anstype == 'N')
        output("PASS\n");
}

void init()
{
    //srand(time(0));
    for (int i = 0; i < CNAME_XUNUM; ++i)
        for (int j = 1; j <= 9; ++j)
            wallcards[i][j] = 4;
    for (int i = 1; i <= 4; ++i)
        wallcards[CNAME_FENG][i] = 4;
    for (int i = 1; i <= 3; ++i)
        wallcards[CNAME_JIAN][i] = 4;
    wallcnt = 144;
    initAllPacks();
    MahjongInit();
}

Card readCard()
{
    char cname, num;
    scanf(" %c%c", &cname, &num);
    return Card(char2cname(cname), num - '0');
}

void read0()
{
    scanf("%d", &nturn);
    scanf("%*d%d%d", &menfeng, &quanfeng);
    swap(card_prob[CNAME_FENG][1], card_prob[CNAME_FENG][quanfeng]);
    output("PASS\n");
}

void read1()
{
    scanf("%*d");
    wallcnt -= 13 * 4;
    for (int i = 0; i < 4; ++i) {
        scanf("%d", &huashu[i]);
        wallcnt -= huashu[i];
    }
    for (int i = 0; i < 13; ++i) {
        Card c = readCard();
        ++handcards[c.cname][c.num];
        --wallcards[c.cname][c.num];
    }
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < huashu[i]; ++j)
            scanf("%*s");
    output("PASS\n");
}

void main_loop()
{
    Card lastcard, nowcard;
    int lastfrom;
    bool lastmo = false, lastgang = false;
    int player;
    auto newcardOut = [&]() {
        nowcard = readCard();
        if (player != menfeng) {
            --wallcards[nowcard.cname][nowcard.num];
            forcePlay(nowcard, player, lastgang);
        }
        else {
            --handcards[nowcard.cname][nowcard.num];
            output("PASS\n");
        }
        throweds[player].push_back(nowcard);
        lastcard = nowcard;
        lastfrom = player;
    };
    while (1) {
        bool nowmo = false, nowgang = false;
        int op;
        scanf("%d", &op);
        timer.start();
        if (op == 2) {
            nowcard = readCard();
            ++handcards[nowcard.cname][nowcard.num];
            --wallcards[nowcard.cname][nowcard.num];
            --wallcnt;
            lastcard = selectPlay(nowcard, lastgang);
            lastfrom = menfeng;
            nowmo = true;
        }
        else  { // op == 3
            char act[10];
            scanf("%d%s", &player, act);
            if (strcmp(act, "BUHUA") == 0) {
                scanf("%*s");
                ++huashu[player];
                --wallcnt;
                output("PASS\n");
            }
            else if (strcmp(act, "DRAW") == 0) {
                --wallcnt;
                output("PASS\n");
                nowmo = true;
            }
            else if (strcmp(act, "PLAY") == 0)
                newcardOut();
            else if (strcmp(act, "PENG") == 0) {
                if (player != menfeng)
                    wallcards[lastcard.cname][lastcard.num] -= 2;
                else
                    handcards[lastcard.cname][lastcard.num] -= 2;
                packs[player].push_back(Pack(PNAME_PENG, lastcard, lastfrom));
                newcardOut();
            }
            else if (strcmp(act, "CHI") == 0) {
                Card chicard = readCard();
                if (player != menfeng) {
                    for (int i = -1; i <= 1; ++i)
                        if (chicard.num + i != lastcard.num)
                            --wallcards[chicard.cname][chicard.num + i];
                }
                else {
                    for (int i = -1; i <= 1; ++i)
                        if (chicard.num + i != lastcard.num)
                            --handcards[chicard.cname][chicard.num + i];
                }
                packs[player].push_back(Pack(PNAME_CHI, chicard, lastcard.num - chicard.num + 2));
                newcardOut();
            }
            else if (strcmp(act, "GANG") == 0) {
                if (lastmo) { // 暗杠
                    if (player == menfeng) {
                        handcards[lastcard.cname][lastcard.num] -= 4;
                        packs[player].push_back(Pack(PNAME_GANG, lastcard, player));
                    }
                    else
                        packs[player].push_back(Pack(PNAME_GANG, Card(), player));
                }
                else { // 明杠
                    if (player != menfeng)
                        wallcards[lastcard.cname][lastcard.num] -= 3;
                    else
                        handcards[lastcard.cname][lastcard.num] -= 3;
                    packs[player].push_back(Pack(PNAME_GANG, lastcard, lastfrom));
                }
                output("PASS\n");
                nowgang = true;
            }
            else if (strcmp(act, "BUGANG") == 0) {
                nowcard = readCard();
                if (player != menfeng) {
                    --wallcards[nowcard.cname][nowcard.num];
                    if (canHu(handcards, packs[menfeng], nowcard, false, true, true, wallcnt == 0)) { // 抢杠
                        output("HU\n");
                        exit(0);
                    }
                }
                else
                    --handcards[nowcard.cname][nowcard.num];
                for (Pack &p : packs[player])
                    if (p.pname == PNAME_PENG && p.card == nowcard) {
                        p.pname = PNAME_GANG;
                        break;
                    }
                output("PASS\n");
                nowgang = true;
            }
        }
        lastmo = nowmo;
        lastgang = nowgang;
    }
}

int main()
{
    init();
    read0();
    read1();
    main_loop();
    return 0;
}
