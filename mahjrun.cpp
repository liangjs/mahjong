#include "LibMahjongGB/MahjongGB.h"
#ifdef _MSC_VER
#include <process.h>
#else
#include <unistd.h>
#endif

char *exe[4];

int nturn[4];
vector<string> logs[4];

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

int quanfeng;
vector<Card> huas[4];
int handcards[4][CNAME_TNUM][NUM_MAX]; // 手牌，不计入已亮出的牌
int leftcards[CNAME_TNUM][NUM_MAX];
deque<Card> wallcards;
vector<Pack> packs[4];
vector<Card> throweds[4];

vector<pair<int, string>> fan(int id, Card winCard, bool isZIMO, bool isJUEZHANG, bool isGANG, bool isLAST)
{
    vector<pair<string, pair<string, int>>> pack;
    for (Pack p : packs[id])
        switch (p.pname) {
        case PNAME_CHI:
            pack.push_back(make_pair("CHI", make_pair(p.card.toString(), p.from)));
            break;
        case PNAME_PENG:
            pack.push_back(make_pair("PENG", make_pair(p.card.toString(), (id - p.from) & 3)));
            break;
        case PNAME_GANG:
            pack.push_back(make_pair("GANG", make_pair(p.card.toString(), (id - p.from) & 3)));
            break;
        }
    vector<string> hand;
    for (int i = 0; i < CNAME_TNUM; ++i)
        for (int j = 1; j <= 9; ++j)
            for (int k = 0; k < handcards[id][i][j]; ++k)
                hand.push_back(cname2char[i] + to_string(j));
    string wintile = winCard.toString();
    vector<pair<int, string>> ans;
    try {
        ans = MahjongFanCalculator(pack, hand, wintile, huas[id].size(), isZIMO, isJUEZHANG, isGANG, isLAST, id, quanfeng);
    }
    catch (...) {
    }
    return ans;
}

void init()
{
    int seed = time(0) ^ getpid();
    srand(seed);
    printf("seed %d\n", seed);

    quanfeng = rand() % 4;
    for (int i = 0; i < CNAME_XUNUM; ++i)
        for (int j = 1; j <= 9; ++j)
            for (int k = 0; k < 4; ++k)
                wallcards.push_back(Card(i, j));
    for (int i = 1; i <= 4; ++i)
        for (int k = 0; k < 4; ++k)
            wallcards.push_back(Card(CNAME_FENG, i));
    for (int i = 1; i <= 3; ++i)
        for (int k = 0; k < 4; ++k)
            wallcards.push_back(Card(CNAME_JIAN, i));
    for (int i = 1; i <= 8; ++i)
        wallcards.push_back(Card(CNAME_HUA, i));
    random_shuffle(wallcards.begin(), wallcards.end());
    for (Card c : wallcards)
        leftcards[c.cname][c.num]++;
    for (int i = 0; i < 4; ++i) {
        int cnt = 0;
        while (cnt < 13) {
            Card c = wallcards.front();
            wallcards.pop_front();
            --leftcards[c.cname][c.num];
            if (c.cname == CNAME_HUA)
                huas[i].push_back(c);
            else {
                ++cnt;
                handcards[i][c.cname][c.num]++;
            }
        }
    }
}

void runerror(int id)
{
    int score[4] = {10, 10, 10, 10};
    score[id] = -30;
    printf("%d %d %d %d\n", score[0], score[1], score[2], score[3]);
    exit(0);
}

void show_stat(int id, const char *fmt, ...)
{
    printf("turn %d\n", nturn[id]);
    for (int i = 0; i < CNAME_XUNUM; ++i)
        for (int j = 1; j <= 9; ++j)
            printf("%c%d=%d ", cname2char[i], j, leftcards[i][j]);
    for (int i = 1; i <= 4; ++i)
        printf("%c%d=%d ", cname2char[CNAME_FENG], i, leftcards[CNAME_FENG][i]);
    for (int i = 1; i <= 3; ++i)
        printf("%c%d=%d ", cname2char[CNAME_JIAN], i, leftcards[CNAME_JIAN][i]);
    putchar('\n');
    for (int i = 0; i < 4; ++i) {
        for (Pack p : packs[i])
            printf("{%s, %s} ", pname2char[p.pname], p.card.toString().c_str());
        printf("| ");
        for (int _i = 0; _i < CNAME_TNUM; ++_i)
            for (int j = 1; j <= 9; ++j)
                for (int k = 0; k < handcards[i][_i][j]; ++k)
                    printf("%c%d ", cname2char[_i], j);
        printf("| ");
        for (Card c : huas[i])
            printf("%c%d ", cname2char[c.cname], c.num);
        putchar('\n');
    }
    va_list ap;
    va_start(ap, fmt);
    printf("%d ", id);
    vprintf(fmt, ap);
    va_end(ap);
    putchar('\n');
    putchar('\n');
    fflush(stdout);
}

void hu(int id, int from, Card winCard, bool isGANG)
{
    vector<pair<int, string>> f = fan(id, winCard, id == from, leftcards[winCard.cname][winCard.num] == 0, isGANG, wallcards.empty());
    int sum = 0;
    for (auto i : f)
        sum += i.first;
    int score[4] = {};
    for (int i = 0; i < 4; ++i)
        if (i != id) {
            int t = 8;
            if (id == from || from == i)
                t += sum;
            score[i] -= t;
            score[id] += t;
        }
    string ff;
    for (auto &i : f)
        ff += i.second + '(' + to_string(i.first) + ") ";
    ++handcards[id][winCard.cname][winCard.num];
    show_stat(id, "HU %s", ff.c_str());
    if (sum < 8)
        runerror(id);
    printf("%d %d %d %d\n", score[0], score[1], score[2], score[3]);
    exit(0);
}

char abuf[100];
char *interact(int id, const char *fmt, ...)
{
    ++nturn[id];

    FILE *file;
    va_list ap;
    va_start(ap, fmt);
    vsprintf(abuf, fmt, ap);
    va_end(ap);
    logs[id].push_back(abuf);
    sprintf(abuf, "log/r%dp%d.in", nturn[id], id);
    file = fopen(abuf, "w");
    fprintf(file, "%d\n", nturn[id]);
    for (const string &s : logs[id])
        fprintf(file, "%s\n", s.c_str());
    fclose(file);

    sprintf(abuf, "%s < log/r%dp%d.in > log/r%dp%d.out", exe[id], nturn[id], id, nturn[id], id);
    int r = system(abuf);
    if (r)
        runerror(id);

    sprintf(abuf, "log/r%dp%d.out", nturn[id], id);
    file = fopen(abuf, "r");
    fgets(abuf, 100, file);
    abuf[strlen(abuf) - 1] = '\0'; // ignore '\n'
    logs[id].push_back(abuf);
    fclose(file);

    return abuf;
}

void turn01()
{
    for (int i = 0; i < 4; ++i) {
        interact(i, "0 %d %d", i, quanfeng);
        if (strcmp(abuf, "PASS") != 0)
            runerror(i);
    }

    for (int i = 0; i < 4; ++i) {
        string s = "1";
        for (int j = 0; j < 4; ++j)
            s += ' ' + to_string(huas[j].size());
        for (int j = 0; j < CNAME_TNUM; ++j)
            for (int k = 1; k <= 9; ++k)
                for (int l = 0; l < handcards[i][j][k]; ++l)
                    s += ' ' + Card(j, k).toString();
        for (int j = 0; j < 4; ++j)
            for (Card c : huas[j])
                s += ' ' + c.toString();
        interact(i, "%s", s.c_str());
        if (strcmp(abuf, "PASS") != 0)
            runerror(i);
    }
}

bool mopai(int id, Card *newCard)
{
    while (!wallcards.empty()) {
        Card c = wallcards.front();
        wallcards.pop_front();
        --leftcards[c.cname][c.num];
        if (c.cname == CNAME_HUA) {
            huas[id].push_back(c);
            for (int i = 0; i < 4; ++i) {
                interact(i, "3 %d BUHUA %s", id, c.toString().c_str());
                if (strcmp(abuf, "PASS") != 0)
                    runerror(i);
            }
        }
        else {
            for (int i = 0; i < 4; ++i)
                if (i != id) {
                    interact(i, "3 %d DRAW", id);
                    if (strcmp(abuf, "PASS") != 0)
                        runerror(i);
                }
            interact(id, "2 %s", c.toString().c_str());
            *newCard = c;
            ++handcards[id][c.cname][c.num];
            return true;
        }
    }
    return false;
}

static int order(const char *s)
{
    if (s[0] == 'P' && s[1] == 'A')
        return 0;
    if (s[0] == 'C')
        return 1;
    if (s[0] == 'P' && s[1] == 'E')
        return 2;
    if (s[0] == 'G')
        return 3;
    if (s[0] == 'H')
        return 4;
    return -1;
}

void main_loop()
{
    static char act[4][100];
    int id = 0;
    Card nc;
    bool lastgang = false;
    while (mopai(id, &nc)) {
        if (strncmp(abuf, "PLAY", 4) == 0) {
            Card c;
            char cname;
            sscanf(abuf, "%*s %c%hhd", &cname, &c.num);
            c.cname = char2cname(cname);
            if (handcards[id][c.cname][c.num] == 0)
                runerror(id);
            --handcards[id][c.cname][c.num];
            throweds[id].push_back(c);
            show_stat(id, "GET %s PLAY %s", nc.toString().c_str(), c.toString().c_str());
            for (int i = 0; i < 4; ++i)
                strcpy(act[i], interact(i, "3 %d PLAY %s", id, c.toString().c_str()));
            lastgang = false;
            while (1) {
                int od = -1;
                for (int i = 0; i < 4; ++i) {
                    int odi = order(act[i]);
                    if (odi == -1)
                        runerror(i);
                    if (odi > od)
                        od = odi;
                }
                if (od == 0) { // pass
                    id = (id + 1) % 4;
                    break;
                }
                int id0 = id;
                for (int i = 0; i < 4; ++i)
                    if (od == order(act[i])) {
                        id = i;
                        break;
                    }
                if (strncmp(act[id], "CHI", 3) == 0) {
                    char c1, c2;
                    int num1, num2;
                    sscanf(act[id], "%*s %c%d %c%d", &c1, &num1, &c2, &num2);
                    Card pc(char2cname(c1), num1);
                    if (pc.cname != c.cname)
                        runerror(id);
                    int cnt = 0;
                    for (int i = pc.num - 1; i <= pc.num + 1; ++i)
                        if (i != c.num) {
                            if (handcards[id][pc.cname][i] == 0)
                                runerror(id);
                            --handcards[id][pc.cname][i];
                            ++cnt;
                        }
                    if (cnt != 2)
                        runerror(id);
                    packs[id].push_back(Pack(PNAME_CHI, pc, id0));
                    c = Card(char2cname(c2), num2);
                    if (handcards[id][c.cname][c.num] == 0)
                        runerror(id);
                    --handcards[id][c.cname][c.num];
                    throweds[id].push_back(c);
                    show_stat(id, "CHI %s PlAY %s", pc.toString().c_str(), c.toString().c_str());
                    for (int i = 0; i < 4; ++i)
                        strcpy(act[i], interact(i, "3 %d CHI %s %s", id, pc.toString().c_str(), c.toString().c_str()));
                    lastgang = false;
                }
                else if (strncmp(act[id], "PENG", 4) == 0) {
                    char c1;
                    int num1;
                    sscanf(act[id], "%*s %c%d", &c1, &num1);
                    if (handcards[id][c.cname][c.num] < 2)
                        runerror(id);
                    handcards[id][c.cname][c.num] -= 2;
                    packs[id].push_back(Pack(PNAME_PENG, c, id0));
                    Card pc = c;
                    c = Card(char2cname(c1), num1);
                    if (handcards[id][c.cname][c.num] == 0)
                        runerror(id);
                    --handcards[id][c.cname][c.num];
                    throweds[id].push_back(c);
                    show_stat(id, "PENG %s PlAY %s", pc.toString().c_str(), c.toString().c_str());
                    for (int i = 0; i < 4; ++i)
                        strcpy(act[i], interact(i, "3 %d PENG %s", id, c.toString().c_str()));
                    lastgang = false;
                }
                else if (strncmp(act[id], "GANG", 4) == 0) {
                    if (handcards[id][c.cname][c.num] < 3)
                        runerror(id);
                    handcards[id][c.cname][c.num] -= 3;
                    packs[id].push_back(Pack(PNAME_PENG, c, id0));
                    show_stat(id, "GANG %s", c.toString().c_str());
                    for (int i = 0; i < 4; ++i) {
                        interact(i, "3 %d GANG", id);
                        if (strcmp(abuf, "PASS") != 0)
                            runerror(i);
                    }
                    lastgang = true;
                    break;
                }
                else if (strncmp(act[id], "HU", 2) == 0)
                    hu(id, id0, c, lastgang);
                else
                    runerror(id);
            }
        }
        else if (strncmp(abuf, "GANG", 4) == 0) {
            Card c;
            char cname;
            sscanf(abuf, "%*s %c%hhd", &cname, &c.num);
            c.cname = char2cname(cname);
            if (handcards[id][c.cname][c.num] < 4)
                runerror(id);
            handcards[id][c.cname][c.num] = 0;
            packs[id].push_back(Pack(PNAME_GANG, c, FROM_SELF));
            show_stat(id, "GET %s GANG %s", nc.toString().c_str(), c.toString().c_str());
            for (int i = 0; i < 4; ++i) {
                interact(i, "3 %d GANG", id);
                if (strcmp(abuf, "PASS") != 0)
                    runerror(i);
            }
            lastgang = true;
        }
        else if (strncmp(abuf, "BUGANG", 6) == 0) {
            bool ok = false;
            for (Pack &p : packs[id])
                if (p.pname == PNAME_PENG && p.card == nc) {
                    handcards[id][nc.cname][nc.num] = 0;
                    p.pname = PNAME_GANG;
                    ok = true;
                    break;
                }
            if (!ok)
                runerror(id);
            show_stat(id, "GET %s BUGANG", nc.toString().c_str());
            for (int i = 0; i < 4; ++i) {
                interact(i, "3 %d BUGANG %s", id, nc.toString().c_str());
                if (strcmp(abuf, "HU") == 0)
                    hu(i, id, nc, true);
                else if (strcmp(abuf, "PASS") != 0)
                    runerror(i);
            }
            lastgang = true;
        }
        else if (strncmp(abuf, "HU", 2) == 0) {
            --handcards[id][nc.cname][nc.num];
            hu(id, id, nc, lastgang);
        }
        else
            runerror(id);
    }
}

void prepare_log()
{
#ifdef _WIN32
    system("mkdir log > nul");
#else
    system("mkdir -p log");
#endif
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        printf("Usage: %s <player0> <player1> <player2> <player3>\n", argv[0]);
        return 1;
    }
    for (int i = 0; i < 4; ++i)
        exe[i] = argv[i + 1];
    prepare_log();

    init();
    MahjongInit();
    turn01();
    main_loop();
    printf("0 0 0 0\n");
    return 0;
}
